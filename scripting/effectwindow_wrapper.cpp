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

#include "effectwindow_wrapper.h"
#include "effectshandler_wrapper.h"

namespace KWin
{

EffectWindowWrapper::EffectWindowWrapper(QJSEngine *engine, EffectsHandlerWrapper *wrappedEffects, EffectWindow *wrapped)
    : m_engine(engine)
    , m_wrappedEffects(wrappedEffects)
    , m_wrapped(wrapped)
{
}

EffectWindowWrapper::~EffectWindowWrapper()
{
}

void EffectWindowWrapper::addRepaint(const QRect &r)
{
    m_wrapped->addRepaint(r);
}

void EffectWindowWrapper::addRepaint(int x, int y, int w, int h)
{
    m_wrapped->addRepaint(x, y, w, h);
}

void EffectWindowWrapper::addRepaintFull()
{
    m_wrapped->addRepaintFull();
}

void EffectWindowWrapper::addLayerRepaint(const QRect &r)
{
    m_wrapped->addLayerRepaint(r);
}

void EffectWindowWrapper::addLayerRepaint(int x, int y, int w, int h)
{
    m_wrapped->addLayerRepaint(x, y, w, h);
}

bool EffectWindowWrapper::isOnActivity(const QString &id) const
{
    return m_wrapped->isOnActivity(id);
}

QJSValue EffectWindowWrapper::findModal()
{
    return m_engine->newQObject(m_wrappedEffects->findWrappedWindow(m_wrapped->findModal()));
}

QJSValue EffectWindowWrapper::mainWindows() const
{
    const EffectWindowList windows = m_wrapped->mainWindows();

    QJSValue wrappedWindows = m_engine->newArray(windows.count());
    for (int i = 0; i < windows.count(); i++) {
        QJSValue wrappedWindow = m_engine->newQObject(m_wrappedEffects->findWrappedWindow(windows.at(i)));
        wrappedWindows.setProperty(i, wrappedWindow);
    }

    return wrappedWindows;
}

void EffectWindowWrapper::closeWindow() const
{
    m_wrapped->closeWindow();
}

void EffectWindowWrapper::setData(int role, const QJSValue &data)
{
    m_wrapped->setData(role, data.toVariant());
}

QVariant EffectWindowWrapper::data(int role) const
{
    return m_wrapped->data(role);
}

EffectWindow *EffectWindowWrapper::window() const
{
    return m_wrapped;
}

#define WRAP_GETTER(rettype, methodname)            \
    rettype EffectWindowWrapper::methodname() const \
    {                                               \
        return m_wrapped->methodname();             \
    }

WRAP_GETTER(bool, hasAlpha)
WRAP_GETTER(qreal, opacity)
WRAP_GETTER(QPoint, pos)
WRAP_GETTER(int, x)
WRAP_GETTER(int, y)
WRAP_GETTER(QSize, size)
WRAP_GETTER(int, width)
WRAP_GETTER(int, height)
WRAP_GETTER(QRect, geometry)
WRAP_GETTER(QRect, expandedGeometry)
WRAP_GETTER(QRect, rect)
WRAP_GETTER(int, screen)
WRAP_GETTER(int, desktop)
WRAP_GETTER(bool, isOnAllDesktops)
WRAP_GETTER(bool, isOnCurrentDesktop)
WRAP_GETTER(QString, windowClass)
WRAP_GETTER(QString, windowRole)
WRAP_GETTER(bool, isDesktop)
WRAP_GETTER(bool, isDock)
WRAP_GETTER(bool, isToolbar)
WRAP_GETTER(bool, isMenu)
WRAP_GETTER(bool, isNormalWindow)
WRAP_GETTER(bool, isDialog)
WRAP_GETTER(bool, isSplash)
WRAP_GETTER(bool, isUtility)
WRAP_GETTER(bool, isDropdownMenu)
WRAP_GETTER(bool, isPopupMenu)
WRAP_GETTER(bool, isTooltip)
WRAP_GETTER(bool, isNotification)
WRAP_GETTER(bool, isOnScreenDisplay)
WRAP_GETTER(bool, isComboBox)
WRAP_GETTER(bool, isDNDIcon)
WRAP_GETTER(int, windowType)
WRAP_GETTER(bool, isManaged)
WRAP_GETTER(bool, isDeleted)
WRAP_GETTER(bool, hasOwnShape)
WRAP_GETTER(QRegion, shape)
WRAP_GETTER(QString, caption)
WRAP_GETTER(bool, keepAbove)
WRAP_GETTER(bool, keepBelow)
WRAP_GETTER(bool, isMinimized)

void EffectWindowWrapper::setMinimized(bool minimized)
{
    return m_wrapped->setMinimized(minimized);
}

WRAP_GETTER(bool, isModal)
WRAP_GETTER(bool, isMovable)
WRAP_GETTER(bool, isMovableAcrossScreens)
WRAP_GETTER(bool, isSpecialWindow)
WRAP_GETTER(QSize, basicUnit)
WRAP_GETTER(bool, isUserMove)
WRAP_GETTER(bool, isUserResize)
WRAP_GETTER(QIcon, icon)
WRAP_GETTER(QRect, iconGeometry)
WRAP_GETTER(bool, isSkipSwitcher)
WRAP_GETTER(bool, skipsCloseAnimation)
WRAP_GETTER(QRect, contentsRect)
WRAP_GETTER(QRect, decorationInnerRect)
WRAP_GETTER(bool, hasDecoration)
WRAP_GETTER(QStringList, activities)
WRAP_GETTER(bool, isOnCurrentActivity)
WRAP_GETTER(bool, isOnAllActivities)
WRAP_GETTER(bool, decorationHasAlpha)
WRAP_GETTER(bool, isVisible)
WRAP_GETTER(bool, isFullScreen)
WRAP_GETTER(bool, isUnresponsive)
WRAP_GETTER(KWayland::Server::SurfaceInterface *, surface)

} // namespace KWin
