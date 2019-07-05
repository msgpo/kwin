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

#include "drm_allocator.h"

namespace KWin
{

DrmAllocator::DrmAllocator(DrmDevice *device, QObject *parent)
    : QObject(parent)
    , m_device(device)
{
}

DrmAllocator::~DrmAllocator()
{
}

DrmDevice *DrmAllocator::device() const
{
    return m_device;
}

} // namespace KWin
