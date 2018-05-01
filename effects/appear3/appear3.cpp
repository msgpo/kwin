/*
 * Copyright (C) 2018 Vlad Zagorodniy <vladzzag@gmail.com>
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

#include "appear3.h"

// KConfigSkeleton
#include "appear3config.h"

namespace KWin {

Appear3Effect::Appear3Effect()
{
    initConfig<Appear3Config>();
    reconfigure(ReconfigureAll);

    connect(effects, &EffectsHandler::windowAdded,
            this, &Appear3Effect::start);
    connect(effects, &EffectsHandler::windowClosed,
            this, &Appear3Effect::stop);
    connect(effects, &EffectsHandler::windowDeleted,
            this, &Appear3Effect::stop);
    connect(effects, &EffectsHandler::windowMinimized,
            this, &Appear3Effect::stop);
}

Appear3Effect::~Appear3Effect()
{
}

void Appear3Effect::reconfigure(ReconfigureFlags flags)
{
    Q_UNUSED(flags);

    Appear3Config::self()->read();
    m_blacklist = Appear3Config::blacklist().toSet();
    m_duration = animationTime(Appear3Config::duration() > 0
        ? Appear3Config::duration()
        : 160);
    m_opacity = Appear3Config::opacity();
    m_distance = Appear3Config::distance();
}

void Appear3Effect::prePaintScreen(ScreenPrePaintData& data, int time)
{
    auto it = m_animations.begin();
    while (it != m_animations.end()) {
        QTimeLine* t = *it;
        t->setCurrentTime(t->currentTime() + time);
        if (t->currentTime() >= m_duration) {
            delete t;
            it = m_animations.erase(it);
        } else {
            ++it;
        }
    }

    if (! m_animations.isEmpty()) {
        data.mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS;
    }

    effects->prePaintScreen(data, time);
}

void Appear3Effect::prePaintWindow(EffectWindow* w, WindowPrePaintData& data, int time)
{
    if (m_animations.contains(w)) {
        data.setTransformed();
    }

    effects->prePaintWindow(w, data, time);
}

void Appear3Effect::paintWindow(EffectWindow* w, int mask, QRegion region, WindowPaintData& data)
{
    const auto it = m_animations.constFind(w);
    if (it != m_animations.cend()) {
        const qreal t = (*it)->currentValue();

        data.setZTranslation(interpolate(m_distance, 0, t));
        data.multiplyOpacity(interpolate(m_opacity, 1, t));
    }

    effects->paintWindow(w, mask, region, data);
}

void Appear3Effect::postPaintScreen()
{
    if (! m_animations.isEmpty()) {
        effects->addRepaintFull();
    }
    effects->postPaintScreen();
}

bool Appear3Effect::isActive() const
{
    return !m_animations.isEmpty();
}

bool Appear3Effect::supported()
{
    return effects->isOpenGLCompositing()
        && effects->animationsSupported();
}

bool Appear3Effect::shouldAnimate(const EffectWindow* w) const
{
    if (effects->activeFullScreenEffect()) {
        return false;
    }

    const auto* addGrab = w->data(WindowAddedGrabRole).value<void*>();
    if (addGrab != nullptr && addGrab != this) {
        return false;
    }

    if (! w->isManaged()) {
        return false;
    }

    if (m_blacklist.contains(w->windowClass())) {
        return false;
    }

    return w->isNormalWindow()
        || w->isDialog();
}

void Appear3Effect::start(EffectWindow* w)
{
    if (! shouldAnimate(w)) {
        return;
    }

    // Tell other effects(like fade, for example) to ignore this window.
    w->setData(WindowAddedGrabRole, QVariant::fromValue(static_cast<void*>(this)));

    auto* t = new QTimeLine(m_duration, this);
    t->setCurveShape(QTimeLine::EaseInCurve);
    m_animations.insert(w, t);
}

void Appear3Effect::stop(EffectWindow* w)
{
    if (m_animations.isEmpty()) {
        return;
    }
    auto it = m_animations.find(w);
    if (it == m_animations.end()) {
        return;
    }
    delete *it;
    m_animations.erase(it);
}

} // namespace KWin
