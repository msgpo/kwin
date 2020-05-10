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

#include "qtquickintegration.h"

#include "workspace.h"

#include <QSGSimpleTextureNode>

namespace KWin
{

KQuickWindowDropShadowRenderJob::KQuickWindowDropShadowRenderJob(KQuickWindowDropShadowRenderer *renderer)
    : m_dropShadowRenderer(renderer)
{
}

void KQuickWindowDropShadowRenderJob::run()
{
    // TODO: Actually render the drop shadow.
}

KQuickWindowDecorationRenderJob::KQuickWindowDecorationRenderJob(KQuickWindowDecorationRenderer *renderer)
    : m_decorationRenderer(renderer)
{
}

void KQuickWindowDecorationRenderJob::run()
{
    // TODO: Actually render the decoration.
}

KQuickWindowTextureReferenceUpdateJob::KQuickWindowTextureReferenceUpdateJob(KQuickWindowTextureReference *reference)
    : m_textureReference(reference)
{
}

void KQuickWindowTextureReferenceUpdateJob::run()
{
    // TODO: Actually update the referenced texture.
}

KQuickWindowDropShadowRenderer::KQuickWindowDropShadowRenderer(QObject *parent)
    : QObject(parent)
{
}

KQuickWindowDropShadowRenderer::~KQuickWindowDropShadowRenderer()
{
}

QSGTexture *KQuickWindowDropShadowRenderer::texture() const
{
    return m_sceneGraphTexture;
}

KQuickWindowDropShadowItem::KQuickWindowDropShadowItem(QQuickItem *parent)
    : QQuickItem(parent)
    , m_dropShadowRenderer(new KQuickWindowDropShadowRenderer(this))
{
    setFlag(ItemHasContents);
}

KQuickWindowDropShadowItem::~KQuickWindowDropShadowItem()
{
}

QSGNode *KQuickWindowDropShadowItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    QSGSimpleTextureNode *textureNode = static_cast<QSGSimpleTextureNode *>(oldNode);
    if (!textureNode)
        textureNode = new QSGSimpleTextureNode();

    textureNode->setTexture(m_dropShadowRenderer->texture());
    textureNode->setRect(boundingRect());

    return textureNode;
}

KQuickWindowDecorationRenderer::KQuickWindowDecorationRenderer(QObject *parent)
    : QObject(parent)
{
}

KQuickWindowDecorationRenderer::~KQuickWindowDecorationRenderer()
{
}

QSGTexture *KQuickWindowDecorationRenderer::texture() const
{
    return m_sceneGraphTexture;
}

KQuickWindowDecorationItem::KQuickWindowDecorationItem(QQuickItem *parent)
    : QQuickItem(parent)
    , m_decorationRenderer(new KQuickWindowDecorationRenderer(this))
{
    setFlag(ItemHasContents);
}

KQuickWindowDecorationItem::~KQuickWindowDecorationItem()
{
}

QSGNode *KQuickWindowDecorationItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    QSGSimpleTextureNode *textureNode = static_cast<QSGSimpleTextureNode *>(oldNode);
    if (!textureNode)
        textureNode = new QSGSimpleTextureNode();

    textureNode->setTexture(m_decorationRenderer->texture());
    textureNode->setRect(boundingRect());

    return textureNode;
}

KQuickWindowTextureReference::KQuickWindowTextureReference()
{
}

KQuickWindowTextureReference::~KQuickWindowTextureReference()
{
}

QSGTexture *KQuickWindowTextureReference::texture() const
{
    return m_texture;
}

KQuickWindowSurfaceItem::KQuickWindowSurfaceItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents);
}

KQuickWindowSurfaceItem::~KQuickWindowSurfaceItem()
{
}

QSGNode *KQuickWindowSurfaceItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    QSGSimpleTextureNode *textureNode = static_cast<QSGSimpleTextureNode *>(oldNode);
    if (!textureNode)
        textureNode = new QSGSimpleTextureNode();

    textureNode->setTexture(m_textureReference->texture());
    textureNode->setRect(boundingRect());

    return textureNode;
}

KQuickWindowThumbnailItem::KQuickWindowThumbnailItem(QQuickItem *parent)
    : QQuickItem(parent)
{
}

KQuickWindowThumbnailItem::~KQuickWindowThumbnailItem()
{
}

void KQuickWindowThumbnailItem::setHandle(const QUuid &handle)
{
    setClient(workspace()->findClient(handle));
}

QUuid KQuickWindowThumbnailItem::handle() const
{
    return client() ? client()->internalId() : QUuid();
}

void KQuickWindowThumbnailItem::setClient(AbstractClient *client)
{
    if (m_client == client)
        return;
    m_client = client;
    emit clientChanged();
}

AbstractClient *KQuickWindowThumbnailItem::client() const
{
    return m_client;
}

} // namespace KWin
