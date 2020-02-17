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

#include "subsurfacetreemonitor.h"

#include <KWayland/Server/subcompositor_interface.h>
#include <KWayland/Server/surface_interface.h>

using namespace KWayland::Server;

namespace KWin
{

/**
 * Creates a SubSurfaceTreeMonitor object with the specified toplevel surface @p surface.
 */
SubSurfaceTreeMonitor::SubSurfaceTreeMonitor(SurfaceInterface *surface, QObject *parent)
    : QObject(parent)
{
    registerSurface(surface);
}

void SubSurfaceTreeMonitor::registerSubSurface(SubSurfaceInterface *subSurface)
{
    SurfaceInterface *surface = subSurface->surface();

    connect(subSurface, &SubSurfaceInterface::positionChanged,
            this, &SubSurfaceTreeMonitor::subSurfaceMoved);
    connect(surface, &SurfaceInterface::sizeChanged,
            this, &SubSurfaceTreeMonitor::subSurfaceResized);

    registerSurface(surface);
}

void SubSurfaceTreeMonitor::unregisterSubSurface(SubSurfaceInterface *subSurface)
{
    SurfaceInterface *surface = subSurface->surface();
    if (!surface)
        return;

    disconnect(subSurface, &SubSurfaceInterface::positionChanged,
               this, &SubSurfaceTreeMonitor::subSurfaceMoved);
    disconnect(surface, &SurfaceInterface::sizeChanged,
               this, &SubSurfaceTreeMonitor::subSurfaceResized);

    unregisterSurface(surface);
}

void SubSurfaceTreeMonitor::registerSurface(SurfaceInterface *surface)
{
    connect(surface, &SurfaceInterface::childSubSurfaceAdded,
            this, &SubSurfaceTreeMonitor::subSurfaceAdded);
    connect(surface, &SurfaceInterface::childSubSurfaceRemoved,
            this, &SubSurfaceTreeMonitor::subSurfaceRemoved);
    connect(surface, &SurfaceInterface::childSubSurfaceAdded,
            this, &SubSurfaceTreeMonitor::registerSubSurface);
    connect(surface, &SurfaceInterface::childSubSurfaceRemoved,
            this, &SubSurfaceTreeMonitor::unregisterSubSurface);
}

void SubSurfaceTreeMonitor::unregisterSurface(SurfaceInterface *surface)
{
    disconnect(surface, &SurfaceInterface::childSubSurfaceAdded,
               this, &SubSurfaceTreeMonitor::subSurfaceAdded);
    disconnect(surface, &SurfaceInterface::childSubSurfaceRemoved,
               this, &SubSurfaceTreeMonitor::subSurfaceRemoved);
    disconnect(surface, &SurfaceInterface::childSubSurfaceAdded,
               this, &SubSurfaceTreeMonitor::registerSubSurface);
    disconnect(surface, &SurfaceInterface::childSubSurfaceRemoved,
               this, &SubSurfaceTreeMonitor::unregisterSubSurface);
}

/**
 * @class SubSurfaceTreeMonitor
 * @brief The SubSurfaceTreeMonitor class provides a convenient way for monitoring changes
 * in the sub-surface tree, e.g. addition or removal of sub-surfaces, etc.
 */

/**
 * @fn void SubSurfaceTreeMonitor::subSurfaceAdded()
 *
 * This signal is emitted when a new sub-surface is added to the sub-surface tree.
 */

/**
 * @fn void SubSurfaceTreeMonitor::subSurfaceRemoved()
 *
 * This signal is emitted when a sub-surface in the sub-surface tree has been removed.
 */

/**
 * @fn void SubSurfaceTreeMonitor::subSurfaceMoved()
 *
 * This signal is emitted when the position of a sub-surface in the tree has been changed.
 */

/**
 * @fn void SubSurfaceTreeMonitor::subSurfaceResized()
 *
 * This signal is emitted when the size of a sub-surface in the tree has been changed.
 *
 * Notice that this signal is not emitted when the main surface has been resized.
 */

} // namespace KWin
