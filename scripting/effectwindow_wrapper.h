/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006 Lubos Lunak <l.lunak@kde.org>
Copyright (C) 2009 Lucas Murray <lmurray@undefinedfire.com>
Copyright (C) 2010, 2011 Martin Gräßlin <mgraesslin@kde.org>
Copyright (C) 2018 Vlad Zagorodniy <vladzzag@gmail.com>

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

#include <kwineffects.h>

#include <QJSEngine>
#include <QJSValue>

namespace KWin
{

class EffectsHandlerWrapper;

class EffectWindowWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool alpha READ hasAlpha CONSTANT)
    Q_PROPERTY(QRect geometry READ geometry)
    Q_PROPERTY(QRect expandedGeometry READ expandedGeometry)
    Q_PROPERTY(int height READ height)
    Q_PROPERTY(qreal opacity READ opacity)
    Q_PROPERTY(QPoint pos READ pos)
    Q_PROPERTY(int screen READ screen)
    Q_PROPERTY(QSize size READ size)
    Q_PROPERTY(int width READ width)
    Q_PROPERTY(int x READ x)
    Q_PROPERTY(int y READ y)
    Q_PROPERTY(int desktop READ desktop)
    Q_PROPERTY(bool onAllDesktops READ isOnAllDesktops)
    Q_PROPERTY(bool onCurrentDesktop READ isOnCurrentDesktop)
    Q_PROPERTY(QRect rect READ rect)
    Q_PROPERTY(QString windowClass READ windowClass)
    Q_PROPERTY(QString windowRole READ windowRole)
    Q_PROPERTY(bool desktopWindow READ isDesktop)
    Q_PROPERTY(bool dock READ isDock)
    Q_PROPERTY(bool toolbar READ isToolbar)
    Q_PROPERTY(bool menu READ isMenu)
    Q_PROPERTY(bool normalWindow READ isNormalWindow)
    Q_PROPERTY(bool dialog READ isDialog)
    Q_PROPERTY(bool splash READ isSplash)
    Q_PROPERTY(bool utility READ isUtility)
    Q_PROPERTY(bool dropdownMenu READ isDropdownMenu)
    Q_PROPERTY(bool popupMenu READ isPopupMenu)
    Q_PROPERTY(bool tooltip READ isTooltip)
    Q_PROPERTY(bool notification READ isNotification)
    Q_PROPERTY(bool onScreenDisplay READ isOnScreenDisplay)
    Q_PROPERTY(bool comboBox READ isComboBox)
    Q_PROPERTY(bool dndIcon READ isDNDIcon)
    Q_PROPERTY(int windowType READ windowType)
    Q_PROPERTY(bool managed READ isManaged)
    Q_PROPERTY(bool deleted READ isDeleted)
    Q_PROPERTY(bool shaped READ hasOwnShape)
    Q_PROPERTY(QRegion shape READ shape)
    Q_PROPERTY(QString caption READ caption)
    Q_PROPERTY(bool keepAbove READ keepAbove)
    Q_PROPERTY(bool keepBelow READ keepBelow)
    Q_PROPERTY(bool minimized READ isMinimized WRITE setMinimized)
    Q_PROPERTY(bool modal READ isModal)
    Q_PROPERTY(bool moveable READ isMovable)
    Q_PROPERTY(bool moveableAcrossScreens READ isMovableAcrossScreens)
    Q_PROPERTY(QSize basicUnit READ basicUnit)
    Q_PROPERTY(bool move READ isUserMove)
    Q_PROPERTY(bool resize READ isUserResize)
    Q_PROPERTY(QRect iconGeometry READ iconGeometry)
    Q_PROPERTY(bool specialWindow READ isSpecialWindow)
    Q_PROPERTY(QIcon icon READ icon)
    Q_PROPERTY(bool skipSwitcher READ isSkipSwitcher)
    Q_PROPERTY(QRect contentsRect READ contentsRect)
    Q_PROPERTY(QRect decorationInnerRect READ decorationInnerRect)
    Q_PROPERTY(bool hasDecoration READ hasDecoration)
    Q_PROPERTY(QStringList activities READ activities)
    Q_PROPERTY(bool onCurrentActivity READ isOnCurrentActivity)
    Q_PROPERTY(bool onAllActivities READ isOnAllActivities)
    Q_PROPERTY(bool decorationHasAlpha READ decorationHasAlpha)
    Q_PROPERTY(bool visible READ isVisible)
    Q_PROPERTY(bool skipsCloseAnimation READ skipsCloseAnimation)
    Q_PROPERTY(KWayland::Server::SurfaceInterface *surface READ surface)
    Q_PROPERTY(bool fullScreen READ isFullScreen)
    Q_PROPERTY(bool unresponsive READ isUnresponsive)

public:
    EffectWindowWrapper(QJSEngine *engine, EffectsHandlerWrapper *wrappedEffects, EffectWindow *wrapped);
    ~EffectWindowWrapper() override;

    Q_INVOKABLE void addRepaint(const QRect &r);
    Q_INVOKABLE void addRepaint(int x, int y, int w, int h);
    Q_INVOKABLE void addRepaintFull();
    Q_INVOKABLE void addLayerRepaint(const QRect &r);
    Q_INVOKABLE void addLayerRepaint(int x, int y, int w, int h);

    Q_INVOKABLE bool isOnActivity(const QString &id) const;

    Q_INVOKABLE QJSValue findModal();
    Q_INVOKABLE QJSValue mainWindows() const;

    Q_INVOKABLE void closeWindow() const;

    Q_INVOKABLE void setData(int role, const QJSValue &data);
    Q_INVOKABLE QVariant data(int role) const;

    EffectWindow *window() const;

    bool hasAlpha() const;
    qreal opacity() const;

    QPoint pos() const;
    int x() const;
    int y() const;

    QSize size() const;
    int width() const;
    int height() const;

    QRect geometry() const;
    QRect expandedGeometry() const;
    QRect rect() const;

    int screen() const;

    int desktop() const;
    bool isOnAllDesktops() const;
    bool isOnCurrentDesktop() const;

    QString windowClass() const;
    QString windowRole() const;

    bool isDesktop() const;
    bool isDock() const;
    bool isToolbar() const;
    bool isMenu() const;
    bool isNormalWindow() const;
    bool isDialog() const;
    bool isSplash() const;
    bool isUtility() const;
    bool isDropdownMenu() const;
    bool isPopupMenu() const;
    bool isTooltip() const;
    bool isNotification() const;
    bool isOnScreenDisplay() const;
    bool isComboBox() const;
    bool isDNDIcon() const;
    int windowType() const;

    bool isManaged() const;
    bool isDeleted() const;

    bool hasOwnShape() const;
    QRegion shape() const;

    QString caption() const;

    bool keepAbove() const;
    bool keepBelow() const;

    bool isMinimized() const;
    void setMinimized(bool minimized);

    bool isModal() const;
    bool isMovable() const;
    bool isMovableAcrossScreens() const;
    bool isSpecialWindow() const;

    QSize basicUnit() const;
    bool isUserMove() const;
    bool isUserResize() const;

    QIcon icon() const;
    QRect iconGeometry() const;

    bool isSkipSwitcher() const;
    bool skipsCloseAnimation() const;

    QRect contentsRect() const;
    QRect decorationInnerRect() const;
    bool hasDecoration() const;

    QStringList activities() const;
    bool isOnCurrentActivity() const;
    bool isOnAllActivities() const;

    bool decorationHasAlpha() const;

    bool isVisible() const;
    bool isFullScreen() const;
    bool isUnresponsive() const;

    KWayland::Server::SurfaceInterface *surface() const;

private:
    QJSEngine *m_engine;
    EffectsHandlerWrapper *m_wrappedEffects;
    EffectWindow *m_wrapped;

    Q_DISABLE_COPY(EffectWindowWrapper)
};

} // namespace KWin
