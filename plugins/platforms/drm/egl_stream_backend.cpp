/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2019 NVIDIA Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "egl_stream_backend.h"
#include "composite.h"
#include "drm_backend.h"
#include "drm_output.h"
#include "drm_object_crtc.h"
#include "drm_object_plane.h"
#include "logging.h"
#include "logind.h"
#include "options.h"
#include "scene.h"
#include "screens.h"
#include "wayland_server.h"
#include <kwinglplatform.h>
#include <QOpenGLContext>
#include <KWayland/Server/surface_interface.h>
#include <KWayland/Server/buffer_interface.h>
#include <KWayland/Server/eglstream_controller_interface.h>
#include <KWayland/Server/display.h>
#include <KWayland/Server/resource.h>
#include <wayland-server-core.h>

namespace KWin
{

typedef EGLStreamKHR (*PFNEGLCREATESTREAMATTRIBNV)(EGLDisplay, EGLAttrib *);
typedef EGLBoolean (*PFNEGLGETOUTPUTLAYERSEXT)(EGLDisplay, EGLAttrib *, EGLOutputLayerEXT *, EGLint, EGLint *);
typedef EGLBoolean (*PFNEGLSTREAMCONSUMEROUTPUTEXT)(EGLDisplay, EGLStreamKHR, EGLOutputLayerEXT);
typedef EGLSurface (*PFNEGLCREATESTREAMPRODUCERSURFACEKHR)(EGLDisplay, EGLConfig, EGLStreamKHR, EGLint *);
typedef EGLBoolean (*PFNEGLDESTROYSTREAMKHR)(EGLDisplay, EGLStreamKHR);
typedef EGLBoolean (*PFNEGLSTREAMCONSUMERACQUIREATTRIBNV)(EGLDisplay, EGLStreamKHR, EGLAttrib *);
typedef EGLBoolean (*PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALKHR)(EGLDisplay, EGLStreamKHR);
typedef EGLBoolean (*PFNEGLQUERYSTREAMATTRIBNV)(EGLDisplay, EGLStreamKHR, EGLenum, EGLAttrib *);
typedef EGLBoolean (*PFNEGLSTREAMCONSUMERRELEASEKHR)(EGLDisplay, EGLStreamKHR);
typedef EGLBoolean (*PFNEGLQUERYWAYLANDBUFFERWL)(EGLDisplay, wl_resource *, EGLint, EGLint *);
PFNEGLCREATESTREAMATTRIBNV pEglCreateStreamAttribNV = nullptr;
PFNEGLGETOUTPUTLAYERSEXT pEglGetOutputLayersEXT = nullptr;
PFNEGLSTREAMCONSUMEROUTPUTEXT pEglStreamConsumerOutputEXT = nullptr;
PFNEGLCREATESTREAMPRODUCERSURFACEKHR pEglCreateStreamProducerSurfaceKHR = nullptr;
PFNEGLDESTROYSTREAMKHR pEglDestroyStreamKHR = nullptr;
PFNEGLSTREAMCONSUMERACQUIREATTRIBNV pEglStreamConsumerAcquireAttribNV = nullptr;
PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALKHR pEglStreamConsumerGLTextureExternalKHR = nullptr;
PFNEGLQUERYSTREAMATTRIBNV pEglQueryStreamAttribNV = nullptr;
PFNEGLSTREAMCONSUMERRELEASEKHR pEglStreamConsumerReleaseKHR = nullptr;
PFNEGLQUERYWAYLANDBUFFERWL pEglQueryWaylandBufferWL = nullptr;

#ifndef EGL_CONSUMER_AUTO_ACQUIRE_EXT
#define EGL_CONSUMER_AUTO_ACQUIRE_EXT 0x332B
#endif

#ifndef EGL_DRM_MASTER_FD_EXT
#define EGL_DRM_MASTER_FD_EXT 0x333C
#endif

#ifndef EGL_DRM_FLIP_EVENT_DATA_NV
#define EGL_DRM_FLIP_EVENT_DATA_NV 0x333E
#endif

#ifndef EGL_WAYLAND_EGLSTREAM_WL
#define EGL_WAYLAND_EGLSTREAM_WL 0x334B
#endif

#ifndef EGL_WAYLAND_Y_INVERTED_WL
#define EGL_WAYLAND_Y_INVERTED_WL 0x31DB
#endif

EglStreamBackend::EglStreamBackend(DrmBackend *b)
    : AbstractEglBackend(), m_backend(b)
{
    setIsDirectRendering(true);
    connect(m_backend, &DrmBackend::outputAdded, this, &EglStreamBackend::createOutput);
    connect(m_backend, &DrmBackend::outputRemoved, this,
        [this] (DrmOutput *output) {
            auto it = std::find_if(m_outputs.begin(), m_outputs.end(),
                                   [output] (const Output &o) {
                                       return o.output == output;
                                   });
            if (it == m_outputs.end()) {
                return;
            }
            cleanupOutput(*it);
            m_outputs.erase(it);
        });
}

EglStreamBackend::~EglStreamBackend()
{
    cleanup();
}

void EglStreamBackend::cleanupSurfaces()
{
    for (auto it = m_outputs.constBegin(); it != m_outputs.constEnd(); ++it) {
        cleanupOutput(*it);
    }
    m_outputs.clear();
}

void EglStreamBackend::cleanupOutput(const Output &o)
{
    if (o.buffer != nullptr) {
        delete o.buffer;
    }
    if (o.eglSurface != EGL_NO_SURFACE) {
        eglDestroySurface(eglDisplay(), o.eglSurface);
    }
    if (o.eglStream != EGL_NO_STREAM_KHR) {
        pEglDestroyStreamKHR(eglDisplay(), o.eglStream);
    }
}

bool EglStreamBackend::initializeEgl()
{
    initClientExtensions();
    EGLDisplay display = m_backend->sceneEglDisplay();
    if (display == EGL_NO_DISPLAY) {
        if (!hasClientExtension(QByteArrayLiteral("EGL_EXT_device_base")) &&
            !(hasClientExtension(QByteArrayLiteral("EGL_EXT_device_query")) &&
              hasClientExtension(QByteArrayLiteral("EGL_EXT_device_enumeration")))) {
            setFailed("Missing required EGL client extension: "
                      "EGL_EXT_device_base or "
                      "EGL_EXT_device_query and EGL_EXT_device_enumeration");
            return false;
        }

        // Try to find the EGLDevice corresponding to our DRM device file
        int numDevices;
        eglQueryDevicesEXT(0, nullptr, &numDevices);
        QVector<EGLDeviceEXT> devices(numDevices);
        eglQueryDevicesEXT(numDevices, devices.data(), &numDevices);
        for (EGLDeviceEXT device : devices) {
            const char *drmDeviceFile = eglQueryDeviceStringEXT(device, EGL_DRM_DEVICE_FILE_EXT);
            if (m_backend->devNode() != drmDeviceFile) {
                continue;
            }

            const char *deviceExtensionCString = eglQueryDeviceStringEXT(device, EGL_EXTENSIONS);
            QByteArray deviceExtensions = QByteArray::fromRawData(deviceExtensionCString,
                                                                  qstrlen(deviceExtensionCString));
            if (!deviceExtensions.split(' ').contains(QByteArrayLiteral("EGL_EXT_device_drm"))) {
                continue;
            }

            EGLint platformAttribs[] = {
                EGL_DRM_MASTER_FD_EXT, m_backend->fd(),
                EGL_NONE
            };
            display = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, device, platformAttribs);
            break;
        }
    }

    if (display == EGL_NO_DISPLAY) {
        setFailed("No suitable EGL device found");
        return false;
    }

    setEglDisplay(display);
    if (!initEglAPI()) {
        return false;
    }

    const QVector<QByteArray> requiredExtensions = {
        QByteArrayLiteral("EGL_EXT_output_base"),
        QByteArrayLiteral("EGL_EXT_output_drm"),
        QByteArrayLiteral("EGL_KHR_stream"),
        QByteArrayLiteral("EGL_KHR_stream_producer_eglsurface"),
        QByteArrayLiteral("EGL_EXT_stream_consumer_egloutput"),
        QByteArrayLiteral("EGL_NV_stream_attrib"),
        QByteArrayLiteral("EGL_EXT_stream_acquire_mode"),
        QByteArrayLiteral("EGL_KHR_stream_consumer_gltexture"),
        QByteArrayLiteral("EGL_WL_wayland_eglstream")
    };
    for (const QByteArray &ext : requiredExtensions) {
        if (!hasExtension(ext)) {
            setFailed(QStringLiteral("Missing required EGL extension: ") + ext);
            return false;
        }
    }

    pEglCreateStreamAttribNV = (PFNEGLCREATESTREAMATTRIBNV)eglGetProcAddress("eglCreateStreamAttribNV");
    pEglGetOutputLayersEXT = (PFNEGLGETOUTPUTLAYERSEXT)eglGetProcAddress("eglGetOutputLayersEXT");
    pEglStreamConsumerOutputEXT = (PFNEGLSTREAMCONSUMEROUTPUTEXT)eglGetProcAddress("eglStreamConsumerOutputEXT");
    pEglCreateStreamProducerSurfaceKHR = (PFNEGLCREATESTREAMPRODUCERSURFACEKHR)eglGetProcAddress("eglCreateStreamProducerSurfaceKHR");
    pEglDestroyStreamKHR = (PFNEGLDESTROYSTREAMKHR)eglGetProcAddress("eglDestroyStreamKHR");
    pEglStreamConsumerAcquireAttribNV = (PFNEGLSTREAMCONSUMERACQUIREATTRIBNV)eglGetProcAddress("eglStreamConsumerAcquireAttribNV");
    pEglStreamConsumerGLTextureExternalKHR = (PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALKHR)eglGetProcAddress("eglStreamConsumerGLTextureExternalKHR");
    pEglQueryStreamAttribNV = (PFNEGLQUERYSTREAMATTRIBNV)eglGetProcAddress("eglQueryStreamAttribNV");
    pEglStreamConsumerReleaseKHR = (PFNEGLSTREAMCONSUMERRELEASEKHR)eglGetProcAddress("eglStreamConsumerReleaseKHR");
    pEglQueryWaylandBufferWL = (PFNEGLQUERYWAYLANDBUFFERWL)eglGetProcAddress("eglQueryWaylandBufferWL");
    return true;
}

EglStreamBackend::StreamTexture *EglStreamBackend::lookupStreamTexture(KWayland::Server::SurfaceInterface *surface)
{
    auto it = m_streamTextures.find(surface);
    return it != m_streamTextures.end() ?
           &it.value() :
           nullptr;
}

void EglStreamBackend::attachStreamConsumer(KWayland::Server::SurfaceInterface *surface,
                                            void *eglStream,
                                            wl_array *attribs)
{
    QVector<EGLAttrib> streamAttribs;
    streamAttribs << EGL_WAYLAND_EGLSTREAM_WL << (EGLAttrib)eglStream;
    EGLAttrib *attribArray = (EGLAttrib *)attribs->data;
    for (unsigned int i = 0; i < attribs->size; ++i) {
        streamAttribs << attribArray[i];
    }
    streamAttribs << EGL_NONE;

    EGLStreamKHR stream = pEglCreateStreamAttribNV(eglDisplay(), streamAttribs.data());
    if (stream == EGL_NO_STREAM_KHR) {
        qCWarning(KWIN_DRM) << "Failed to create EGL stream";
        return;
    }

    GLuint texture;
    StreamTexture *st = lookupStreamTexture(surface);
    if (st != nullptr) {
        pEglDestroyStreamKHR(eglDisplay(), st->stream);
        st->stream = stream;
        texture = st->texture;
    } else {
        StreamTexture newSt = { stream, 0 };
        glGenTextures(1, &newSt.texture);
        m_streamTextures.insert(surface, newSt);
        texture = newSt.texture;

        connect(surface, &KWayland::Server::Resource::unbound, this,
            [surface, this]() {
                const StreamTexture &st = m_streamTextures.take(surface);
                pEglDestroyStreamKHR(eglDisplay(), st.stream);
                glDeleteTextures(1, &st.texture);
            });
    }

    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture);
    if (!pEglStreamConsumerGLTextureExternalKHR(eglDisplay(), stream)) {
        qCWarning(KWIN_DRM) << "Failed to bind EGL stream to texture";
    }
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
}

void EglStreamBackend::init()
{
    if (!m_backend->atomicModeSetting()) {
        setFailed("EGLStream backend requires atomic modesetting");
        return;
    }

    if (!initializeEgl()) {
        setFailed("Failed to initialize EGL api");
        return;
    }
    if (!initRenderingContext()) {
        setFailed("Failed to initialize rendering context");
        return;
    }

    initKWinGL();
    setSupportsBufferAge(false);
    initWayland();

    using namespace KWayland::Server;
    m_eglStreamControllerInterface = waylandServer()->display()->createEglStreamControllerInterface();
    connect(m_eglStreamControllerInterface, &EglStreamControllerInterface::streamConsumerAttached, this,
            &EglStreamBackend::attachStreamConsumer);
    m_eglStreamControllerInterface->create();
    if (!m_eglStreamControllerInterface->isValid()) {
        setFailed("failed to initialize wayland-eglstream-controller interface");
    }
}

bool EglStreamBackend::initRenderingContext()
{
    initBufferConfigs();

    if (!createContext()) {
        return false;
    }

    const auto outputs = m_backend->drmOutputs();
    for (DrmOutput *drmOutput : outputs) {
        createOutput(drmOutput);
    }
    if (m_outputs.isEmpty()) {
        qCCritical(KWIN_DRM) << "Failed to create output surface";
        return false;
    }
    // set our first surface as the one for the abstract backend
    setSurface(m_outputs.first().eglSurface);

    return makeContextCurrent(m_outputs.first());
}

bool EglStreamBackend::resetOutput(Output &o, DrmOutput *drmOutput)
{
    o.output = drmOutput;
    if (o.buffer != nullptr) {
        delete o.buffer;
    }
    // dumb buffer used for modesetting
    o.buffer = m_backend->createBuffer(drmOutput->pixelSize());

    EGLAttrib streamAttribs[] = {
        EGL_STREAM_FIFO_LENGTH_KHR, 0, // mailbox mode
        EGL_CONSUMER_AUTO_ACQUIRE_EXT, EGL_FALSE,
        EGL_NONE
    };
    EGLStreamKHR stream = pEglCreateStreamAttribNV(eglDisplay(), streamAttribs);
    if (stream == EGL_NO_STREAM_KHR) {
        qCCritical(KWIN_DRM) << "Failed to create EGL stream for output";
        return false;
    }

    EGLAttrib outputAttribs[3];
    if (drmOutput->primaryPlane()) {
        outputAttribs[0] = EGL_DRM_PLANE_EXT;
        outputAttribs[1] = drmOutput->primaryPlane()->id();
    } else {
        outputAttribs[0] = EGL_DRM_CRTC_EXT;
        outputAttribs[1] = drmOutput->crtc()->id();
    }
    outputAttribs[2] = EGL_NONE;
    EGLint numLayers;
    EGLOutputLayerEXT outputLayer;
    pEglGetOutputLayersEXT(eglDisplay(), outputAttribs, &outputLayer, 1, &numLayers);
    if (numLayers == 0) {
        qCCritical(KWIN_DRM) << "No EGL output layers found";
        return false;
    }

    pEglStreamConsumerOutputEXT(eglDisplay(), stream, outputLayer);
    EGLint streamProducerAttribs[] = {
        EGL_WIDTH, drmOutput->pixelSize().width(),
        EGL_HEIGHT, drmOutput->pixelSize().height(),
        EGL_NONE
    };
    EGLSurface eglSurface = pEglCreateStreamProducerSurfaceKHR(eglDisplay(), config(), stream,
                                                               streamProducerAttribs);
    if (eglSurface == EGL_NO_SURFACE) {
        qCCritical(KWIN_DRM) << "Failed to create EGL surface for output";
        return false;
    }

    if (o.eglSurface != EGL_NO_SURFACE) {
        if (surface() == o.eglSurface) {
            setSurface(eglSurface);
        }
        eglDestroySurface(eglDisplay(), o.eglSurface);
    }

    if (o.eglStream != EGL_NO_STREAM_KHR) {
        pEglDestroyStreamKHR(eglDisplay(), o.eglStream);
    }

    o.eglStream = stream;
    o.eglSurface = eglSurface;
    return true;
}

void EglStreamBackend::createOutput(DrmOutput *drmOutput)
{
    Output o;
    if (!resetOutput(o, drmOutput)) {
        return;
    }

    connect(drmOutput, &DrmOutput::modeChanged, this,
        [drmOutput, this] {
            auto it = std::find_if(m_outputs.begin(), m_outputs.end(),
                [drmOutput] (const auto &o) {
                    return o.output == drmOutput;
                }
            );
            if (it == m_outputs.end()) {
                return;
            }
            resetOutput(*it, drmOutput);
        }
    );
    m_outputs << o;
}

bool EglStreamBackend::makeContextCurrent(const Output &output)
{
    const EGLSurface surface = output.eglSurface;
    if (surface == EGL_NO_SURFACE) {
        return false;
    }

    if (eglMakeCurrent(eglDisplay(), surface, surface, context()) == EGL_FALSE) {
        qCCritical(KWIN_DRM) << "Failed to make EGL context current";
        return false;
    }

    EGLint error = eglGetError();
    if (error != EGL_SUCCESS) {
        qCWarning(KWIN_DRM) << "Error occurred while making EGL context current" << error;
        return false;
    }

    const QSize &overall = screens()->size();
    const QRect &v = output.output->geometry();
    qreal scale = output.output->scale();
    glViewport(-v.x() * scale, (v.height() - overall.height() + v.y()) * scale,
               overall.width() * scale, overall.height() * scale);
    return true;
}

bool EglStreamBackend::initBufferConfigs()
{
    const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE,         EGL_STREAM_BIT_KHR,
        EGL_RED_SIZE,             1,
        EGL_GREEN_SIZE,           1,
        EGL_BLUE_SIZE,            1,
        EGL_ALPHA_SIZE,           0,
        EGL_RENDERABLE_TYPE,      isOpenGLES() ? EGL_OPENGL_ES2_BIT : EGL_OPENGL_BIT,
        EGL_CONFIG_CAVEAT,        EGL_NONE,
        EGL_NONE,
    };
    EGLint count;
    EGLConfig config;
    if (!eglChooseConfig(eglDisplay(), configAttribs, &config, 1, &count)) {
        qCCritical(KWIN_DRM) << "Failed to query available EGL configs";
        return false;
    }
    if (count == 0) {
        qCCritical(KWIN_DRM) << "No suitable EGL config found";
        return false;
    }

    setConfig(config);
    return true;
}

void EglStreamBackend::present()
{
    for (auto &o : m_outputs) {
        makeContextCurrent(o);
        presentOnOutput(o);
    }
}

void EglStreamBackend::presentOnOutput(EglStreamBackend::Output &o)
{
    eglSwapBuffers(eglDisplay(), o.eglSurface);
    if (!m_backend->present(o.buffer, o.output)) {
        return;
    }

    EGLAttrib acquireAttribs[] = {
        EGL_DRM_FLIP_EVENT_DATA_NV, (EGLAttrib)o.output,
        EGL_NONE,
    };
    if (!pEglStreamConsumerAcquireAttribNV(eglDisplay(), o.eglStream, acquireAttribs)) {
        qCWarning(KWIN_DRM) << "Failed to acquire output EGL stream frame";
    }
}

void EglStreamBackend::screenGeometryChanged(const QSize &size)
{
    Q_UNUSED(size)
}

QRegion EglStreamBackend::prepareRenderingFrame()
{
    startRenderTimer();
    return QRegion();
}

QRegion EglStreamBackend::prepareRenderingForScreen(int screenId)
{
    const Output &o = m_outputs.at(screenId);
    makeContextCurrent(o);
    return o.output->geometry();
}

void EglStreamBackend::endRenderingFrame(const QRegion &renderedRegion, const QRegion &damagedRegion)
{
    Q_UNUSED(renderedRegion)
    Q_UNUSED(damagedRegion)
}

void EglStreamBackend::endRenderingFrameForScreen(int screenId, const QRegion &renderedRegion, const QRegion &damagedRegion)
{
    Q_UNUSED(renderedRegion);
    Q_UNUSED(damagedRegion);
    Output &o = m_outputs[screenId];
    presentOnOutput(o);
}

bool EglStreamBackend::usesOverlayWindow() const
{
    return false;
}

bool EglStreamBackend::perScreenRendering() const
{
    return true;
}

} // namespace
