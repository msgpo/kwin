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

#include "udev_monitor.h"
#include "udev_device.h"

#include <QSocketNotifier>

namespace KWin
{

UdevMonitor::UdevMonitor(UdevContext *context, QObject *parent)
    : QObject(parent)
    , m_context(*context)
    , m_monitor(udev_monitor_new_from_netlink(*context, "udev"))
{
}

UdevMonitor::~UdevMonitor()
{
    if (m_monitor) {
        udev_monitor_unref(m_monitor);
    }
}

bool UdevMonitor::isValid() const
{
    return m_monitor;
}

void UdevMonitor::enable()
{
    if (m_isEnabled) {
        return;
    }

    udev_monitor_enable_receiving(m_monitor);

    const int fd = udev_monitor_get_fd(m_monitor);
    QSocketNotifier *notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, &UdevMonitor::dispatchEvents);

    m_isEnabled = true;
}

void UdevMonitor::filterBySubsystem(const QString &subsystem)
{
    if (!m_monitor) {
        return;
    }

    udev_monitor_filter_add_match_subsystem_devtype(m_monitor, subsystem.toUtf8(), nullptr);

    if (m_isEnabled) {
        udev_monitor_filter_update(m_monitor);
    }
}

void UdevMonitor::dispatchEvents()
{
    UdevDevice device(udev_monitor_receive_device(m_monitor));
    if (!device.isValid()) {
        return;
    }

    const char *action = udev_device_get_action(device);
    if (!action) {
        return;
    }

    if (!qstrcmp(action, "add")) {
        emit deviceAdded(device);
    } else if (!qstrcmp(action, "remove")) {
        emit deviceRemoved(device);
    } else if (!qstrcmp(action, "change")) {
        emit deviceChanged(device);
    }
}

} // namespace Carbon
