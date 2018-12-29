/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2013 Antonis Tsiapaliokas <kok3rs@gmail.com>
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

#pragma once

#include <kwin_export.h>

#include <KPluginInfo>
#include <KSharedConfig>

#include <QAbstractItemModel>
#include <QString>
#include <QUrl>

namespace KWin
{

class KWIN_EXPORT EffectModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    /**
     * This enum type is used to specify data roles.
     **/
    enum EffectRoles {
        /**
         * The user-friendly name of the effect.
         **/
        NameRole = Qt::UserRole + 1,
        /**
         * The description of the effect.
         **/
        DescriptionRole,
        /**
         * The name of the effect's author. If there are several authors, they
         * will be comma separated.
         **/
        AuthorNameRole,
        /**
         * The email of the effect's author. If there are several authors, the
         * emails will be comma separated.
         **/
        AuthorEmailRole,
        /**
         * The license of the effect.
         **/
        LicenseRole,
        /**
         * The version of the effect.
         **/
        VersionRole,
        /**
         * The category of the effect.
         **/
        CategoryRole,
        /**
         * The service name(plugin name) of the effect.
         **/
        ServiceNameRole,
        /**
         * The icon name of the effect.
         **/
        IconNameRole,
        /**
         * Whether the effect is enabled or disabled.
         **/
        EffectStatusRole,
        /**
         * Link to a video demonstration of the effect.
         **/
        VideoRole,
        /**
         * Link to the home page of the effect.
         **/
        WebsiteRole,
        /**
         * Whether the effect is supported.
         **/
        SupportedRole,
        /**
         * The exclusive group of the effect.
         **/
        ExclusiveRole,
        /**
         * Whether the effect is internal.
         **/
        InternalRole,
        /**
         * Whether the effect has a KCM.
         **/
        ConfigurableRole,
        /**
         * Whether the effect is scripted.
         **/
        ScriptedRole,
        /**
         * Whether the effect is enabled by default.
         **/
        EnabledByDefaultRole
    };

    /**
     * This enum type is used to specify the status of a given effect.
     **/
    enum class EffectStatus {
        /**
         * The effect is disabled.
         **/
        Disabled = Qt::Unchecked,
        /**
         * An enable function is used to determine whether the effect is enabled.
         * For example, such function can be useful to disable the blur effect
         * when running in a virtual machine.
         **/
        EnabledUndeterminded = Qt::PartiallyChecked,
        /**
         * The effect is enabled.
         **/
        Enabled = Qt::Checked
    };

    explicit EffectModel(QObject *parent = nullptr);

    // Reimplemented from QAbstractItemModel.
    QHash<int, QByteArray> roleNames() const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    /**
     * Changes the status of a given effect.
     *
     * @param rowIndex An effect represented by the given index.
     * @param effectState The new state.
     * @note In order to actually apply the change, you have to call @link save.
     **/
    void updateEffectStatus(const QModelIndex &rowIndex, EffectStatus effectState);

    /**
     * Loads effects.
     *
     * You have to call this method in order to populate the model.
     **/
    void load();

    /**
     * Saves status of each modified effect.
     **/
    void save();

    /**
     * Resets the status of each effect to the default state.
     *
     * @note In order to actually apply the change, you have to call @link save.
     **/
    void defaults();

protected:
    enum class Kind {
        Builtin,
        Binary,
        Scripted
    };

    struct EffectData {
        QString name;
        QString description;
        QString authorName;
        QString authorEmail;
        QString license;
        QString version;
        QString untranslatedCategory;
        QString category;
        QString serviceName;
        QString iconName;
        EffectStatus effectStatus;
        bool enabledByDefault;
        bool enabledByDefaultFunction;
        QUrl video;
        QUrl website;
        bool supported;
        QString exclusiveGroup;
        bool internal;
        bool configurable;
        Kind kind;
        bool changed = false;
    };

    /**
     * Returns whether the given effect should be stored in the model.
     *
     * @param data The effect.
     * @returns @c true if the effect should be stored, otherwise @c false.
     **/
    virtual bool shouldStore(const EffectData &data) const;

private:
    void loadBuiltInEffects(const KConfigGroup &kwinConfig, const KPluginInfo::List &configs);
    void loadJavascriptEffects(const KConfigGroup &kwinConfig);
    void loadPluginEffects(const KConfigGroup &kwinConfig, const KPluginInfo::List &configs);
    int findRowByServiceName(const QString &serviceName);
    void syncEffectsToKWin();

    QVector<EffectData> m_effectsList;
    QVector<EffectData> m_effectsChanged;

    Q_DISABLE_COPY(EffectModel)
};

}
