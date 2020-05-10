/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright 2019 Roman Gilg <subdiff@gmail.com>
Copyright 2013 Martin Gräßlin <mgraesslin@kde.org>

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
#define WL_EGL_PLATFORM 1

#include "egl_wayland_backend.h"
#include "eglbuffer_internal_p.h"
#include "eglbuffer_wayland_p.h"

#include "wayland_backend.h"
#include "wayland_output.h"

#include "composite.h"
#include "logging.h"
#include "options.h"

#include "wayland_server.h"
#include "screens.h"

// kwin libs
#include <kwinglplatform.h>

// KDE
#include <KWayland/Client/surface.h>
#include <KWaylandServer/buffer_interface.h>
#include <KWaylandServer/display.h>

// Qt
#include <QOpenGLContext>

namespace KWin
{
namespace Wayland
{

EglWaylandOutput::EglWaylandOutput(WaylandOutput *output, QObject *parent)
    : QObject(parent)
    , m_waylandOutput(output)
{
}

bool EglWaylandOutput::init(EglWaylandBackend *backend)
{
    auto surface = m_waylandOutput->surface();
    const QSize &size = m_waylandOutput->geometry().size();
    auto overlay = wl_egl_window_create(*surface, size.width(), size.height());
    if (!overlay) {
        qCCritical(KWIN_WAYLAND_BACKEND) << "Creating Wayland Egl window failed";
        return false;
    }
    m_overlay = overlay;

    EGLSurface eglSurface = EGL_NO_SURFACE;
    if (backend->havePlatformBase()) {
        eglSurface = eglCreatePlatformWindowSurfaceEXT(backend->eglDisplay(), backend->config(), (void *) overlay, nullptr);
    } else {
        eglSurface = eglCreateWindowSurface(backend->eglDisplay(), backend->config(), overlay, nullptr);
    }
    if (eglSurface == EGL_NO_SURFACE) {
        qCCritical(KWIN_WAYLAND_BACKEND) << "Create Window Surface failed";
        return false;
    }
    m_eglSurface = eglSurface;

    connect(m_waylandOutput, &WaylandOutput::sizeChanged, this, &EglWaylandOutput::updateSize);

    return true;
}

void EglWaylandOutput::updateSize(const QSize &size)
{
    wl_egl_window_resize(m_overlay, size.width(), size.height(), 0, 0);
}

EglWaylandBackend::EglWaylandBackend(WaylandBackend *b)
    : AbstractEglBackend()
    , m_backend(b)
{
    if (!m_backend) {
        setFailed("Wayland Backend has not been created");
        return;
    }
    qCDebug(KWIN_WAYLAND_BACKEND) << "Connected to Wayland display?" << (m_backend->display() ? "yes" : "no" );
    if (!m_backend->display()) {
        setFailed("Could not connect to Wayland compositor");
        return;
    }

    // Egl is always direct rendering
    setIsDirectRendering(true);

    connect(m_backend, &WaylandBackend::outputAdded, this, &EglWaylandBackend::createEglWaylandOutput);
    connect(m_backend, &WaylandBackend::outputRemoved, this,
        [this] (WaylandOutput *output) {
            auto it = std::find_if(m_outputs.begin(), m_outputs.end(),
                [output] (const EglWaylandOutput *o) {
                    return o->m_waylandOutput == output;
                }
            );
            if (it == m_outputs.end()) {
                return;
            }
            cleanupOutput(*it);
            m_outputs.erase(it);
        }
    );
}

EglWaylandBackend::~EglWaylandBackend()
{
    cleanup();
}

BufferX11Private *EglWaylandBackend::createBufferX11Private()
{
    return nullptr;
}

BufferInternalPrivate *EglWaylandBackend::createBufferInternalPrivate()
{
    return new EGLBufferInternalPrivate(this);
}

BufferWaylandPrivate *EglWaylandBackend::createBufferWaylandPrivate()
{
    return new EGLBufferWaylandPrivate(this);
}

void EglWaylandBackend::cleanupSurfaces()
{
    for (auto o : m_outputs) {
        cleanupOutput(o);
    }
    m_outputs.clear();
}

bool EglWaylandBackend::createEglWaylandOutput(WaylandOutput *waylandOutput)
{
    auto *output = new EglWaylandOutput(waylandOutput, this);
    if (!output->init(this)) {
        return false;
    }
    m_outputs << output;
    return true;
}

void EglWaylandBackend::cleanupOutput(EglWaylandOutput *output)
{
    wl_egl_window_destroy(output->m_overlay);
}

bool EglWaylandBackend::initializeEgl()
{
    initClientExtensions();
    EGLDisplay display = m_backend->sceneEglDisplay();

    // Use eglGetPlatformDisplayEXT() to get the display pointer
    // if the implementation supports it.
    if (display == EGL_NO_DISPLAY) {
        m_havePlatformBase = hasClientExtension(QByteArrayLiteral("EGL_EXT_platform_base"));
        if (m_havePlatformBase) {
            // Make sure that the wayland platform is supported
            if (!hasClientExtension(QByteArrayLiteral("EGL_EXT_platform_wayland")))
                return false;

            display = eglGetPlatformDisplayEXT(EGL_PLATFORM_WAYLAND_EXT, m_backend->display(), nullptr);
        } else {
            display = eglGetDisplay(m_backend->display());
        }
    }

    if (display == EGL_NO_DISPLAY)
        return false;
    setEglDisplay(display);
    return initEglAPI();
}

void EglWaylandBackend::init()
{
    if (!initializeEgl()) {
        setFailed("Could not initialize egl");
        return;
    }
    if (!initRenderingContext()) {
        setFailed("Could not initialize rendering context");
        return;
    }

    initKWinGL();
    initBufferAge();
    initWayland();
}

bool EglWaylandBackend::initRenderingContext()
{
    initBufferConfigs();

    if (!createContext()) {
        return false;
    }

    auto waylandOutputs = m_backend->waylandOutputs();

    // we only allow to start with at least one output
    if (waylandOutputs.isEmpty()) {
        return false;
    }

    for (auto *out : waylandOutputs) {
        if (!createEglWaylandOutput(out)) {
            return false;
        }
    }

    if (m_outputs.isEmpty()) {
        qCCritical(KWIN_WAYLAND_BACKEND) << "Create Window Surfaces failed";
        return false;
    }

    auto *firstOutput = m_outputs.first();
    // set our first surface as the one for the abstract backend, just to make it happy
    setSurface(firstOutput->m_eglSurface);
    return makeContextCurrent(firstOutput);
}

bool EglWaylandBackend::makeContextCurrent(EglWaylandOutput *output)
{
    const EGLSurface eglSurface = output->m_eglSurface;
    if (eglSurface == EGL_NO_SURFACE) {
        return false;
    }
    if (eglMakeCurrent(eglDisplay(), eglSurface, eglSurface, context()) == EGL_FALSE) {
        qCCritical(KWIN_WAYLAND_BACKEND) << "Make Context Current failed";
        return false;
    }

    EGLint error = eglGetError();
    if (error != EGL_SUCCESS) {
        qCWarning(KWIN_WAYLAND_BACKEND) << "Error occurred while creating context " << error;
        return false;
    }

    const QRect &v = output->m_waylandOutput->geometry();

    qreal scale = output->m_waylandOutput->scale();

    const QSize overall = screens()->size();
    glViewport(-v.x() * scale, (v.height() - overall.height() + v.y()) * scale,
               overall.width() * scale, overall.height() * scale);
    return true;
}

bool EglWaylandBackend::initBufferConfigs()
{
    const EGLint config_attribs[] = {
        EGL_SURFACE_TYPE,         EGL_WINDOW_BIT,
        EGL_RED_SIZE,             1,
        EGL_GREEN_SIZE,           1,
        EGL_BLUE_SIZE,            1,
        EGL_ALPHA_SIZE,           0,
        EGL_RENDERABLE_TYPE,      isOpenGLES() ? EGL_OPENGL_ES2_BIT : EGL_OPENGL_BIT,
        EGL_CONFIG_CAVEAT,        EGL_NONE,
        EGL_NONE,
    };

    EGLint count;
    EGLConfig configs[1024];
    if (eglChooseConfig(eglDisplay(), config_attribs, configs, 1, &count) == EGL_FALSE) {
        qCCritical(KWIN_WAYLAND_BACKEND) << "choose config failed";
        return false;
    }
    if (count != 1) {
        qCCritical(KWIN_WAYLAND_BACKEND) << "choose config did not return a config" << count;
        return false;
    }
    setConfig(configs[0]);

    return true;
}

void EglWaylandBackend::present()
{
    for (auto *output: qAsConst(m_outputs)) {
        makeContextCurrent(output);
        presentOnSurface(output);
    }
}

void EglWaylandBackend::presentOnSurface(EglWaylandOutput *output)
{
    output->m_waylandOutput->surface()->setupFrameCallback();
    if (!m_swapping) {
        m_swapping = true;
        Compositor::self()->aboutToSwapBuffers();
    }

    if (supportsBufferAge()) {
        eglSwapBuffers(eglDisplay(), output->m_eglSurface);
        eglQuerySurface(eglDisplay(), output->m_eglSurface, EGL_BUFFER_AGE_EXT, &output->m_bufferAge);
    } else {
        eglSwapBuffers(eglDisplay(), output->m_eglSurface);
    }

}

void EglWaylandBackend::screenGeometryChanged(const QSize &size)
{
    Q_UNUSED(size)
    // no backend specific code needed
    // TODO: base implementation in OpenGLBackend

    // The back buffer contents are now undefined
    for (auto *output : qAsConst(m_outputs)) {
        output->m_bufferAge = 0;
    }
}

QRegion EglWaylandBackend::prepareRenderingFrame()
{
    eglWaitNative(EGL_CORE_NATIVE_ENGINE);
    startRenderTimer();
    m_swapping = false;
    return QRegion();
}

QRegion EglWaylandBackend::prepareRenderingForScreen(int screenId)
{
    auto *output = m_outputs.at(screenId);
    makeContextCurrent(output);
    if (supportsBufferAge()) {
        QRegion region;

        // Note: An age of zero means the buffer contents are undefined
        if (output->m_bufferAge > 0 && output->m_bufferAge <= output->m_damageHistory.count()) {
            for (int i = 0; i < output->m_bufferAge - 1; i++)
                region |= output->m_damageHistory[i];
        } else {
            region = output->m_waylandOutput->geometry();
        }

        return region;
    }
    return QRegion();
}

void EglWaylandBackend::endRenderingFrame(const QRegion &renderedRegion, const QRegion &damagedRegion)
{
    Q_UNUSED(renderedRegion)
    Q_UNUSED(damagedRegion)
}

void EglWaylandBackend::endRenderingFrameForScreen(int screenId, const QRegion &renderedRegion, const QRegion &damagedRegion)
{
    EglWaylandOutput *output = m_outputs[screenId];
    if (damagedRegion.intersected(output->m_waylandOutput->geometry()).isEmpty() && screenId == 0) {

        // If the damaged region of a window is fully occluded, the only
        // rendering done, if any, will have been to repair a reused back
        // buffer, making it identical to the front buffer.
        //
        // In this case we won't post the back buffer. Instead we'll just
        // set the buffer age to 1, so the repaired regions won't be
        // rendered again in the next frame.
        if (!renderedRegion.intersected(output->m_waylandOutput->geometry()).isEmpty()) {
            glFlush();
        }

        for (auto *o : qAsConst(m_outputs)) {
            o->m_bufferAge = 1;
        }
        return;
    }
    presentOnSurface(output);

    // Save the damaged region to history
    // Note: damage history is only collected for the first screen. See EglGbmBackend
    // for mor information regarding this limitation.
    if (supportsBufferAge() && screenId == 0) {
        if (output->m_damageHistory.count() > 10) {
            output->m_damageHistory.removeLast();
        }

        output->m_damageHistory.prepend(damagedRegion.intersected(output->m_waylandOutput->geometry()));
    }
}

bool EglWaylandBackend::usesOverlayWindow() const
{
    return false;
}

bool EglWaylandBackend::perScreenRendering() const
{
    return true;
}

}
}
