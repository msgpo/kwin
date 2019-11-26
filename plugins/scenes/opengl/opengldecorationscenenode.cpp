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

#include "opengldecorationscenenode.h"

namespace KWin
{

OpenGLDecorationSceneNode::OpenGLDecorationSceneNode(Toplevel *toplevel)
    : DecorationSceneNode(toplevel)
{
}

#if 0
GLTexture *SceneOpenGL::Window::getDecorationTexture() const
{
    if (AbstractClient *client = dynamic_cast<AbstractClient *>(toplevel)) {
        if (client->noBorder()) {
            return nullptr;
        }

        if (!client->isDecorated()) {
            return nullptr;
        }
        if (OpenGLDecorationRenderer *renderer = static_cast<OpenGLDecorationRenderer*>(client->decoratedClient()->renderer())) {
            renderer->render();
            return renderer->texture();
        }
    } else if (toplevel->isDeleted()) {
        Deleted *deleted = static_cast<Deleted *>(toplevel);
        if (!deleted->wasClient() || deleted->noBorder()) {
            return nullptr;
        }
        if (const OpenGLDecorationRenderer *renderer = static_cast<const OpenGLDecorationRenderer*>(deleted->decorationRenderer())) {
            return renderer->texture();
        }
    }
    return nullptr;
}
#endif

void OpenGLDecorationSceneNode::updateTexture()
{
}

GLTexture *OpenGLDecorationSceneNode::texture() const
{
    return nullptr;
}

} // namespace KWin
