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

class SceneOpenGL2;

class OpenGLWindow final : public Scene::Window
{
public:
    explicit OpenGLWindow(Toplevel *toplevel, SceneOpenGL2 *scene);
    ~OpenGLWindow() override;

    void performPaint(int mask, QRegion region, WindowPaintData data) override;

    WindowPixmap *createWindowPixmap() override;
    ShadowSceneNode *createShadowNode() override;
    DecorationSceneNode *createDecorationNode() override;
    SurfaceSceneNode *createSurfaceNode() override;

private:
    QMatrix4x4 modelViewProjectionMatrix(int mask, const WindowPaintData &data) const;
    QVector4D modulate(float opacity, float brightness) const;
    QMatrix4x4 transformation(int mask, const WindowPaintData &data) const;

    bool beginRenderWindow(int mask, const QRegion &region, WindowPaintData &data);
    void endRenderWindow();
    void setBlendEnabled(bool enabled);

    SceneOpenGL2 *m_scene;
    bool m_blendingEnabled = false;
    bool m_hardwareClipping = false;
};

} // namespace KWin
