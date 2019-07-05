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

#pragma once

#include <QObject>
#include <QVector>

namespace KWin
{

class DrmAllocator;
class DrmConnector;
class DrmContext;
class DrmCrtc;
class DrmOutput;
class DrmPlane;

/**
 * The DrmDevice class represents a DRM device.
 *
 * @todo Write documentation.
 */
class DrmDevice : public QObject
{
    Q_OBJECT

public:
    explicit DrmDevice(const QString &fileName, DrmContext *context, QObject *parent = nullptr);
    ~DrmDevice() override;

    /**
     * Returns @c true if this device is valid.
     */
    bool isValid() const;

    /**
     * Returns the file name of this device, e.g. /dev/dri/card0.
     */
    QString fileName() const;

    /**
     * Returns the file descriptor associated with this device.
     */
    int fd() const;

    /**
     * This enum type is used to specify device capabilities.
     */
    enum DeviceCapability {
        /**
         * This device supports dumb buffers.
         */
        DeviceCapabilityDumbBuffer,
        /**
         * This device can export buffers over dma-buf.
         */
        DeviceCapabilityExportBuffer,
        /**
         * This device can import buffer over dma-buf.
         */
        DeviceCapabilityImportBuffer,
        /**
         * This device supports buffer modifiers.
         */
        DeviceCapabilityBufferModifier,
    };

    /**
     * Checks whether this device supports given capability.
     *
     * If the capability is supported, @c true is returned. Otherwise @c false.
     */
    bool supports(DeviceCapability capability) const;

    /**
     * This enum type is used to specify capabilities requested by the client.
     */
    enum ClientCapability {
        /**
         * The client would like to use atomic modesetting API.
         */
        ClientCapabilityAtomic,
        /**
         * The client would like to have access to planes.
         */
        ClientCapabilityUniversalPlanes,
    };

    /**
     * Enables the given client capability.
     *
     * If the device couldn't enable the given capability, @c false is returned.
     */
    bool enable(ClientCapability capability);

    /**
     * Returns the device memory allocator.
     */
    DrmAllocator *allocator() const;

    /**
     * Returns the list of available connectors on this device.
     */
    QVector<DrmConnector *> connectors() const;

    /**
     * Returns a connector with the given id.
     *
     * If there is no such a connector, @c null is returned.
     */
    DrmConnector *findConnector(uint32_t id) const;

    /**
     * Returns the list of available crtcs on this device.
     */
    QVector<DrmCrtc *> crtcs() const;

    /**
     * Returns a CRTC with the given id.
     *
     * If there is no a such CRTC, @c null is returned.
     */
    DrmCrtc *findCrtc(uint32_t id) const;

    /**
     * Returns the list of available planes on this device.
     */
    QVector<DrmPlane *> planes() const;

    /**
     * Returns a list of outputs connected to this device.
     */
    QVector<DrmOutput *> outputs() const;

    /**
     * Returns an output that is driven by the given CRTC.
     */
    DrmOutput *findOutput(uint32_t crtcId) const;

    /**
     * Returns @c true if this device is currently idle, otherwise @c false.
     *
     * A device is considered to be idle if there are no pending flips.
     */
    bool isIdle() const;

    /**
     * Waits until this device becomes idle.
     *
     * If the device is already idle, then this method will return immediately,
     * otherwise it will run the event loop until the device becomes idle.
     *
     * @todo Resolve issues caused by running the event loop.
     */
    void waitIdle();

    /**
     * Returns @c true if this device is currently frozen, otherwise @c false.
     */
    bool isFrozen() const;

    /**
     *
     */
    void freeze();

    /**
     *
     */
    void unfreeze();

private Q_SLOTS:
    void dispatchEvents();

private:
    void scanConnectors();
    void scanCrtcs();
    void scanPlanes();
    void reroute();

    QString m_fileName;

    QVector<DrmConnector *> m_connectors;
    QVector<DrmCrtc *> m_crtcs;
    QVector<DrmPlane *> m_planes;
    QVector<DrmOutput *> m_outputs;

    DrmAllocator *m_allocator = nullptr;
    DrmContext *m_context = nullptr;

    int m_fd = -1;
    int m_freezeCount = 0;

    bool m_supportsDumbBuffer = false;
    bool m_supportsExportBuffer = false;
    bool m_supportsImportBuffer = false;
    bool m_supportsBufferModifier = false;
    bool m_isValid = false;

    friend class DrmDeviceManager;

    Q_DISABLE_COPY(DrmDevice)
};

} // namespace KWin
