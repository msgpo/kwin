/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006 Lubos Lunak <l.lunak@kde.org>
Copyright (C) 2009, 2010, 2011 Martin Gräßlin <mgraesslin@kde.org>
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

class SceneOpenGL;
class SceneOpenGLTexture;

class OpenGLWindowPixmap final : public WindowPixmap
{
public:
    explicit OpenGLWindowPixmap(Scene::Window *window, SceneOpenGL *scene);
    ~OpenGLWindowPixmap() override;

    SceneOpenGLTexture *texture() const;
    bool bind();
    bool isValid() const override;

private:
    QScopedPointer<SceneOpenGLTexture> m_texture;
    SceneOpenGL *m_scene;
};

} // namespace KWin
