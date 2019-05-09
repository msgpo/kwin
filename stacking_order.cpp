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

#include "stacking_order.h"
#include "client.h"
#include "group.h"
#include "toplevel.h"

// Qt
#include <QQueue>

namespace KWin
{

StackingOrder::StackingOrder(QObject *parent)
    : QObject(parent)
{
}

StackingOrder::~StackingOrder()
{
    qDeleteAll(m_constraints);
}

void StackingOrder::add(Toplevel *toplevel)
{
    toplevel->setStackPosition(m_toplevels.count());
    m_toplevels << toplevel;
}

void StackingOrder::remove(Toplevel *toplevel)
{
    const int position = toplevel->stackPosition();
    m_toplevels.removeAt(position);
    shift(position, m_toplevels.count());

    for (int i = m_constraints.count() - 1; i >= 0; --i) {
        Constraint *constraint = m_constraints[i];
        const bool isBelow = (constraint->below == toplevel);
        const bool isAbove = (constraint->above == toplevel);
        if (!isBelow || !isAbove) {
            continue;
        }
        if (isBelow) {
            for (Constraint *child : constraint->children) {
                child->parents.removeOne(constraint);
            }
        } else {
            for (Constraint *parent : constraint->parents) {
                parent->children.removeOne(constraint);
            }
        }
        delete m_constraints.takeAt(i);
    }
}

void StackingOrder::replace(Toplevel *before, Toplevel *after)
{
    const int position = before->stackPosition();

    after->setStackPosition(position);
    m_toplevels[position] = after;

    for (Constraint *constraint : m_constraints) {
        if (constraint->below == before) {
            constraint->below = after;
        } else if (constraint->above == before) {
            constraint->above = after;
        }
    }
}

void StackingOrder::raise(Toplevel *toplevel)
{
    Q_UNUSED(toplevel)
}

void StackingOrder::lower(Toplevel *toplevel)
{
    Q_UNUSED(toplevel)
}

void StackingOrder::restack(const ToplevelList &toplevels)
{
    for (int i = 0; i < toplevels.count() - 1; ++i) {
        Toplevel *below = toplevels.at(i);
        Toplevel *above = toplevels.at(i + 1);
        restack(below, above);
    }
}

void StackingOrder::restack(Toplevel *below, Toplevel *above)
{
    const int belowPosition = below->stackPosition();
    const int abovePosition = above->stackPosition();
    if (belowPosition < abovePosition) {
        return;
    }

    m_toplevels.removeAt(abovePosition);
    m_toplevels.insert(belowPosition, above);

    shift(abovePosition, belowPosition + 1);
}

void StackingOrder::constrain(Toplevel *below, Toplevel *above)
{
    if (below == above) {
        return;
    }

    QVector<Constraint *> parents;
    QVector<Constraint *> children;
    for (Constraint *constraint : m_constraints) {
        if (constraint->below == below && constraint->above == above) {
            return;
        }
        if (constraint->below == above) {
            children << constraint;
        } else if (constraint->above == below) {
            parents << constraint;
        }
    }

    Constraint *constraint = new Constraint();
    constraint->parents = parents;
    constraint->below = below;
    constraint->above = above;
    constraint->children = children;
    m_constraints << constraint;

    for (Constraint *parent : qAsConst(parents)) {
        parent->children << constraint;
    }

    for (Constraint *child : qAsConst(children)) {
        child->parents << constraint;
    }
}

void StackingOrder::unconstrain(Toplevel *below, Toplevel *above)
{
    Constraint *constraint = nullptr;
    for (int i = 0; i < m_constraints.count(); ++i) {
        if (m_constraints[i]->below != below) {
            continue;
        }
        if (m_constraints[i]->above != above) {
            continue;
        }
        constraint = m_constraints.takeAt(i);
        break;
    }

    if (!constraint) {
        return;
    }

    for (Constraint *parent : constraint->parents) {
        parent->children.removeOne(constraint);
    }

    for (Constraint *child : constraint->children) {
        child->parents.removeOne(constraint);
    }

    delete constraint;
}

ToplevelList StackingOrder::toplevels() const
{
    return m_toplevels;
}

void StackingOrder::rebuild()
{
    const ToplevelList old = m_toplevels;

    evaluateLayers();
    evaluateConstraints();

    if (old != m_toplevels) {
        emit changed();
    }
}

void StackingOrder::evaluateConstraints()
{
    if (m_constraints.isEmpty()) {
        return;
    }

    QQueue<Constraint *> constraints;
    constraints.reserve(m_constraints.count());

    for (Constraint *constraint : m_constraints) {
        if (constraint->parents.isEmpty()) {
            constraint->enqueued = true;
            constraints << constraint;
        } else {
            constraint->enqueued = false;
        }
    }

    while (!constraints.isEmpty()) {
        Constraint *constraint = constraints.dequeue();

        restack(constraint->below, constraint->above);

        for (Constraint *child : constraint->children) {
            if (constraint->enqueued) {
                continue;
            }
            child->enqueued = true;
            constraints << child;
        }
    }
}

static bool isLayerPromotable(const Client *client)
{
    switch (client->windowType()) {
    case NET::Dialog:
    case NET::Utility:
        return true;
    default:
        return false;
    }
}

static Layer computeLayer(const Client *client)
{
    Layer layer = client->layer();

    if (!isLayerPromotable(client)) {
        return layer;
    }

    const Group *group = client->group();
    if (!group) {
        return layer;
    }

    const ClientList members = group->members();
    for (const Client *member : members) {
        if (member == client) {
            continue;
        }
        if (member->screen() != client->screen()) {
            continue;
        }
        if (member->layer() > layer) {
            layer = member->layer();
        }
    }

    return layer;
}

static Layer computeLayer(const Toplevel *toplevel)
{
    if (auto client = qobject_cast<const Client *>(toplevel)) {
        return computeLayer(client);
    }

    return toplevel->layer();
}

void StackingOrder::evaluateLayers()
{
    std::array<ToplevelList, NumLayers> toplevels;
    for (Toplevel *toplevel : m_toplevels) {
        const Layer layer = computeLayer(toplevel);
        toplevels[layer] << toplevel;
    }

    m_toplevels.clear();
    for (Layer layer = FirstLayer; layer < NumLayers; ++layer) {
        m_toplevels += toplevels[layer];
    }

    shift(0, m_toplevels.count());
}

void StackingOrder::shift(int start, int end)
{
    for (int i = start; i < end; ++i) {
        m_toplevels[i]->setStackPosition(i);
    }
}

} // namespace KWin
