/*
 *   Copyright © 2010 Fredrik Höglund <fredrik@kde.org>
 *   Copyright © 2011 Philipp Knechtges <philipp-dev@knechtges.com>
 *   Copyright 2014 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; see the file COPYING.  if not, write to
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301, USA.
 */

#include "contrast.h"
#include "contrastshader.h"
// KConfigSkeleton

#include <QMatrix4x4>
#include <QLinkedList>

#include <KWayland/Server/surface_interface.h>
#include <KWayland/Server/contrast_interface.h>
#include <KWayland/Server/display.h>

namespace KWin
{

static const QByteArray s_contrastAtomName = QByteArrayLiteral("_KDE_NET_WM_BACKGROUND_CONTRAST_REGION");

ContrastEffect::ContrastEffect()
{
    shader = ContrastShader::create();

    reconfigure(ReconfigureAll);

    // ### Hackish way to announce support.
    //     Should be included in _NET_SUPPORTED instead.
    if (shader && shader->isValid()) {
        net_wm_contrast_region = effects->announceSupportProperty(s_contrastAtomName, this);
        KWayland::Server::Display *display = effects->waylandDisplay();
        if (display) {
            m_contrastManager = display->createContrastManager(this);
            m_contrastManager->create();
        }
    } else {
        net_wm_contrast_region = 0;
    }

    connect(effects, SIGNAL(windowAdded(KWin::EffectWindow*)), this, SLOT(slotWindowAdded(KWin::EffectWindow*)));
    connect(effects, SIGNAL(windowDeleted(KWin::EffectWindow*)), this, SLOT(slotWindowDeleted(KWin::EffectWindow*)));
    connect(effects, SIGNAL(propertyNotify(KWin::EffectWindow*,long)), this, SLOT(slotPropertyNotify(KWin::EffectWindow*,long)));
    connect(effects, SIGNAL(screenGeometryChanged(QSize)), this, SLOT(slotScreenGeometryChanged()));
    connect(effects, &EffectsHandler::xcbConnectionChanged, this,
        [this] {
            if (shader && shader->isValid()) {
                net_wm_contrast_region = effects->announceSupportProperty(s_contrastAtomName, this);
            }
        }
    );

    // Fetch the contrast regions for all windows
    for (EffectWindow *window: effects->stackingOrder()) {
        updateContrastRegion(window);
    }
}

ContrastEffect::~ContrastEffect()
{
    delete shader;
}

void ContrastEffect::slotScreenGeometryChanged()
{
    effects->makeOpenGLContextCurrent();
    if (!supported()) {
        effects->reloadEffect(this);
        return;
    }
    for (EffectWindow *window: effects->stackingOrder()) {
        updateContrastRegion(window);
    }
}

void ContrastEffect::reconfigure(ReconfigureFlags flags)
{
    Q_UNUSED(flags)

    if (shader)
        shader->init();

    if (!shader || !shader->isValid()) {
        effects->removeSupportProperty(s_contrastAtomName, this);
        delete m_contrastManager;
        m_contrastManager = nullptr;
    }
}

void ContrastEffect::updateContrastRegion(EffectWindow *w)
{
    QRegion region;
    float colorTransform[16];
    QByteArray value;

    if (net_wm_contrast_region != XCB_ATOM_NONE) {
        value = w->readProperty(net_wm_contrast_region, net_wm_contrast_region, 32);

        if (value.size() > 0 && !((value.size() - (16 * sizeof(uint32_t))) % ((4 * sizeof(uint32_t))))) {
            const uint32_t *cardinals = reinterpret_cast<const uint32_t*>(value.constData());
            const float *floatCardinals = reinterpret_cast<const float*>(value.constData());
            unsigned int i = 0;
            for (; i < ((value.size() - (16 * sizeof(uint32_t)))) / sizeof(uint32_t);) {
                int x = cardinals[i++];
                int y = cardinals[i++];
                int w = cardinals[i++];
                int h = cardinals[i++];
                region += QRect(x, y, w, h);
            }

            for (unsigned int j = 0; j < 16; ++j) {
                colorTransform[j] = floatCardinals[i + j];
            }

            QMatrix4x4 colorMatrix(colorTransform);
            m_colorMatrices[w] = colorMatrix;
        }
    }

    KWayland::Server::SurfaceInterface *surf = w->surface();

    if (surf && surf->contrast()) {
        region = surf->contrast()->region();
        m_colorMatrices[w] = colorMatrix(surf->contrast()->contrast(), surf->contrast()->intensity(), surf->contrast()->saturation());
    }

    //!value.isNull() full window in X11 case, surf->contrast()
    //valid, full window in wayland case
    if (region.isEmpty() && (!value.isNull() || (surf && surf->contrast()))) {
        // Set the data to a dummy value.
        // This is needed to be able to distinguish between the value not
        // being set, and being set to an empty region.
        w->setData(WindowBackgroundContrastRole, 1);
    } else
        w->setData(WindowBackgroundContrastRole, region);
}

void ContrastEffect::slotWindowAdded(EffectWindow *w)
{
    KWayland::Server::SurfaceInterface *surf = w->surface();

    if (surf) {
        m_contrastChangedConnections[w] = connect(surf, &KWayland::Server::SurfaceInterface::contrastChanged, this, [this, w] () {

            if (w) {
                updateContrastRegion(w);
            }
        });
    }
    updateContrastRegion(w);
}

void ContrastEffect::slotWindowDeleted(EffectWindow *w)
{
    if (m_contrastChangedConnections.contains(w)) {
        disconnect(m_contrastChangedConnections[w]);
        m_contrastChangedConnections.remove(w);
        m_colorMatrices.remove(w);
    }
}

void ContrastEffect::slotPropertyNotify(EffectWindow *w, long atom)
{
    if (w && atom == net_wm_contrast_region && net_wm_contrast_region != XCB_ATOM_NONE) {
        updateContrastRegion(w);
    }
}

QMatrix4x4 ContrastEffect::colorMatrix(qreal contrast, qreal intensity, qreal saturation)
{
    QMatrix4x4 satMatrix; //saturation
    QMatrix4x4 intMatrix; //intensity
    QMatrix4x4 contMatrix; //contrast

    //Saturation matrix
    if (!qFuzzyCompare(saturation, 1.0)) {
        const qreal rval = (1.0 - saturation) * .2126;
        const qreal gval = (1.0 - saturation) * .7152;
        const qreal bval = (1.0 - saturation) * .0722;

        satMatrix = QMatrix4x4(rval + saturation, rval,     rval,     0.0,
            gval,     gval + saturation, gval,     0.0,
            bval,     bval,     bval + saturation, 0.0,
            0,        0,        0,        1.0);
    }

    //IntensityMatrix
    if (!qFuzzyCompare(intensity, 1.0)) {
        intMatrix.scale(intensity, intensity, intensity);
    }

    //Contrast Matrix
    if (!qFuzzyCompare(contrast, 1.0)) {
        const float transl = (1.0 - contrast) / 2.0;

        contMatrix = QMatrix4x4(contrast, 0,        0,        0.0,
            0,        contrast, 0,        0.0,
            0,        0,        contrast, 0.0,
            transl,   transl,   transl,   1.0);
    }

    QMatrix4x4 colorMatrix = contMatrix * satMatrix * intMatrix;
    //colorMatrix = colorMatrix.transposed();

    return colorMatrix;
}

bool ContrastEffect::enabledByDefault()
{
    GLPlatform *gl = GLPlatform::instance();

    if (gl->isIntel() && gl->chipClass() < SandyBridge)
        return false;
    if (gl->isSoftwareEmulation()) {
        return false;
    }

    return true;
}

bool ContrastEffect::supported()
{
    bool supported = effects->isOpenGLCompositing() && GLRenderTarget::supported();

    if (supported) {
        int maxTexSize;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);

        const QSize screenSize = effects->virtualScreenSize();
        if (screenSize.width() > maxTexSize || screenSize.height() > maxTexSize)
            supported = false;
    }
    return supported;
}

QRegion ContrastEffect::contrastRegion(const EffectWindow *w) const
{
    QRegion region;

    const QVariant value = w->data(WindowBackgroundContrastRole);
    if (value.isValid()) {
        const QRegion appRegion = qvariant_cast<QRegion>(value);
        if (!appRegion.isEmpty()) {
            region |= appRegion.translated(w->contentsRect().topLeft()) &
                      w->decorationInnerRect();
        } else {
            // An empty region means that the blur effect should be enabled
            // for the whole window.
            region = w->decorationInnerRect();
        }
    }

    return region;
}

void ContrastEffect::uploadRegion(QVector2D *&map, const QRegion &region)
{
    for (const QRect &r : region) {
        const QVector2D topLeft(r.x(), r.y());
        const QVector2D topRight(r.x() + r.width(), r.y());
        const QVector2D bottomLeft(r.x(), r.y() + r.height());
        const QVector2D bottomRight(r.x() + r.width(), r.y() + r.height());

        // First triangle
        *(map++) = topRight;
        *(map++) = topLeft;
        *(map++) = bottomLeft;

        // Second triangle
        *(map++) = bottomLeft;
        *(map++) = bottomRight;
        *(map++) = topRight;
    }
}

void ContrastEffect::uploadGeometry(GLVertexBuffer *vbo, const QRegion &region)
{
    const int vertexCount = region.rectCount() * 6;
    if (!vertexCount)
        return;

    QVector2D *map = (QVector2D *) vbo->map(vertexCount * sizeof(QVector2D));
    uploadRegion(map, region);
    vbo->unmap();

    const GLVertexAttrib layout[] = {
        { VA_Position, 2, GL_FLOAT, 0 },
        { VA_TexCoord, 2, GL_FLOAT, 0 }
    };

    vbo->setAttribLayout(layout, 2, sizeof(QVector2D));
}

void ContrastEffect::prePaintScreen(ScreenPrePaintData &data, int time)
{
    m_paintedArea = QRegion();
    m_currentContrast = QRegion();

    effects->prePaintScreen(data, time);
}

void ContrastEffect::prePaintWindow(EffectWindow* w, WindowPrePaintData& data, int time)
{
    // this effect relies on prePaintWindow being called in the bottom to top order

    effects->prePaintWindow(w, data, time);

    if (!w->isPaintingEnabled()) {
        return;
    }
    if (!shader || !shader->isValid()) {
        return;
    }

    const QRegion oldPaint = data.paint;

    // we don't have to blur a region we don't see
    m_currentContrast -= data.clip;
    // if we have to paint a non-opaque part of this window that intersects with the
    // currently blurred region (which is not cached) we have to redraw the whole region
    if ((data.paint-data.clip).intersects(m_currentContrast)) {
        data.paint |= m_currentContrast;
    }

    // in case this window has regions to be blurred
    const QRect screen = effects->virtualScreenGeometry();
    const QRegion contrastArea = contrastRegion(w).translated(w->pos()) & screen;

    // we are not caching the window

    // if this window or an window underneath the modified area is painted again we have to
    // do everything
    if (m_paintedArea.intersects(contrastArea) || data.paint.intersects(contrastArea)) {
        data.paint |= contrastArea;

        // we have to check again whether we do not damage a blurred area
        // of a window we do not cache
        if (contrastArea.intersects(m_currentContrast)) {
            data.paint |= m_currentContrast;
        }
    }

    m_currentContrast |= contrastArea;


    // m_paintedArea keep track of all repainted areas
    m_paintedArea -= data.clip;
    m_paintedArea |= data.paint;
}

bool ContrastEffect::shouldContrast(const EffectWindow *w, int mask, const WindowPaintData &data) const
{
    if (!shader || !shader->isValid())
        return false;

    if (effects->activeFullScreenEffect() && !w->data(WindowForceBackgroundContrastRole).toBool())
        return false;

    if (w->isDesktop())
        return false;

    bool scaled = !qFuzzyCompare(data.xScale(), 1.0) && !qFuzzyCompare(data.yScale(), 1.0);
    bool translated = data.xTranslation() || data.yTranslation();

    if ((scaled || (translated || (mask & PAINT_WINDOW_TRANSFORMED))) && !w->data(WindowForceBackgroundContrastRole).toBool())
        return false;

    if (!w->hasAlpha())
        return false;

    return true;
}

void ContrastEffect::drawWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data)
{
    const QRect screen = GLRenderTarget::virtualScreenGeometry();
    if (shouldContrast(w, mask, data)) {
        QRegion shape = region & contrastRegion(w).translated(w->pos()) & screen;

        // let's do the evil parts - someone wants to blur behind a transformed window
        const bool translated = data.xTranslation() || data.yTranslation();
        const bool scaled = data.xScale() != 1 || data.yScale() != 1;
        if (scaled) {
            QPoint pt = shape.boundingRect().topLeft();
            QVector<QRect> shapeRects = shape.rects();
            shape = QRegion(); // clear
            foreach (QRect r, shapeRects) {
                r.moveTo(pt.x() + (r.x() - pt.x()) * data.xScale() + data.xTranslation(),
                            pt.y() + (r.y() - pt.y()) * data.yScale() + data.yTranslation());
                r.setWidth(r.width() * data.xScale());
                r.setHeight(r.height() * data.yScale());
                shape |= r;
            }
            shape = shape & region;

        //Only translated, not scaled
        } else if (translated) {
            shape = shape.translated(data.xTranslation(), data.yTranslation());
            shape = shape & region;
        }

        if (!shape.isEmpty()) {
            doContrast(w, shape, screen, data.opacity(), data.screenProjectionMatrix());
        }
    }

    // Draw the window over the contrast area
    effects->drawWindow(w, mask, region, data);
}

void ContrastEffect::paintEffectFrame(EffectFrame *frame, QRegion region, double opacity, double frameOpacity)
{
    //FIXME: this is a no-op for now, it should figure out the right contrast, intensity, saturation
    effects->paintEffectFrame(frame, region, opacity, frameOpacity);
}

void ContrastEffect::doContrast(EffectWindow *w, const QRegion& shape, const QRect& screen, const float opacity, const QMatrix4x4 &screenProjection)
{
    const QRegion actualShape = shape & screen;
    const QRect r = actualShape.boundingRect();

    qreal scale = GLRenderTarget::virtualScreenScale();

    // Upload geometry for the horizontal and vertical passes
    GLVertexBuffer *vbo = GLVertexBuffer::streamingBuffer();
    uploadGeometry(vbo, actualShape);
    vbo->bindArrays();

    // Create a scratch texture and copy the area in the back buffer that we're
    // going to blur into it
    GLTexture scratch(GL_RGBA8, r.width() * scale, r.height() * scale);
    scratch.setFilter(GL_LINEAR);
    scratch.setWrapMode(GL_CLAMP_TO_EDGE);

    GLRenderTarget scratchTarget(scratch);
    scratchTarget.blitFromFramebuffer(r);
    scratch.bind();

    // Draw the texture on the offscreen framebuffer object, while blurring it horizontally

    shader->setColorMatrix(m_colorMatrices.value(w));
    shader->bind();


    shader->setOpacity(opacity);
    // Set up the texture matrix to transform from screen coordinates
    // to texture coordinates.
    QMatrix4x4 textureMatrix;
    textureMatrix.scale(1.0 / r.width(), -1.0 / r.height(), 1);
    textureMatrix.translate(-r.x(), -r.height() - r.y(), 0);
    shader->setTextureMatrix(textureMatrix);
    shader->setModelViewProjectionMatrix(screenProjection);

    vbo->draw(GL_TRIANGLES, 0, actualShape.rectCount() * 6);

    scratch.unbind();
    scratch.discard();

    vbo->unbindArrays();

    if (opacity < 1.0) {
        glDisable(GL_BLEND);
    }

    shader->unbind();
}

} // namespace KWin

