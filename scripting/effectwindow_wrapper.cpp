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

bool EffectWindowWrapper::hasAlpha() const
{
    return m_wrapped->hasAlpha();
}

qreal EffectWindowWrapper::opacity() const
{
    return m_wrapped->opacity();
}

QPoint EffectWindowWrapper::pos() const
{
    return m_wrapped->pos();
}

int EffectWindowWrapper::x() const
{
    return m_wrapped->x();
}

int EffectWindowWrapper::y() const
{
    return m_wrapped->y();
}

QSize EffectWindowWrapper::size() const
{
    return m_wrapped->size();
}
int EffectWindowWrapper::width() const
{
    return m_wrapped->width();
}
int EffectWindowWrapper::height() const
{
    return m_wrapped->height();
}

QRect EffectWindowWrapper::geometry() const
{
    return m_wrapped->geometry();
}

QRect EffectWindowWrapper::expandedGeometry() const
{
    return m_wrapped->expandedGeometry();
}

QRect EffectWindowWrapper::rect() const
{
    return m_wrapped->rect();
}

int EffectWindowWrapper::screen() const
{
    return m_wrapped->screen();
}

int EffectWindowWrapper::desktop() const
{
    return m_wrapped->desktop();
}

bool EffectWindowWrapper::isOnAllDesktops() const
{
    return m_wrapped->isOnAllDesktops();
}

bool EffectWindowWrapper::isOnCurrentDesktop() const
{
    return m_wrapped->isOnCurrentDesktop();
}

QString EffectWindowWrapper::windowClass() const
{
    return m_wrapped->windowClass();
}

QString EffectWindowWrapper::windowRole() const
{
    return m_wrapped->windowRole();
}

bool EffectWindowWrapper::isDesktop() const
{
    return m_wrapped->isDesktop();
}

bool EffectWindowWrapper::isDock() const
{
    return m_wrapped->isDock();
}

bool EffectWindowWrapper::isToolbar() const
{
    return m_wrapped->isToolbar();
}

bool EffectWindowWrapper::isMenu() const
{
    return m_wrapped->isMenu();
}

bool EffectWindowWrapper::isNormalWindow() const
{
    return m_wrapped->isNormalWindow();
}

bool EffectWindowWrapper::isDialog() const
{
    return m_wrapped->isDialog();
}

bool EffectWindowWrapper::isSplash() const
{
    return m_wrapped->isSplash();
}

bool EffectWindowWrapper::isUtility() const
{
    return m_wrapped->isUtility();
}

bool EffectWindowWrapper::isDropdownMenu() const
{
    return m_wrapped->isDropdownMenu();
}

bool EffectWindowWrapper::isPopupMenu() const
{
    return m_wrapped->isPopupMenu();
}

bool EffectWindowWrapper::isTooltip() const
{
    return m_wrapped->isTooltip();
}

bool EffectWindowWrapper::isNotification() const
{
    return m_wrapped->isNotification();
}

bool EffectWindowWrapper::isOnScreenDisplay() const
{
    return m_wrapped->isOnScreenDisplay();
}

bool EffectWindowWrapper::isComboBox() const
{
    return m_wrapped->isComboBox();
}

bool EffectWindowWrapper::isDNDIcon() const
{
    return m_wrapped->isDNDIcon();
}

int EffectWindowWrapper::windowType() const
{
    return m_wrapped->windowType();
}

bool EffectWindowWrapper::isManaged() const
{
    return m_wrapped->isManaged();
}

bool EffectWindowWrapper::isDeleted() const
{
    return m_wrapped->isDeleted();
}

bool EffectWindowWrapper::hasOwnShape() const
{
    return m_wrapped->hasOwnShape();
}

QRegion EffectWindowWrapper::shape() const
{
    return m_wrapped->shape();
}

QString EffectWindowWrapper::caption() const
{
    return m_wrapped->caption();
}

bool EffectWindowWrapper::keepAbove() const
{
    return m_wrapped->keepAbove();
}

bool EffectWindowWrapper::keepBelow() const
{
    return m_wrapped->keepBelow();
}

bool EffectWindowWrapper::isMinimized() const
{
    return m_wrapped->isMinimized();
}

void EffectWindowWrapper::setMinimized(bool minimized)
{
    return m_wrapped->setMinimized(minimized);
}

bool EffectWindowWrapper::isModal() const
{
    return m_wrapped->isModal();
}

bool EffectWindowWrapper::isMovable() const
{
    return m_wrapped->isMovable();
}

bool EffectWindowWrapper::isMovableAcrossScreens() const
{
    return m_wrapped->isMovableAcrossScreens();
}

bool EffectWindowWrapper::isSpecialWindow() const
{
    return m_wrapped->isSpecialWindow();
}

QSize EffectWindowWrapper::basicUnit() const
{
    return m_wrapped->basicUnit();
}

bool EffectWindowWrapper::isUserMove() const
{
    return m_wrapped->isUserMove();
}

bool EffectWindowWrapper::isUserResize() const
{
    return m_wrapped->isUserResize();
}

QIcon EffectWindowWrapper::icon() const
{
    return m_wrapped->icon();
}

QRect EffectWindowWrapper::iconGeometry() const
{
    return m_wrapped->iconGeometry();
}

bool EffectWindowWrapper::isSkipSwitcher() const
{
    return m_wrapped->isSkipSwitcher();
}

bool EffectWindowWrapper::skipsCloseAnimation() const
{
    return m_wrapped->skipsCloseAnimation();
}

QRect EffectWindowWrapper::contentsRect() const
{
    return m_wrapped->contentsRect();
}

QRect EffectWindowWrapper::decorationInnerRect() const
{
    return m_wrapped->decorationInnerRect();
}

bool EffectWindowWrapper::hasDecoration() const
{
    return m_wrapped->hasDecoration();
}

QStringList EffectWindowWrapper::activities() const
{
    return m_wrapped->activities();
}

bool EffectWindowWrapper::isOnCurrentActivity() const
{
    return m_wrapped->isOnCurrentActivity();
}

bool EffectWindowWrapper::isOnAllActivities() const
{
    return m_wrapped->isOnAllActivities();
}

bool EffectWindowWrapper::decorationHasAlpha() const
{
    return m_wrapped->decorationHasAlpha();
}

bool EffectWindowWrapper::isVisible() const
{
    return m_wrapped->isVisible();
}

bool EffectWindowWrapper::isFullScreen() const
{
    return m_wrapped->isFullScreen();
}

bool EffectWindowWrapper::isUnresponsive() const
{
    return m_wrapped->isUnresponsive();
}

KWayland::Server::SurfaceInterface *EffectWindowWrapper::surface() const
{
    return m_wrapped->surface();
}

} // namespace KWin
