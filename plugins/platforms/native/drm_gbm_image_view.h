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

#include "drm_image_view.h"

namespace KWin
{

class DrmGbmImage;

class DrmGbmImageView : public DrmImageView
{
public:
    explicit DrmGbmImageView(DrmGbmImage *image);
    ~DrmGbmImageView() override;

    bool isValid() const override;

private:
    Q_DISABLE_COPY(DrmGbmImageView)
};

} // namespace KWin
