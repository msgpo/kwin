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

#include "drm_device_manager.h"
#include "drm_device.h"

#include "toolkit/udev_enumerator.h"
#include "toolkit/udev_monitor.h"

#include <QScopedPointer>

namespace KWin
{

DrmDeviceManager::DrmDeviceManager(DrmContext *context, QObject *parent)
    : QObject(parent)
    , m_context(context)
{
    UdevContext udev;

    UdevEnumerator enumerator(udev);
    enumerator.matchSeat(QStringLiteral("seat0"));
    enumerator.matchSubsystem(QStringLiteral("drm"));
    enumerator.matchSysfsName(QStringLiteral("card[0-9]*"));

    const UdevDeviceList devices = enumerator.scan();

    for (const UdevDevice &device : devices) {
        if (device.types() & UdevDevice::PrimaryGpu) {
            slotDeviceAdded(device);
        }
        if (!m_devices.isEmpty()) {
            break;
        }
    }

    m_master = m_devices.first();

    for (const UdevDevice &device : devices) {
        if (!(device.types() & UdevDevice::PrimaryGpu)) {
            slotDeviceAdded(device);
        }
    }

    UdevMonitor *monitor = new UdevMonitor(&udev, this);
    monitor->filterBySubsystem(QStringLiteral("drm"));
    monitor->enable();

    connect(monitor, &UdevMonitor::deviceAdded, this, &DrmDeviceManager::slotDeviceAdded);
    connect(monitor, &UdevMonitor::deviceRemoved, this, &DrmDeviceManager::slotDeviceRemoved);
    connect(monitor, &UdevMonitor::deviceChanged, this, &DrmDeviceManager::slotDeviceChanged);
}

DrmDeviceManager::~DrmDeviceManager()
{
    for (DrmDevice *device : m_devices) {
        device->freeze();
    }

    for (DrmDevice *device : m_devices) {
        device->waitIdle();
    }
}

bool DrmDeviceManager::isValid() const
{
    return !m_devices.isEmpty();
}

QVector<DrmDevice *> DrmDeviceManager::devices() const
{
    return m_devices;
}

DrmDevice *DrmDeviceManager::master() const
{
    return m_master;
}

void DrmDeviceManager::slotDeviceAdded(const UdevDevice &udevDevice)
{
    if (!(udevDevice.types() & UdevDevice::Gpu)) {
        return;
    }

    const QString fileName = udevDevice.deviceNode();
    if (fileName.isEmpty()) {
        return;
    }

    QScopedPointer<DrmDevice> device(new DrmDevice(fileName, m_context, this));

    if (m_master) {
        if (!m_master->supports(DrmDevice::DeviceCapabilityExportBuffer)) {
            return;
        }
        if (!device->supports(DrmDevice::DeviceCapabilityImportBuffer)) {
            return;
        }
    }

    m_devices << device.take();
}

void DrmDeviceManager::slotDeviceRemoved(const UdevDevice &udevDevice)
{
    if (!(udevDevice.types() & UdevDevice::Gpu)) {
        return;
    }

    DrmDevice *device = findDevice(udevDevice);
    if (!device) {
        return;
    }

    m_devices.removeOne(device);
    delete device;
}

void DrmDeviceManager::slotDeviceChanged(const UdevDevice &udevDevice)
{
    if (!(udevDevice.types() & UdevDevice::Gpu)) {
        return;
    }

    DrmDevice *device = findDevice(udevDevice);
    if (!device) {
        return;
    }

    if (device->isFrozen()) {
        return;
    }

    device->waitIdle();
    device->scanConnectors();
}

DrmDevice *DrmDeviceManager::findDevice(const UdevDevice &udev)
{
    for (DrmDevice *device : m_devices) {
        if (device->fileName() == udev.deviceNode()) {
            return device;
        }
    }
    return nullptr;
}

} // namespace KWin
