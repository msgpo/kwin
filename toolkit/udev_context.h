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

#include <QString>

#include <libudev.h>

namespace KWin
{

class UdevDevice;

/**
 * The UdevContext class represents an udev contenxt.
 */
class KWIN_EXPORT UdevContext
{
public:
    explicit UdevContext();
    UdevContext(const UdevContext &other);
    UdevContext(UdevContext &&other);
    ~UdevContext();

    UdevContext &operator=(const UdevContext &other);
    UdevContext &operator=(UdevContext &&other);

    /**
     * Returns @c true if the udev context is valid, otherwise @c false.
     */
    bool isValid() const;

    /**
     * Creates an UdevDevice, the udev device is looked by its absolute path.
     *
     * If there is no such a device, an invalid udev device is returned.
     */
    UdevDevice deviceFromFileName(const QString &fileName) const;

    /**
     * Creates an UdevDevice from a device id string.
     *
     * If there is no such a device, an invalid udev device is returned.
     */
    UdevDevice deviceFromId(const QString &id) const;

    /**
     * Creates an UdevDevice, the udev device is looked up by the @p subSystem
     * and @name string of the device.
     *
     * If there is no such a device, an invalid udev device is returned.
     */
    UdevDevice deviceFromSubSystemAndName(const QString &subSystem, const QString &name) const;

    /**
     * Create an UdevDevice from sysfs path. The syspath is an absolute path
     * to the device.
     *
     * If there is no such a device, an invalid udev device is returned.
     */
    UdevDevice deviceFromSysfsPath(const QString &path) const;

    operator udev*() const;

private:
    udev *m_udev;
};

} // namespace KWin
