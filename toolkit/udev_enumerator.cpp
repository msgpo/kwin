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

#include "udev_enumerator.h"

#include <QVector>

namespace KWin
{

UdevEnumerator::UdevEnumerator(const UdevContext &context)
    : m_context(context)
    , m_enumerate(udev_enumerate_new(context))
{
}

UdevEnumerator::UdevEnumerator(const UdevEnumerator &other)
    : m_context(other.m_context)
    , m_enumerate(udev_enumerate_ref(other.m_enumerate))
{
}

UdevEnumerator::UdevEnumerator(UdevEnumerator &&other)
    : m_context(std::move(other.m_context))
    , m_enumerate(std::exchange(other.m_enumerate, nullptr))
{
}

UdevEnumerator::~UdevEnumerator()
{
    if (m_enumerate) {
        udev_enumerate_unref(m_enumerate);
    }
}

UdevEnumerator &UdevEnumerator::operator=(const UdevEnumerator &other)
{
    if (m_enumerate) {
        udev_enumerate_unref(m_enumerate);
    }

    m_context = other.m_context;

    if (other.m_enumerate) {
        m_enumerate = udev_enumerate_ref(other.m_enumerate);
    } else {
        m_enumerate = nullptr;
    }

    return *this;
}

UdevEnumerator &UdevEnumerator::operator=(UdevEnumerator &&other)
{
    if (m_enumerate) {
        udev_enumerate_unref(m_enumerate);
    }

    m_context = std::move(other.m_context);
    m_enumerate = std::exchange(other.m_enumerate, nullptr);

    return *this;
}

bool UdevEnumerator::isValid() const
{
    return m_enumerate;
}

void UdevEnumerator::matchSeat(const QString &seat)
{
    m_seat = seat;
}

void UdevEnumerator::matchSubsystem(const QString &subsystem)
{
    if (m_enumerate) {
        udev_enumerate_add_match_subsystem(m_enumerate, subsystem.toUtf8());
    }
}

void UdevEnumerator::matchSysfsName(const QString &name)
{
    if (m_enumerate) {
        udev_enumerate_add_match_sysname(m_enumerate, name.toUtf8());
    }
}

UdevDeviceList UdevEnumerator::scan() const
{
    if (!m_enumerate) {
        return UdevDeviceList();
    }

    if (udev_enumerate_scan_devices(m_enumerate)) {
        return UdevDeviceList();
    }

    UdevDeviceList devices;

    for (auto it = udev_enumerate_get_list_entry(m_enumerate);
         it;
         it = udev_list_entry_get_next(it)) {
        const QString sysfsPath = QString::fromUtf8(udev_list_entry_get_name(it));
        const UdevDevice device = m_context.deviceFromSysfsPath(sysfsPath);
        if (!device.isValid()) {
            continue;
        }
        if (device.seat() != m_seat) {
            continue;
        }

        devices.append(device);
    }

    return devices;
}

} // namespace KWin
