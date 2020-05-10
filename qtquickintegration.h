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

#include "abstract_client.h"

#include <QQuickItem>
#include <QRunnable>
#include <QSGTexture>

namespace KWin
{

class KQuickWindowDropShadowRenderer;
class KQuickWindowDecorationRenderer;
class KQuickWindowTextureReference;

class KQuickWindowDropShadowRenderJob : public QRunnable
{
public:
    explicit KQuickWindowDropShadowRenderJob(KQuickWindowDropShadowRenderer *renderer);

    void run() override;

private:
    KQuickWindowDropShadowRenderer *m_dropShadowRenderer;
};

class KQuickWindowDecorationRenderJob : public QRunnable
{
public:
    explicit KQuickWindowDecorationRenderJob(KQuickWindowDecorationRenderer *renderer);

    void run() override;

private:
    KQuickWindowDecorationRenderer *m_decorationRenderer;
};

class KQuickWindowTextureReferenceUpdateJob : public QRunnable
{
public:
    explicit KQuickWindowTextureReferenceUpdateJob(KQuickWindowTextureReference *reference);

    void run() override;

private:
    KQuickWindowTextureReference *m_textureReference;
};

class KQuickWindowDropShadowRenderer : public QObject
{
    Q_OBJECT

public:
    explicit KQuickWindowDropShadowRenderer(QObject *parent = nullptr);
    ~KQuickWindowDropShadowRenderer() override;

    QSGTexture *texture() const;

private:
    QSGTexture *m_sceneGraphTexture = nullptr;
};

class KQuickWindowDropShadowItem : public QQuickItem
{
    Q_OBJECT

public:
    explicit KQuickWindowDropShadowItem(QQuickItem *parent = nullptr);
    ~KQuickWindowDropShadowItem() override;

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

private:
    KQuickWindowDropShadowRenderer *m_dropShadowRenderer;
};

class KQuickWindowDecorationRenderer : public QObject
{
    Q_OBJECT

public:
    explicit KQuickWindowDecorationRenderer(QObject *parent = nullptr);
    ~KQuickWindowDecorationRenderer() override;

    QSGTexture *texture() const;

private:
    QSGTexture *m_sceneGraphTexture = nullptr;
};

class KQuickWindowDecorationItem : public QQuickItem
{
    Q_OBJECT

public:
    explicit KQuickWindowDecorationItem(QQuickItem *parent = nullptr);
    ~KQuickWindowDecorationItem() override;

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

private:
    KQuickWindowDecorationRenderer *m_decorationRenderer;
};

class KQuickWindowTextureReference
{
public:
    explicit KQuickWindowTextureReference();
    ~KQuickWindowTextureReference();

    QSGTexture *texture() const;

private:
    QSGTexture *m_texture = nullptr;
};

class KQuickWindowSurfaceItem : public QQuickItem
{
    Q_OBJECT

public:
    explicit KQuickWindowSurfaceItem(QQuickItem *parent = nullptr);
    ~KQuickWindowSurfaceItem() override;

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

private:
    KQuickWindowTextureReference *m_textureReference = nullptr;
};

class KQuickWindowThumbnailItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(KWin::AbstractClient *client READ client WRITE setClient NOTIFY clientChanged)
    Q_PROPERTY(QUuid wId READ handle WRITE setHandle NOTIFY handleChanged)

public:
    explicit KQuickWindowThumbnailItem(QQuickItem *parent = nullptr);
    ~KQuickWindowThumbnailItem() override;

    void setHandle(const QUuid &handle);
    QUuid handle() const;

    void setClient(AbstractClient *client);
    AbstractClient *client() const;

Q_SIGNALS:
    void clientChanged();
    void handleChanged();

private:
    AbstractClient *m_client = nullptr;
    KQuickWindowDropShadowItem *m_dropShadowItem = nullptr;
    KQuickWindowDecorationItem *m_decorationItem = nullptr;
    KQuickWindowSurfaceItem *m_surfaceItem = nullptr;
};

} // namespace KWin
