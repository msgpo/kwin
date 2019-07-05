/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2019 Vlad Zagorodniy <vladzzag@gmail.com>

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

#include "drm_device.h"
#include "drm_connector.h"
#include "drm_context.h"
#include "drm_crtc.h"
#include "drm_dumb_allocator.h"
#include "drm_gbm_allocator.h"
#include "drm_output.h"
#include "drm_plane.h"
#include "drm_pointer.h"

#include "logind.h"

#include <memory>

#include <xf86drm.h>

namespace KWin
{

static uint64_t queryCapability(int fd, uint32_t capability)
{
    uint64_t value = 0;

    if (drmGetCap(fd, capability, &value)) {
        return 0;
    }

    return value;
}

DrmDevice::DrmDevice(const QString &fileName, DrmContext *context, QObject *parent)
    : QObject(parent)
    , m_fileName(fileName)
    , m_context(context)
{
    Q_UNUSED(context)

    m_fd = LogindIntegration::self()->takeDevice(fileName.toUtf8());
    if (m_fd < 0) {
        return;
    }

    DrmScopedPointer<drmModeRes> resources(drmModeGetResources(m_fd));
    if (!resources) {
        return;
    }

    m_supportsDumbBuffer = queryCapability(m_fd, DRM_CAP_DUMB_BUFFER);
    m_supportsExportBuffer = queryCapability(m_fd, DRM_CAP_PRIME) & DRM_PRIME_CAP_EXPORT;
    m_supportsImportBuffer = queryCapability(m_fd, DRM_CAP_PRIME) & DRM_PRIME_CAP_IMPORT;
    m_supportsBufferModifier = queryCapability(m_fd, DRM_CAP_ADDFB2_MODIFIERS);

    switch (m_context->compositingType()) {
    case QPainterCompositing:
        m_allocator = new DrmDumbAllocator(this);
        break;

    case OpenGLCompositing:
    case OpenGL2Compositing:
        m_allocator = new DrmGbmAllocator(this);
        break;

    default:
        return;
    }

    m_isValid = true;
}

DrmDevice::~DrmDevice()
{
    qDeleteAll(m_outputs);
    qDeleteAll(m_planes);
    qDeleteAll(m_crtcs);
    qDeleteAll(m_connectors);

    delete m_allocator;

    if (m_fd != -1) {
        LogindIntegration::self()->releaseDevice(m_fd);
    }
}

bool DrmDevice::isValid() const
{
    return m_isValid;
}

QString DrmDevice::fileName() const
{
    return m_fileName;
}

int DrmDevice::fd() const
{
    return m_fd;
}

bool DrmDevice::supports(DeviceCapability capability) const
{
    switch (capability) {
    case DeviceCapabilityDumbBuffer:
        return m_supportsDumbBuffer;
    case DeviceCapabilityExportBuffer:
        return m_supportsExportBuffer;
    case DeviceCapabilityImportBuffer:
        return m_supportsImportBuffer;
    case DeviceCapabilityBufferModifier:
        return m_supportsBufferModifier;
    default:
        Q_UNREACHABLE();
    }
}

bool DrmDevice::enable(ClientCapability capability)
{
    switch (capability) {
    case ClientCapabilityAtomic:
        return !drmSetClientCap(m_fd, DRM_CLIENT_CAP_ATOMIC, 1);
    case ClientCapabilityUniversalPlanes:
        return !drmSetClientCap(m_fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
    default:
        Q_UNREACHABLE();
    }
}

DrmAllocator *DrmDevice::allocator() const
{
    return m_allocator;
}

QVector<DrmConnector *> DrmDevice::connectors() const
{
    return m_connectors;
}

DrmConnector *DrmDevice::findConnector(uint32_t id) const
{
    for (DrmConnector *connector : m_connectors) {
        if (connector->objectId() == id) {
            return connector;
        }
    }

    return nullptr;
}

QVector<DrmCrtc *> DrmDevice::crtcs() const
{
    return m_crtcs;
}

DrmCrtc *DrmDevice::findCrtc(uint32_t id) const
{
    for (DrmCrtc *crtc : m_crtcs) {
        if (crtc->objectId() == id) {
            return crtc;
        }
    }

    return nullptr;
}

QVector<DrmPlane *> DrmDevice::planes() const
{
    return m_planes;
}

QVector<DrmOutput *> DrmDevice::outputs() const
{
    return m_outputs;
}

DrmOutput *DrmDevice::findOutput(uint32_t crtcId) const
{
    for (DrmOutput *output : m_outputs) {
        DrmCrtc *crtc = output->crtc();
        if (!crtc) {
            continue;
        }
        if (crtc->objectId() == crtcId) {
            return output;
        }
    }

    return nullptr;
}

// static std::chrono::nanoseconds makeTimestamp(uint tv_sec, uint tv_usec)
// {
//     const auto seconds = std::chrono::seconds(tv_sec);
//     const auto microseconds = std::chrono::microseconds(tv_usec);
//     return seconds + microseconds;
// }

static void deviceEventHandler(int,
    unsigned int sequence,
    unsigned int tv_sec,
    unsigned int tv_usec,
    unsigned int crtc_id,
    void *user_data)
{
    Q_UNUSED(sequence)
    Q_UNUSED(tv_sec)
    Q_UNUSED(tv_usec)

    DrmDevice *device = static_cast<DrmDevice *>(user_data);

    DrmOutput *output = device->findOutput(crtc_id);
    if (!output) {
        return;
    }

    // TODO: Notify Compositor. Perhaps we need to supply the presentation timestamp.

    if (device->isFrozen()) {
        return;
    }
}

void DrmDevice::dispatchEvents()
{
    drmEventContext context = {};
    context.version = 3;
    context.page_flip_handler2 = deviceEventHandler;

    drmHandleEvent(m_fd, &context);
}

bool DrmDevice::isIdle() const
{
    return true;
}

void DrmDevice::waitIdle()
{
    freeze();

    while (!isIdle()) {
        QCoreApplication::processEvents();
    }

    unfreeze();
}

bool DrmDevice::isFrozen() const
{
    return m_freezeCount;
}

void DrmDevice::freeze()
{
    m_freezeCount++;
}

void DrmDevice::unfreeze()
{
    m_freezeCount--;
}

void DrmDevice::scanConnectors()
{
    DrmScopedPointer<drmModeRes> resources(drmModeGetResources(m_fd));
    if (!resources) {
        return;
    }

    QVector<DrmConnector *> connectors;

    for (int i = 0; i < resources->count_connectors; ++i) {
        const uint32_t connectorId = resources->connectors[i];

        if (DrmConnector *connector = findConnector(connectorId)) {
            connectors << connector;
            continue;
        }

        auto connector = std::make_unique<DrmConnector>(this, connectorId);
        if (!connector->isOnline()) {
            continue;
        }

        connectors << connector.release();
    }

    m_connectors = connectors;
}

void DrmDevice::scanCrtcs()
{
    DrmScopedPointer<drmModeRes> resources(drmModeGetResources(m_fd));
    if (!resources) {
        return;
    }

    for (int i = 0; i < resources->count_crtcs; ++i) {
        DrmCrtc *crtc = new DrmCrtc(this, resources->crtcs[i], i);
        m_crtcs << crtc;
    }
}

void DrmDevice::scanPlanes()
{
    DrmScopedPointer<drmModePlaneRes> resources(drmModeGetPlaneResources(m_fd));
    if (!resources) {
        return;
    }

    for (uint32_t i = 0; i < resources->count_planes; ++i) {
        DrmPlane *plane = new DrmPlane(this, resources->planes[i]);
        m_planes << plane;
    }
}

void DrmDevice::reroute()
{
}

} // namespace KWin
