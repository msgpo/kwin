/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006 Lubos Lunak <l.lunak@kde.org>
Copyright (C) 2007 Christian Nitschkowski <christian.nitschkowski@kdemail.net>
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

// own
#include "mousemark.h"
#include "mousemark_canvas.h"

// KConfigSkeleton
#include "mousemarkconfig.h"

// KF
#include <KGlobalAccel>
#include <KLocalizedString>

// Qt
#include <QAction>
#include <QStandardPaths>

namespace KWin
{

static const QString mainQmlFile = "kwin/effects/mousemark/qml/main.qml";

MouseMarkEffect::MouseMarkEffect()
{
    initConfig<MouseMarkConfig>();
    reconfigure(ReconfigureAll);

    auto *clearAllAction = new QAction(this);
    clearAllAction->setObjectName(QStringLiteral("ClearMouseMarks"));
    clearAllAction->setText(i18n("Clear All Mouse Marks"));
    KGlobalAccel::self()->setDefaultShortcut(clearAllAction, {Qt::SHIFT + Qt::META + Qt::Key_F11});
    KGlobalAccel::self()->setShortcut(clearAllAction, {Qt::SHIFT + Qt::META + Qt::Key_F11});
    effects->registerGlobalShortcut(Qt::SHIFT + Qt::META + Qt::Key_F11, clearAllAction);
    connect(clearAllAction, SIGNAL(triggered(bool)), this, SLOT(clearAll()));

    connect(effects, &EffectsHandler::mouseChanged,
            this, &MouseMarkEffect::slotMouseChanged);
    connect(effects, &EffectsHandler::screenLockingChanged,
            this, &MouseMarkEffect::slotScreenLockingChanged);

    effects->startMousePolling();

    createOverlayWindow();
}

MouseMarkEffect::~MouseMarkEffect()
{
    effects->stopMousePolling();

    if (m_overlayView) {
        m_overlayView->deleteLater();
    }
}

void MouseMarkEffect::reconfigure(ReconfigureFlags flags)
{
    Q_UNUSED(flags)

    MouseMarkConfig::self()->read();
    m_lineWidth = MouseMarkConfig::lineWidth();
    m_color = MouseMarkConfig::color();
}

bool MouseMarkEffect::isActive() const
{
    return !effects->isScreenLocked() && false;
}

void MouseMarkEffect::clearAll()
{
    if (m_canvas != nullptr) {
        m_canvas->clear();
        m_currentPath.clear();
    }
}

void MouseMarkEffect::slotMouseChanged(const QPoint& pos, const QPoint &oldPos,
                                       Qt::MouseButtons buttons, Qt::MouseButtons oldButtons,
                                       Qt::KeyboardModifiers modifiers, Qt::KeyboardModifiers oldModifiers)
{
    Q_UNUSED(oldPos)
    Q_UNUSED(buttons)
    Q_UNUSED(oldButtons)
    Q_UNUSED(oldModifiers)

    if (m_canvas == nullptr) {
        return;
    }

    if (modifiers == (Qt::META | Qt::SHIFT)) {
        m_currentPath.append(QPointF(pos));
        m_canvas->setCurrentPath(m_currentPath, QPen(m_color, m_lineWidth));
    } else if (!m_currentPath.isEmpty()) {
        m_canvas->addPath(m_currentPath, QPen(m_color, m_lineWidth));
        m_canvas->clearCurrentPath();
        m_currentPath.clear();
    }
}

void MouseMarkEffect::slotScreenLockingChanged(bool locked)
{
    if (locked) {
        effects->stopMousePolling();
    } else {
        effects->startMousePolling();
    }
}

void MouseMarkEffect::createOverlayWindow()
{
    qmlRegisterType<MouseMarkCanvas>("org.kde.kwin.mousemark", 1, 0, "MouseMarkCanvas");

    m_overlayView = new QQuickView();
    m_overlayView->setColor(Qt::transparent);
    m_overlayView->setFlags(
        Qt::FramelessWindowHint |
        Qt::WindowTransparentForInput |
        Qt::X11BypassWindowManagerHint);
    m_overlayView->setGeometry(effects->virtualScreenGeometry());
    m_overlayView->setResizeMode(QQuickView::SizeRootObjectToView);
    m_overlayView->setSource(QUrl(QStandardPaths::locate(QStandardPaths::GenericDataLocation, mainQmlFile)));

    if (auto *canvas = m_overlayView->rootObject()->findChild<MouseMarkCanvas *>(QStringLiteral("canvas"))) {
        m_canvas = canvas;
    }

    m_overlayView->show();
}

} // namespace KWin
