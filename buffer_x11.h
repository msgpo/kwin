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

#include "buffer.h"

namespace KWin
{

class BufferX11Private;

class KWIN_EXPORT BufferX11 : public Buffer
{
    Q_OBJECT

public:
    explicit BufferX11(QObject *parent = nullptr);
    ~BufferX11() override;

    void setToplevel(Toplevel *toplevel) override;
    Toplevel *toplevel() const override;

    bool isDirty() const override;
    bool isValid() const override;

    bool create() override;
    void update() override;

private:
    QScopedPointer<BufferX11Private> d;
};

} // namespace KWin
