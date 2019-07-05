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

#include "drm_allocator.h"

namespace KWin
{

class DrmDumbAllocator : public DrmAllocator
{
    Q_OBJECT

public:
    explicit DrmDumbAllocator(DrmDevice *device, QObject *parent = nullptr);
    ~DrmDumbAllocator() override;

    bool isValid() const override;
    DrmImage *allocate(uint32_t width, uint32_t height, uint32_t format,
        const QVector<uint64_t> &modifiers) override;

private:
    Q_DISABLE_COPY(DrmDumbAllocator)
};

} // namespace KWin
