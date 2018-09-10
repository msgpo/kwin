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

#include "effectshandler_wrapper.h"
#include "effectwindow_wrapper.h"

#include <QQmlEngine>

namespace KWin
{

EffectsHandlerWrapper::EffectsHandlerWrapper(QJSEngine *engine, EffectsHandler *wrapped)
    : m_engine(engine)
    , m_wrapped(wrapped)
{
    connect(m_wrapped, QOverload<int, int, EffectWindow *>::of(&EffectsHandler::desktopChanged), this,
        [this] (int oldDesktop, int newDesktop, EffectWindow *with) {
            QJSValue withWrapped = m_engine->newQObject(findWrappedWindow(with));
            emit desktopChanged(oldDesktop, newDesktop, withWrapped);
        }
    );

    connect(m_wrapped, &EffectsHandler::desktopPresenceChanged, this,
        [this] (EffectWindow *w, int oldDesktop, int newDesktop) {
            QJSValue wrapped = m_engine->newQObject(findWrappedWindow(w));
            emit desktopPresenceChanged(wrapped, oldDesktop, newDesktop);
        }
    );

    connect(m_wrapped, &EffectsHandler::numberDesktopsChanged,
            this, &EffectsHandlerWrapper::numberDesktopsChanged);

    connect(m_wrapped, &EffectsHandler::numberScreensChanged,
            this, &EffectsHandlerWrapper::numberScreensChanged);

    connect(m_wrapped, &EffectsHandler::showingDesktopChanged,
            this, &EffectsHandlerWrapper::showingDesktopChanged);

    connect(m_wrapped, &EffectsHandler::windowAdded, this,
        [this] (EffectWindow *w) {
            emit windowAdded(m_engine->newQObject(findWrappedWindow(w)));
        }
    );

    connect(m_wrapped, &EffectsHandler::windowClosed, this,
        [this] (EffectWindow *w) {
            emit windowClosed(m_engine->newQObject(findWrappedWindow(w)));
        }
    );

    connect(m_wrapped, &EffectsHandler::windowActivated, this,
        [this] (EffectWindow *w) {
            emit windowActivated(m_engine->newQObject(findWrappedWindow(w)));
        }
    );

    connect(m_wrapped, &EffectsHandler::windowDeleted, this,
        [this] (EffectWindow *w) {
            auto it = m_wrappedWindows.find(w);
            if (it == m_wrappedWindows.end()) {
                // Can it be possible?
                return;
            }
            emit windowDeleted(m_engine->newQObject(*it));
            delete *it;
            m_wrappedWindows.erase(it);
        }
    );

    connect(m_wrapped, &EffectsHandler::windowStartUserMovedResized, this,
        [this] (EffectWindow *w) {
            emit windowStartUserMovedResized(m_engine->newQObject(findWrappedWindow(w)));
        }
    );

    connect(m_wrapped, &EffectsHandler::windowStepUserMovedResized, this,
        [this] (EffectWindow *w, const QRect &geometry) {
            QJSValue wrapped = m_engine->newQObject(findWrappedWindow(w));
            emit windowStepUserMovedResized(wrapped, geometry);
        }
    );

    connect(m_wrapped, &EffectsHandler::windowFinishUserMovedResized, this,
        [this] (EffectWindow *w) {
            emit windowFinishUserMovedResized(m_engine->newQObject(findWrappedWindow(w)));
        }
    );

    connect(m_wrapped, &EffectsHandler::windowMaximizedStateChanged, this,
        [this] (EffectWindow *w, bool horizontal, bool vertical) {
            QJSValue wrapped = m_engine->newQObject(findWrappedWindow(w));
            emit windowMaximizedStateChanged(wrapped, horizontal, vertical);
        }
    );

    connect(m_wrapped, &EffectsHandler::windowGeometryShapeChanged, this,
        [this] (EffectWindow *w, const QRect &old) {
            QJSValue wrapped = m_engine->newQObject(findWrappedWindow(w));
            emit windowGeometryShapeChanged(wrapped, old);
        }
    );

    connect(m_wrapped, &EffectsHandler::windowPaddingChanged, this,
        [this] (EffectWindow *w, const QRect &old) {
            QJSValue wrapped = m_engine->newQObject(findWrappedWindow(w));
            emit windowPaddingChanged(wrapped, old);
        }
    );

    connect(m_wrapped, &EffectsHandler::windowOpacityChanged, this,
        [this] (EffectWindow *w, qreal oldOpacity, qreal newOpacity) {
            QJSValue wrapped = m_engine->newQObject(findWrappedWindow(w));
            emit windowOpacityChanged(wrapped, oldOpacity, newOpacity);
        }
    );

    connect(m_wrapped, &EffectsHandler::windowMinimized, this,
        [this] (EffectWindow *w) {
            emit windowMinimized(m_engine->newQObject(findWrappedWindow(w)));
        }
    );

    connect(m_wrapped, &EffectsHandler::windowUnminimized, this,
        [this] (EffectWindow *w) {
            emit windowUnminimized(m_engine->newQObject(findWrappedWindow(w)));
        }
    );

    connect(m_wrapped, &EffectsHandler::windowModalityChanged, this,
        [this] (EffectWindow *w) {
            emit windowModalityChanged(m_engine->newQObject(findWrappedWindow(w)));
        }
    );

    connect(m_wrapped, &EffectsHandler::windowUnresponsiveChanged, this,
        [this] (EffectWindow *w, bool unresponsive) {
            QJSValue wrapped = m_engine->newQObject(findWrappedWindow(w));
            emit windowUnresponsiveChanged(wrapped, unresponsive);
        }
    );

    connect(m_wrapped, &EffectsHandler::windowDamaged, this,
        [this] (EffectWindow *w, const QRect &r) {
            QJSValue wrapped = m_engine->newQObject(findWrappedWindow(w));
            emit windowDamaged(wrapped, r);
        }
    );

    connect(m_wrapped, &EffectsHandler::tabBoxAdded,
            this, &EffectsHandlerWrapper::tabBoxAdded);

    connect(m_wrapped, &EffectsHandler::tabBoxClosed,
            this, &EffectsHandlerWrapper::tabBoxClosed);

    connect(m_wrapped, &EffectsHandler::tabBoxUpdated,
            this, &EffectsHandlerWrapper::tabBoxUpdated);

    // connect(m_wrapped, &EffectsHandler::tabBoxKeyEvent,
    //         this, &EffectsHandlerWrapper::tabBoxKeyEvent);

    connect(m_wrapped, &EffectsHandler::currentTabAboutToChange, this,
        [this] (EffectWindow *from, EffectWindow *to) {
            QJSValue fromWrapped = m_engine->newQObject(findWrappedWindow(from));
            QJSValue toWrapped = m_engine->newQObject(findWrappedWindow(to));
            emit currentTabAboutToChange(fromWrapped, toWrapped);
        }
    );

    connect(m_wrapped, &EffectsHandler::tabAdded, this,
        [this] (EffectWindow *from, EffectWindow *to) {
            QJSValue fromWrapped = m_engine->newQObject(findWrappedWindow(from));
            QJSValue toWrapped = m_engine->newQObject(findWrappedWindow(to));
            emit tabAdded(fromWrapped, toWrapped);
        }
    );

    connect(m_wrapped, &EffectsHandler::tabRemoved, this,
        [this] (EffectWindow *c, EffectWindow *group) {
            QJSValue cWrapped = m_engine->newQObject(findWrappedWindow(c));
            QJSValue groupWrapped = m_engine->newQObject(findWrappedWindow(group));
            emit tabRemoved(cWrapped, groupWrapped);
        }
    );

    connect(m_wrapped, &EffectsHandler::mouseChanged,
            this, &EffectsHandlerWrapper::mouseChanged);

    connect(m_wrapped, &EffectsHandler::cursorShapeChanged,
            this, &EffectsHandlerWrapper::cursorShapeChanged);

    // Do scripted effects really need this one?
    connect(m_wrapped, &EffectsHandler::propertyNotify, this,
        [this] (EffectWindow *w, long atom) {
            QJSValue wrapped = m_engine->newQObject(findWrappedWindow(w));
            emit propertyNotify(wrapped, atom);
        }
    );

    connect(m_wrapped, &EffectsHandler::screenGeometryChanged,
            this, &EffectsHandlerWrapper::screenGeometryChanged);

    connect(m_wrapped, &EffectsHandler::currentActivityChanged,
            this, &EffectsHandlerWrapper::currentActivityChanged);

    connect(m_wrapped, &EffectsHandler::activityAdded,
            this, &EffectsHandlerWrapper::activityAdded);

    connect(m_wrapped, &EffectsHandler::activityRemoved,
            this, &EffectsHandlerWrapper::activityRemoved);

    connect(m_wrapped, &EffectsHandler::screenLockingChanged,
            this, &EffectsHandlerWrapper::screenLockingChanged);

    connect(m_wrapped, &EffectsHandler::stackingOrderChanged,
            this, &EffectsHandlerWrapper::stackingOrderChanged);

    connect(m_wrapped, &EffectsHandler::screenEdgeApproaching,
            this, &EffectsHandlerWrapper::screenEdgeApproaching);

    connect(m_wrapped, &EffectsHandler::virtualScreenSizeChanged,
            this, &EffectsHandlerWrapper::virtualScreenSizeChanged);

    connect(m_wrapped, &EffectsHandler::virtualScreenGeometryChanged,
            this, &EffectsHandlerWrapper::virtualScreenGeometryChanged);

    connect(m_wrapped, &EffectsHandler::windowShown, this,
        [this] (EffectWindow *w) {
            emit windowShown(m_engine->newQObject(findWrappedWindow(w)));
        }
    );

    connect(m_wrapped, &EffectsHandler::windowHidden, this,
        [this] (EffectWindow *w) {
            emit windowHidden(m_engine->newQObject(findWrappedWindow(w)));
        }
    );

    connect(m_wrapped, &EffectsHandler::windowDataChanged, this,
        [this] (EffectWindow *w, int role) {
            emit windowDataChanged(m_engine->newQObject(findWrappedWindow(w)), role);
        }
    );

    // Do scripted effects really need this one?
    connect(m_wrapped, &EffectsHandler::xcbConnectionChanged,
            this, &EffectsHandlerWrapper::xcbConnectionChanged);

    // Do scripted effects really need this one?
    connect(m_wrapped, &EffectsHandler::activeFullScreenEffectChanged,
            this, &EffectsHandlerWrapper::activeFullScreenEffectChanged);
}

EffectsHandlerWrapper::~EffectsHandlerWrapper()
{
    qDeleteAll(m_wrappedWindows);
}

static inline EffectWindowWrapper *toEffectWindowWrapper(QJSValue value)
{
    if (!value.isQObject()) {
        return nullptr;
    }
    return qobject_cast<EffectWindowWrapper *>(value.toQObject());
}

void EffectsHandlerWrapper::moveWindow(QJSValue w, const QPoint &pos, bool snap, double snapAdjust)
{
    EffectWindowWrapper *wrappedWindow = toEffectWindowWrapper(w);
    if (wrappedWindow == nullptr) {
        // TODO: Handle errors.
        return;
    }
    m_wrapped->moveWindow(wrappedWindow->window(), pos, snap, snapAdjust);
}

void EffectsHandlerWrapper::windowToDesktop(QJSValue w, int desktop)
{
    EffectWindowWrapper *wrappedWindow = toEffectWindowWrapper(w);
    if (wrappedWindow == nullptr) {
        // TODO: Handle errors.
        return;
    }
    m_wrapped->windowToDesktop(wrappedWindow->window(), desktop);
}

void EffectsHandlerWrapper::windowToScreen(QJSValue w, int screen)
{
    EffectWindowWrapper *wrappedWindow = toEffectWindowWrapper(w);
    if (wrappedWindow == nullptr) {
        // TODO: Handle errors.
        return;
    }
    m_wrapped->windowToScreen(wrappedWindow->window(), screen);
}

int EffectsHandlerWrapper::desktopAbove(int desktop, bool wrap) const
{
    return m_wrapped->desktopAbove(desktop, wrap);
}

int EffectsHandlerWrapper::desktopToRight(int desktop, bool wrap) const
{
    return m_wrapped->desktopToRight(desktop, wrap);
}

int EffectsHandlerWrapper::desktopBelow(int desktop, bool wrap) const
{
    return m_wrapped->desktopBelow(desktop, wrap);
}

int EffectsHandlerWrapper::desktopToLeft(int desktop, bool wrap) const
{
    return m_wrapped->desktopToLeft(desktop, wrap);
}

QString EffectsHandlerWrapper::desktopName(int desktop) const
{
    return m_wrapped->desktopName(desktop);
}

int EffectsHandlerWrapper::screenNumber(const QPoint &pos) const
{
    return m_wrapped->screenNumber(pos);
}

QJSValue EffectsHandlerWrapper::findWindow(WId id) const
{
    EffectWindow *w = m_wrapped->findWindow(id);
    return m_engine->newQObject(findWrappedWindow(w));
}

QJSValue EffectsHandlerWrapper::findWindow(KWayland::Server::SurfaceInterface *surf) const
{
    EffectWindow *w = m_wrapped->findWindow(surf);
    return m_engine->newQObject(findWrappedWindow(w));
}

void EffectsHandlerWrapper::setElevatedWindow(QJSValue w, bool set)
{
    EffectWindowWrapper *wrappedWindow = toEffectWindowWrapper(w);
    if (wrappedWindow == nullptr) {
        // TODO: Handle errors.
        return;
    }
    m_wrapped->setElevatedWindow(wrappedWindow->window(), set);
}

void EffectsHandlerWrapper::addRepaintFull()
{
    m_wrapped->addRepaintFull();
}

void EffectsHandlerWrapper::addRepaint(const QRect &r)
{
    m_wrapped->addRepaint(r);
}

void EffectsHandlerWrapper::addRepaint(const QRegion &r)
{
    m_wrapped->addRepaint(r);
}

void EffectsHandlerWrapper::addRepaint(int x, int y, int w, int h)
{
    m_wrapped->addRepaint(x, y, w, h);
}

int EffectsHandlerWrapper::currentDesktop() const
{
    return m_wrapped->currentDesktop();
}

void EffectsHandlerWrapper::setCurrentDesktop(int desktop)
{
    m_wrapped->setCurrentDesktop(desktop);
}

QString EffectsHandlerWrapper::currentActivity() const
{
    return m_wrapped->currentActivity();
}

QJSValue EffectsHandlerWrapper::activeWindow() const
{
    return m_engine->newQObject(findWrappedWindow(m_wrapped->activeWindow()));
}

void EffectsHandlerWrapper::activateWindow(QJSValue w)
{
    EffectWindowWrapper *wrappedWindow = toEffectWindowWrapper(w);
    if (wrappedWindow == nullptr) {
        // TODO: Handle errors.
        return;
    }
    m_wrapped->activateWindow(wrappedWindow->window());
}

QSize EffectsHandlerWrapper::desktopGridSize() const
{
    return m_wrapped->desktopGridSize();
}

int EffectsHandlerWrapper::desktopGridWidth() const
{
    return m_wrapped->desktopGridWidth();
}

int EffectsHandlerWrapper::desktopGridHeight() const
{
    return m_wrapped->desktopGridHeight();
}

int EffectsHandlerWrapper::workspaceWidth() const
{
    return m_wrapped->workspaceWidth();
}

int EffectsHandlerWrapper::workspaceHeight() const
{
    return m_wrapped->workspaceHeight();
}

int EffectsHandlerWrapper::numberOfDesktops() const
{
    return m_wrapped->numberOfDesktops();
}

void EffectsHandlerWrapper::setNumberOfDesktops(int desktops)
{
    m_wrapped->setNumberOfDesktops(desktops);
}

bool EffectsHandlerWrapper::optionRollOverDesktops() const
{
    return m_wrapped->optionRollOverDesktops();
}

int EffectsHandlerWrapper::activeScreen() const
{
    return m_wrapped->activeScreen();
}

int EffectsHandlerWrapper::numScreens() const
{
    return m_wrapped->numScreens();
}

qreal EffectsHandlerWrapper::animationTimeFactor() const
{
    return m_wrapped->animationTimeFactor();
}

QJSValue EffectsHandlerWrapper::stackingOrder() const
{
    const EffectWindowList windows = m_wrapped->stackingOrder();

    QJSValue wrappedStackingOrder = m_engine->newArray(windows.count());
    for (int i = 0; i < windows.count(); i++) {
        QJSValue wrappedWindow = m_engine->newQObject(findWrappedWindow(windows.at(i)));
        wrappedStackingOrder.setProperty(i, wrappedWindow);
    }

    return wrappedStackingOrder;
}

bool EffectsHandlerWrapper::decorationsHaveAlpha() const
{
    return m_wrapped->decorationsHaveAlpha();
}

bool EffectsHandlerWrapper::decorationSupportsBlurBehind() const
{
    return m_wrapped->decorationSupportsBlurBehind();
}

CompositingType EffectsHandlerWrapper::compositingType() const
{
    return m_wrapped->compositingType();
}

QPoint EffectsHandlerWrapper::cursorPos() const
{
    return m_wrapped->cursorPos();
}

QSize EffectsHandlerWrapper::virtualScreenSize() const
{
    return m_wrapped->virtualScreenSize();
}

QRect EffectsHandlerWrapper::virtualScreenGeometry() const
{
    return m_wrapped->virtualScreenGeometry();
}

EffectWindowWrapper *EffectsHandlerWrapper::findWrappedWindow(EffectWindow *w) const
{
    if (w == nullptr) {
        return nullptr;
    }
    EffectWindowWrapper *&wrapped = m_wrappedWindows[w];
    if (wrapped != nullptr) {
        return wrapped;
    }
    wrapped = new EffectWindowWrapper(m_engine, const_cast<EffectsHandlerWrapper *>(this), w);
    QQmlEngine::setObjectOwnership(wrapped, QQmlEngine::CppOwnership);
    return wrapped;
}

} // namespace KWin
