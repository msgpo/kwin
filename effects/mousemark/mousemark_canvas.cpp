/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

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
#include "mousemark_canvas.h"

namespace KWin
{

static QPainterPath toSmoothPainterPath(const QVector<QPointF> &points)
{
    QPainterPath path;

    path.moveTo(points.first());

    for (int i = 0; i < points.size() - 1; ++i) {
        path.quadTo(points[i], 0.5 * (points[i] + points[i + 1]));
    }

    path.lineTo(points.last());

    return path;
}

MouseMarkCanvas::MouseMarkCanvas(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
}

void MouseMarkCanvas::paint(QPainter *painter)
{
    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);

    for (const PathData &pathData : qAsConst(m_paths)) {
        paintPath(painter, pathData);
    }

    if (!m_currentPath.path.isEmpty()) {
        paintPath(painter, m_currentPath);
    }

    painter->restore();
}

void MouseMarkCanvas::paintPath(QPainter *painter, const PathData &data)
{
    painter->setPen(data.pen);
    painter->drawPath(data.path);
}

void MouseMarkCanvas::clear()
{
    m_currentPath = {};
    m_paths.clear();
    update();
}

void MouseMarkCanvas::addPath(const QVector<QPointF> &points, const QPen &pen)
{
    m_paths.append({ toSmoothPainterPath(points), pen });
    update();
}

void MouseMarkCanvas::clearCurrentPath()
{
    m_currentPath = {};
    update();
}

void MouseMarkCanvas::setCurrentPath(const QVector<QPointF> &points, const QPen &pen)
{
    m_currentPath = { toSmoothPainterPath(points), pen };
    update();
}

} // namespace KWin
