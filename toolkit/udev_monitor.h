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

#include <QObject>

namespace KWin
{

/**
 * The UdevMonitor class represents an udev monitor.
 */
class KWIN_EXPORT UdevMonitor : public QObject
{
    Q_OBJECT

public:
    explicit UdevMonitor(UdevContext *context, QObject *parent = nullptr);
    ~UdevMonitor() override;

    /**
     * Returns @c true if the udev monitor is valid, otherwise @c false.
     */
    bool isValid() const;

    /**
     * Enables receiving events.
     */
    void enable();

    /**
     * Adds a subsytem filter.
     */
    void filterBySubsystem(const QString &subsystem);

Q_SIGNALS:
    /**
     * Emitted when a new device is added.
     */
    void deviceAdded(const UdevDevice &device);

    /**
     * Emitted when a device gets removed.
     */
    void deviceRemoved(const UdevDevice &device);

    /**
     * Emitted when a device is changed, e.g. an output was attached to a graphics
     * card, etc.
     */
    void deviceChanged(const UdevDevice &device);

private Q_SLOTS:
    void dispatchEvents();

private:
    UdevContext m_context;
    udev_monitor *m_monitor;
    bool m_isEnabled = false;

    Q_DISABLE_COPY(UdevMonitor)
};

} // namespace KWin
