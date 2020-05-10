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

#include "kwin_export.h"

#include <QImage>

namespace KWin
{

class GLTexture;
class Toplevel;

class KWIN_EXPORT BufferPrivate
{
public:
    virtual ~BufferPrivate() {}

    virtual bool create() = 0;
    virtual void update() = 0;

    Toplevel *toplevel = nullptr;
    GLTexture *texture = nullptr;
    QImage image;
};

} // namespace KWin
