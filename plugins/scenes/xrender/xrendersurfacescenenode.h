/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2019 Vlad Zahorodnii <vladzzag@gmail.com>

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

#include "scene.h"

namespace KWin
{

class XRenderSurfaceSceneNode : public SurfaceSceneNode
{
public:
    ~XRenderSurfaceSceneNode() override;

    xcb_render_picture_t picture() const;

protected:
    void updateTexture() override;
    void updateQuads() override;

private:
    xcb_render_picture_t m_picture = XCB_RENDER_PICTURE_NONE;
};

} // namespace KWin
