/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2007 Lubos Lunak <l.lunak@kde.org>
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

#ifndef KWIN_MOUSEMARK_H
#define KWIN_MOUSEMARK_H

// kwineffects
#include <kwineffects.h>

// Qt
#include <QQuickView>

namespace KWin
{

class MouseMarkCanvas;

class MouseMarkEffect : public Effect
{
    Q_OBJECT
    Q_PROPERTY(int lineWidth READ lineWidth)
    Q_PROPERTY(QColor color READ color)

public:
    MouseMarkEffect();
    ~MouseMarkEffect() override;

    void reconfigure(ReconfigureFlags flags) override;

    bool isActive() const override;
    int requestedEffectChainPosition() const override;

    int lineWidth() const;
    QColor color() const;

private Q_SLOTS:
    void clearAll();

    void slotMouseChanged(const QPoint& pos, const QPoint &oldPos,
                          Qt::MouseButtons buttons, Qt::MouseButtons oldButtons,
                          Qt::KeyboardModifiers modifiers, Qt::KeyboardModifiers oldModifiers);
    void slotScreenLockingChanged(bool locked);

private:
    void createOverlayWindow();

private:
    int m_lineWidth;
    QColor m_color;

    QQuickView *m_overlayView = nullptr;
    MouseMarkCanvas *m_canvas = nullptr;
    QVector<QPointF> m_currentPath;
};

inline int MouseMarkEffect::requestedEffectChainPosition() const
{
    return 10;
}

inline int MouseMarkEffect::lineWidth() const
{
    return m_lineWidth;
}

inline QColor MouseMarkEffect::color() const
{
    return m_color;
}

} // namespace KWin

#endif
