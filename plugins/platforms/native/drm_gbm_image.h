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

#include "drm_image.h"

#include <gbm.h>

namespace KWin
{

class DrmDevice;

class DrmGbmImage : public DrmImage
{
public:
    DrmGbmImage(DrmDevice *device, gbm_bo *nativeHandle);
    ~DrmGbmImage() override;

    bool isValid() const override;
    DrmBuffer *buffer() const override;

private:
    DrmBuffer *m_buffer = nullptr;
    gbm_bo *m_nativeHandle = nullptr;

    Q_DISABLE_COPY(DrmGbmImage)
};

} // namespace KWin
