/*
 * Copyright (C) 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QObject>

namespace KWayland
{
namespace Server
{
class SubSurfaceInterface;
class SurfaceInterface;
}
}

namespace KWin
{

class SubSurfaceTreeMonitor : public QObject
{
    Q_OBJECT

public:
    SubSurfaceTreeMonitor(KWayland::Server::SurfaceInterface *surface, QObject *parent);

Q_SIGNALS:
    void subSurfaceAdded();
    void subSurfaceRemoved();
    void subSurfaceMoved();
    void subSurfaceResized();

private:
    void registerSubSurface(KWayland::Server::SubSurfaceInterface *subSurface);
    void unregisterSubSurface(KWayland::Server::SubSurfaceInterface *subSurface);
    void registerSurface(KWayland::Server::SurfaceInterface *surface);
    void unregisterSurface(KWayland::Server::SurfaceInterface *surface);
};

} // namespace KWin
