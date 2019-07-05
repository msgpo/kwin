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

class DrmContext;
class DrmDevice;
class UdevDevice;

/**
 * The DrmDeviceManager class is responsible for managing DRM devices.
 *
 * @todo More documentation.
 */
class DrmDeviceManager : public QObject
{
    Q_OBJECT

public:
    explicit DrmDeviceManager(DrmContext *context, QObject *parent = nullptr);
    ~DrmDeviceManager() override;

    /**
     * Returns @c true if the DRM device manager is valid.
     */
    bool isValid() const;

    /**
     * Returns all managed DRM devices.
     */
    QVector<DrmDevice *> devices() const;

    /**
     * Returns the primary DRM device.
     *
     * @todo Write some stuff about PRIME.
     */
    DrmDevice *master() const;

private Q_SLOTS:
    void slotDeviceAdded(const UdevDevice &device);
    void slotDeviceRemoved(const UdevDevice &device);
    void slotDeviceChanged(const UdevDevice &device);

private:
    DrmDevice *findDevice(const UdevDevice &udev);

    QVector<DrmDevice *> m_devices;
    DrmContext *m_context = nullptr;
    DrmDevice *m_master = nullptr;

    Q_DISABLE_COPY(DrmDeviceManager)
};

} // namespace KWin
