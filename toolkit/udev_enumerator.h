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

#include "udev_context.h"
#include "udev_device.h"

namespace KWin
{

/**
 * The UdevEnumerator class represents an udev enumerator object.
 *
 * The udev enumerator object can be used to enumerate all devices that match
 * some certain criterias.
 */
class KWIN_EXPORT UdevEnumerator
{
public:
    explicit UdevEnumerator(const UdevContext &context);
    UdevEnumerator(const UdevEnumerator &other);
    UdevEnumerator(UdevEnumerator &&other);
    ~UdevEnumerator();

    UdevEnumerator &operator=(const UdevEnumerator &other);
    UdevEnumerator &operator=(UdevEnumerator &&other);

    /**
     * Returns @c true if the udev enumerator is valid, otherwise @c false.
     */
    bool isValid() const;

    /**
     * Adds a seat filter.
     *
     * Match only devices belonging to the given @p seat.
     */
    void matchSeat(const QString &seat);

    /**
     * Adds a subsystem filter.
     *
     * Match only devices belonging to the given @p subsystem.
     */
    void matchSubsystem(const QString &subsystem);

    /**
     * Adds a sysfs device name filter.
     *
     * Match only devices with a given sysfs device name.
     */
    void matchSysfsName(const QString &name);

    /**
     * Scans /sys for all devices that match the given filters.
     *
     * If no filters were specified, all currently available devices will be listed.
     */
    UdevDeviceList scan() const;

private:
    UdevContext m_context;
    udev_enumerate *m_enumerate;
    QString m_seat;
};

} // namespace KWin
