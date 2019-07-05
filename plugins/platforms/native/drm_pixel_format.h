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

#include <cstdint>

namespace KWin
{

class DrmPixelFormat
{
public:
    DrmPixelFormat(uint32_t format);

    /**
     * Returns the color depth, i.e. the number of pits per a single pixel.
     *
     * The returned value is valid only for a subset of RGB formats.
     */
    int bitsPerPixel() const;

    /**
     * Returns the number of color planes in the format (1 to 3).
     */
    int planeCount() const;

private:
    int m_bitsPerPixel;
    int m_planeCount;
};

} // namespace KWin
