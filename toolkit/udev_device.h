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

#include "kwin_export.h"

#include <QByteArray>

#include <libudev.h>

namespace KWin
{

/**
 * The UdevDevice class represents an udev device.
 */
class KWIN_EXPORT UdevDevice
{
public:
    /**
     * This enum type is used to specify type of this udev device.
     */
    enum Type {
        Unknown,
        Gpu = 1 << 0,
        PrimaryGpu = 1 << 1,
        FrameBuffer = 1 << 2,
        PrimaryFrameBuffer = 1 << 3,
    };
    Q_DECLARE_FLAGS(Types, Type)

    explicit UdevDevice();
    UdevDevice(const UdevDevice &other);
    UdevDevice(UdevDevice &&other);
    ~UdevDevice();

    UdevDevice &operator=(const UdevDevice &other);
    UdevDevice &operator=(UdevDevice &&other);

    /**
     * Returns @c true if the udev device is valid, otherwise @c false.
     */
    bool isValid() const;

    /**
     * Returns the type of the udev device.
     */
    Types types() const;

    /**
     * Returns the parent udev device.
     */
    UdevDevice parent() const;

    /**
     * Returns the sysfs path of the udev device. The path is an absolute path.
     */
    QString sysfsPath() const;

    /**
     * Returns the kernel device name in /sys.
     */
    QString sysfsName() const;

    /**
     * Returns the instance number of the udev device.
     */
    QString sysfsNumber() const;

    /**
     * Returns the kernel device path of the udev device.
     */
    QString devicePath() const;

    /**
     * Returns the device node file name of the udev device. The returned value
     * is an absolute path, and starts with the /dev directory.
     */
    QString deviceNode() const;

    /**
     * Returns the device major/minor number.
     */
    dev_t deviceNumber() const;

    /**
     * Returns the kernel driver name.
     */
    QString driver() const;

    /**
     * Returns the seat of the udev device.
     */
    QString seat() const;

    /**
     * Returns the subsystem string of the udev device.
     */
    QString subsystem() const;

    /**
     * Returns the value of a property with the given @p name.
     */
    QByteArray property(const QString &name) const;

    operator udev_device*() const;

private:
    UdevDevice(udev_device *device);

    udev_device *m_device;

    friend class UdevContext;
    friend class UdevMonitor;
};

typedef QVector<UdevDevice> UdevDeviceList;

} // namespace KWin
