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

#include <QObject>

namespace KWin
{

class DrmDevice;
class DrmImage;

/**
 * The DrmAllocator class ...
 */
class DrmAllocator : public QObject
{
    Q_OBJECT

public:
    explicit DrmAllocator(DrmDevice *device, QObject *parent = nullptr);
    ~DrmAllocator() override;

    /**
     * Returns DRM device associated with this allocator.
     */
    DrmDevice *device() const;

    /**
     * Returns @c true if this allocator is valid, otherwise @c false.
     */
    virtual bool isValid() const = 0;

    /**
     * Allocates an image.
     */
    virtual DrmImage *allocate(uint32_t width, uint32_t height, uint32_t format,
        const QVector<uint64_t> &modifiers) = 0;

private:
    DrmDevice *m_device;

    Q_DISABLE_COPY(DrmAllocator)
};

} // namespace KWin
