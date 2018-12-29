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

#include "animationsmodel.h"

namespace KWin
{

AnimationsModel::AnimationsModel(QObject *parent)
    : EffectModel(parent)
{
    connect(this, &AnimationsModel::currentIndexChanged, this,
        [this] {
            const QModelIndex index_ = index(m_currentIndex, 0);
            if (!index_.isValid()) {
                return;
            }
            const bool configurable = index_.data(ConfigurableRole).toBool();
            if (configurable != m_currentConfigurable) {
                m_currentConfigurable = configurable;
                emit currentConfigurableChanged();
            }
        }
    );
}

bool AnimationsModel::enabled() const
{
    return m_enabled;
}

void AnimationsModel::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        emit enabledChanged();
    }
}

int AnimationsModel::currentIndex() const
{
    return m_currentIndex;
}

void AnimationsModel::setCurrentIndex(int index)
{
    if (m_currentIndex != index) {
        m_currentIndex = index;
        emit currentIndexChanged();
    }
}

bool AnimationsModel::currentConfigurable() const
{
    return m_currentConfigurable;
}

bool AnimationsModel::shouldStore(const EffectData &data) const
{
    return data.untranslatedCategory.contains(
        QStringLiteral("Virtual Desktop Switching Animation"), Qt::CaseInsensitive);
}

EffectModel::EffectStatus AnimationsModel::status(int row) const
{
    return EffectStatus(data(index(row, 0), static_cast<int>(EffectStatusRole)).toInt());
}

bool AnimationsModel::modelCurrentEnabled() const
{
    for (int i = 0; i < rowCount(); ++i) {
        if (status(i) != EffectStatus::Disabled) {
            return true;
        }
    }

    return false;
}

int AnimationsModel::modelCurrentIndex() const
{
    for (int i = 0; i < rowCount(); ++i) {
        if (status(i) != EffectStatus::Disabled) {
            return i;
        }
    }

    return 0;
}

void AnimationsModel::load()
{
    EffectModel::load();
    setEnabled(modelCurrentEnabled());
    setCurrentIndex(modelCurrentIndex());
}

void AnimationsModel::save()
{
    for (int i = 0; i < rowCount(); ++i) {
        const auto status = (m_enabled && i == m_currentIndex)
            ? EffectModel::EffectStatus::Enabled
            : EffectModel::EffectStatus::Disabled;
        updateEffectStatus(index(i, 0), status);
    }

    EffectModel::save();
}

void AnimationsModel::defaults()
{
    EffectModel::defaults();
    setEnabled(modelCurrentEnabled());
    setCurrentIndex(modelCurrentIndex());
}

bool AnimationsModel::needsSave() const
{
    KConfigGroup kwinConfig(KSharedConfig::openConfig("kwinrc"), "Plugins");

    for (int i = 0; i < rowCount(); ++i) {
        const QModelIndex index_ = index(i, 0);
        const bool enabledConfig = kwinConfig.readEntry(
            index_.data(ServiceNameRole).toString() + QLatin1String("Enabled"),
            index_.data(EnabledByDefaultRole).toBool()
        );
        const bool enabled = (m_enabled && i == m_currentIndex);

        if (enabled != enabledConfig) {
            return true;
        }
    }

    return false;
}

}
