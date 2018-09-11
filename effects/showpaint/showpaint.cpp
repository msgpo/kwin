/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2007 Lubos Lunak <l.lunak@kde.org>
Copyright (C) 2010 Martin Gräßlin <mgraesslin@kde.org>

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

#include "showpaint.h"

#include <kwinglutils.h>
#ifdef KWIN_HAVE_XRENDER_COMPOSITING
#include <xcb/render.h>
#endif

#include <QPainter>

namespace KWin
{

static const QVector<QColor> s_colors {
    Qt::red,
    Qt::green,
    Qt::blue,
    Qt::cyan,
    Qt::magenta,
    Qt::yellow,
    Qt::gray
};

ShowPaintEffect::ShowPaintEffect()
    : m_colorIndex(0)
{
}

ShowPaintEffect::~ShowPaintEffect()
{
}

void ShowPaintEffect::paintScreen(int mask, QRegion region, ScreenPaintData &data)
{
    m_painted = QRegion();
    effects->paintScreen(mask, region, data);
    if (effects->isOpenGLCompositing()) {
        paintGL(data.projectionMatrix());
    }
#ifdef KWIN_HAVE_XRENDER_COMPOSITING
    if (effects->compositingType() == XRenderCompositing) {
        paintXrender();
    }
#endif
    if (effects->compositingType() == QPainterCompositing) {
        paintQPainter();
    }
    if (++m_colorIndex == s_colors.count()) {
        m_colorIndex = 0;
    }
}

void ShowPaintEffect::paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data)
{
    m_painted |= region;
    effects->paintWindow(w, mask, region, data);
}

void ShowPaintEffect::paintGL(const QMatrix4x4 &projection)
{
    GLVertexBuffer *vbo = GLVertexBuffer::streamingBuffer();
    vbo->reset();
    vbo->setUseColor(true);
    ShaderBinder binder(ShaderTrait::UniformColor);
    binder.shader()->setUniform(GLShader::ModelViewProjectionMatrix, projection);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    QColor color = s_colors[m_colorIndex];
    color.setAlphaF(0.2);
    vbo->setColor(color);
    QVector<float> verts;
    verts.reserve(m_painted.rects().count() * 12);
    for (const QRect &r : m_painted) {
        verts << r.x() + r.width() << r.y();
        verts << r.x() << r.y();
        verts << r.x() << r.y() + r.height();
        verts << r.x() << r.y() + r.height();
        verts << r.x() + r.width() << r.y() + r.height();
        verts << r.x() + r.width() << r.y();
    }
    vbo->setData(verts.count() / 2, 2, verts.data(), NULL);
    vbo->render(GL_TRIANGLES);
    glDisable(GL_BLEND);
}

void ShowPaintEffect::paintXrender()
{
#ifdef KWIN_HAVE_XRENDER_COMPOSITING
    xcb_render_color_t col;
    float alpha = 0.2;
    const QColor& color = s_colors[m_colorIndex];
    col.alpha = int(alpha * 0xffff);
    col.red = int(alpha * 0xffff * color.red() / 255);
    col.green = int(alpha * 0xffff * color.green() / 255);
    col.blue = int(alpha * 0xffff * color.blue() / 255);
    QVector<xcb_rectangle_t> rects;
    for (const QRect &r : m_painted) {
        xcb_rectangle_t rect = {int16_t(r.x()), int16_t(r.y()), uint16_t(r.width()), uint16_t(r.height())};
        rects << rect;
    }
    xcb_render_fill_rectangles(xcbConnection(), XCB_RENDER_PICT_OP_OVER, effects->xrenderBufferPicture(), col, rects.count(), rects.constData());
#endif
}

void ShowPaintEffect::paintQPainter()
{
    QColor color = s_colors[m_colorIndex];
    color.setAlphaF(0.2);
    for (const QRect &r : m_painted) {
        effects->scenePainter()->fillRect(r, color);
    }
}

} // namespace KWin
