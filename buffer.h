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

#include "kwin_export.h"

#include <QImage>
#include <QObject>
#include <QRegion>

namespace KWin
{

class BufferPrivate;
class GLTexture;
class Toplevel;

class KWIN_EXPORT Buffer : public QObject
{
    Q_OBJECT

public:
    explicit Buffer(QObject *parent = nullptr);
    ~Buffer() override;

    virtual void setToplevel(Toplevel *toplevel);
    virtual Toplevel *toplevel() const;

    virtual bool isDirty() const;
    virtual bool isValid() const;

    virtual bool hasAlphaChannel() const;

    virtual QRegion damage() const;
    virtual QRegion shape() const;
    virtual qreal scale() const;

    virtual bool create();
    virtual void update();

    virtual GLTexture *toOpenGLTexture() const;
    virtual QImage toImage() const;
};

} // namespace KWin
