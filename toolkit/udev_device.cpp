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

#include "udev_device.h"
#include "udev_context.h"

namespace KWin
{

UdevDevice::UdevDevice()
    : m_device(nullptr)
{
}

UdevDevice::UdevDevice(udev_device *device)
    : m_device(device)
{
}

UdevDevice::UdevDevice(const UdevDevice &other)
    : m_device(udev_device_ref(other.m_device))
{
}

UdevDevice::UdevDevice(UdevDevice &&other)
    : m_device(std::exchange(other.m_device, nullptr))
{
}

UdevDevice::~UdevDevice()
{
    if (m_device) {
        udev_device_unref(m_device);
    }
}

UdevDevice &UdevDevice::operator=(const UdevDevice &other)
{
    if (m_device) {
        udev_device_unref(m_device);
    }

    if (other.m_device) {
        m_device = udev_device_ref(other.m_device);
    } else {
        m_device = nullptr;
    }

    return *this;
}

UdevDevice &UdevDevice::operator=(UdevDevice &&other)
{
    if (m_device) {
        udev_device_unref(m_device);
    }

    m_device = std::exchange(other.m_device, nullptr);

    return *this;
}

bool UdevDevice::isValid() const
{
    return m_device;
}

static bool isPrimaryDevice(udev_device *device)
{
    udev_device *pci = udev_device_get_parent_with_subsystem_devtype(device, "pci", nullptr);
    if (!pci) {
        return false;
    }

    const char *value = udev_device_get_sysattr_value(pci, "boot_vga");
    if (!value) {
        return false;
    }

    return value == QByteArrayLiteral("1");
}

UdevDevice::Types UdevDevice::types() const
{
    if (!m_device) {
        return Unknown;
    }

    const char *subsystem = udev_device_get_subsystem(m_device);

    Types types = Unknown;

    if (subsystem == QByteArrayLiteral("drm")) {
        if (isPrimaryDevice(m_device)) {
            types |= PrimaryGpu;
        }
        types |= Gpu;
    }

    if (subsystem == QByteArrayLiteral("graphics")) {
        if (isPrimaryDevice(m_device)) {
            types |= PrimaryFrameBuffer;
        }
        types |= FrameBuffer;
    }

    return types;
}

UdevDevice UdevDevice::parent() const
{
    if (m_device) {
        return udev_device_get_parent(m_device);
    }
    return nullptr;
}

QString UdevDevice::sysfsPath() const
{
    if (m_device) {
        return QString::fromUtf8(udev_device_get_syspath(m_device));
    }
    return QString();
}

QString UdevDevice::sysfsName() const
{
    if (m_device) {
        return QString::fromUtf8(udev_device_get_sysname(m_device));
    }
    return QString();
}

QString UdevDevice::sysfsNumber() const
{
    if (m_device) {
        return QString::fromUtf8(udev_device_get_sysnum(m_device));
    }
    return QString();
}

QString UdevDevice::devicePath() const
{
    if (m_device) {
        return QString::fromUtf8(udev_device_get_devpath(m_device));
    }
    return QString();
}

QString UdevDevice::deviceNode() const
{
    if (m_device) {
        return QString::fromUtf8(udev_device_get_devnode(m_device));
    }
    return QString();
}

dev_t UdevDevice::deviceNumber() const
{
    if (m_device) {
        return udev_device_get_devnum(m_device);
    }
    return -1;
}

QString UdevDevice::driver() const
{
    if (m_device) {
        return QString::fromUtf8(udev_device_get_driver(m_device));
    }
    return QString();
}

QString UdevDevice::seat() const
{
    if (!m_device) {
        return QString();
    }

    QString seat = QString::fromLatin1(udev_device_get_property_value(m_device, "ID_SEAT"));
    if (seat.isEmpty()) {
        return QStringLiteral("seat0");
    }

    return seat;
}

QString UdevDevice::subsystem() const
{
    if (m_device) {
        return QString::fromUtf8(udev_device_get_subsystem(m_device));
    }
    return QString();
}

QByteArray UdevDevice::property(const QString &name) const
{
    if (m_device) {
        return udev_device_get_property_value(m_device, name.toUtf8());
    }
    return QByteArray();
}

UdevDevice::operator udev_device*() const
{
    return m_device;
}

} // namespace KWin
