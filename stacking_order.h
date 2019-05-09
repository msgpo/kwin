/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2019 Vlad Zagorodniy <vladzzag@gmail.com>

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

// Own
#include "utils.h"

// Qt
#include <QObject>
#include <QVector>

namespace KWin
{

/**
 *
 **/
class Q_DECL_EXPORT StackingOrder : public QObject
{
    Q_OBJECT

public:
    explicit StackingOrder(QObject *parent = nullptr);
    ~StackingOrder() override;

    /**
     * Adds given Toplevel to the StackingOrder.
     *
     * @todo More doc.
     **/
    void add(Toplevel *toplevel);

    /**
     * Removes given Toplevel from the StackingOrder.
     *
     * @todo More doc.
     **/
    void remove(Toplevel *toplevel);

    /**
     *
     **/
    void replace(Toplevel *before, Toplevel *after);

    /**
     * Moves given Toplevel to the top of its layer.
     *
     * @todo More doc.
     **/
    void raise(Toplevel *toplevel);

    /**
     * Moves given Toplevel to the bottom of its layer.
     *
     * @todo More doc.
     **/
    void lower(Toplevel *toplevel);

    /**
     *
     **/
    void restack(const ToplevelList &toplevels);

    /**
     *
     **/
    void restack(Toplevel *below, Toplevel *above);

    /**
     *
     **/
    void constrain(Toplevel *below, Toplevel *above);

    /**
     *
     **/
    void unconstrain(Toplevel *below, Toplevel *above);

    /**
     * Returns toplevels in the StackingOrder.
     *
     * @todo More doc.
     **/
    ToplevelList toplevels() const;

    /**
     *
     **/
    void rebuild();

Q_SIGNALS:
    /**
     * Emitted when the stacking order has changed.
     **/
    void changed();

private:
    void evaluateConstraints();
    void evaluateLayers();
    void shift(int start, int end);

    ToplevelList m_toplevels;

    struct Constraint
    {
        Toplevel *below;
        Toplevel *above;
        QVector<Constraint *> parents;
        QVector<Constraint *> children;

        // Used to prevent cycles.
        bool enqueued = false;
    };
    QVector<Constraint *> m_constraints;

    Q_DISABLE_COPY(StackingOrder)
};

} // namespace KWin
