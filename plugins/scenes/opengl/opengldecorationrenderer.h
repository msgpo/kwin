/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

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

#include "decorations/decorationrenderer.h"

namespace KWin
{

class GLTexture;

class OpenGLDecorationRenderer final : public Decoration::Renderer
{
    Q_OBJECT

public:
    enum class DecorationPart : int {
        Left,
        Top,
        Right,
        Bottom,
        Count
    };
    explicit OpenGLDecorationRenderer(Decoration::DecoratedClientImpl *client);
    ~OpenGLDecorationRenderer() override;

    void render() override;
    void reparent(Deleted *deleted) override;

    GLTexture *texture() const;

private:
    void resizeTexture();
    QScopedPointer<GLTexture> m_texture;
};

} // namespace KWin
