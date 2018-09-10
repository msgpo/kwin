/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

 Copyright (C) 2012 Martin Gräßlin <mgraesslin@kde.org>
 Copyright (C) 2018 David Edmundson <davidedmundson@kde.org>

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

#ifndef KWIN_SCRIPTEDEFFECT_H
#define KWIN_SCRIPTEDEFFECT_H

#include <kwinanimationeffect.h>

#include <QJSValue>

class KConfigLoader;
class KPluginMetaData;
class QJSEngine;

namespace KWin
{

class KWIN_EXPORT ScriptedEffect : public KWin::AnimationEffect
{
    Q_OBJECT
    Q_ENUMS(DataRole)
    Q_ENUMS(Qt::Axis)
    Q_ENUMS(Anchor)
    Q_ENUMS(MetaType)
    Q_ENUMS(EasingCurve)
public:
    // copied from kwineffects.h
    enum DataRole {
        // Grab roles are used to force all other animations to ignore the window.
        // The value of the data is set to the Effect's `this` value.
        WindowAddedGrabRole = 1,
        WindowClosedGrabRole,
        WindowMinimizedGrabRole,
        WindowUnminimizedGrabRole,
        WindowForceBlurRole, ///< For fullscreen effects to enforce blurring of windows,
        WindowBlurBehindRole, ///< For single windows to blur behind
        WindowForceBackgroundContrastRole, ///< For fullscreen effects to enforce the background contrast,
        WindowBackgroundContrastRole, ///< For single windows to enable Background contrast
        LanczosCacheRole
    };
    enum EasingCurve {
        GaussianCurve = 128
    };
    static ScriptedEffect *create(const QString &effectName, const QString &pathToScript, int chainPosition);
    static ScriptedEffect *create(const KPluginMetaData &effect);
    ~ScriptedEffect() override;
    static bool supported();

    const QString &scriptFile() const {
        return m_scriptFile;
    }

    void reconfigure(ReconfigureFlags flags) override;

    int requestedEffectChainPosition() const override {
        return m_chainPosition;
    }
    QString activeConfig() const;
    void setActiveConfig(const QString &name);
    /**
     * Whether another effect has grabbed the @p w with the given @p grabRole.
     * @param w The window to check
     * @param grabRole The grab role to check
     * @returns @c true if another window has grabbed the effect, @c false otherwise
     **/
    Q_SCRIPTABLE bool isGrabbed(KWin::EffectWindow *w, DataRole grabRole);
    /**
     * Reads the value from the configuration data for the given key.
     * @param key The key to search for
     * @param defaultValue The value to return if the key is not found
     * @returns The config value if present
     **/
    Q_SCRIPTABLE QVariant readConfig(const QString &key, const QVariant defaultValue = QVariant());
    Q_SCRIPTABLE bool registerShortcut(const QString &objectName, const QString &text, const QString &keySequence, const QJSValue &callback);

    Q_SCRIPTABLE int displayWidth() const;
    Q_SCRIPTABLE int displayHeight() const;
    Q_SCRIPTABLE int animationTime(int defaultTime) const;

    Q_SCRIPTABLE bool registerScreenEdge(int edge, const QJSValue &callback);
    Q_SCRIPTABLE bool unregisterScreenEdge(int edge);
    Q_SCRIPTABLE bool registerTouchScreenEdge(int edge, const QJSValue &callback);
    Q_SCRIPTABLE bool unregisterTouchScreenEdge(int edge);

    QHash<int, QList<QJSValue > > &screenEdgeCallbacks() {
        return m_screenEdgeCallbacks;
    }
public Q_SLOTS:
    int animate(KWin::EffectWindow *w, Attribute a, int ms, const QJSValue &to, const QJSValue &from = QJSValue(), uint metaData = 0, int curve = QEasingCurve::Linear, int delay = 0);
    QJSValue animate(const QJSValue &object);

    int set(KWin::EffectWindow *w, Attribute a, int ms, const QJSValue &to, const QJSValue &from = QJSValue(), uint metaData = 0, int curve = QEasingCurve::Linear, int delay = 0);
    QJSValue set(const QJSValue &object);

    bool retarget(int animationId, const QJSValue &newTarget, int newRemainingTime = -1);
    bool retarget(const QList<int> &animationIds, const QJSValue &newTarget, int newRemainingTime = -1);

    bool cancel(int animationId);
    bool cancel(const QList<int> &animationIds);

    bool borderActivated(ElectricBorder border) override;

Q_SIGNALS:
    /**
     * Signal emitted whenever the effect's config changed.
     **/
    void configChanged();
    void animationEnded(KWin::EffectWindow *w, int animationId);

protected:
    ScriptedEffect();
    QJSEngine *engine() const;
    bool init(const QString &effectName, const QString &pathToScript);
    void animationEnded(KWin::EffectWindow *w, Attribute a, uint meta);

private:
    //wrapper round animateGlobal/setGlobal that parses the animations blob.
    QJSValue startAnimation(const QJSValue &object, bool settingPersists);
    QJSValue createError(const QString &errorMessage);
    QJSEngine *m_engine;
    QString m_effectName;
    QString m_scriptFile;
    QHash<int, QList<QJSValue> > m_screenEdgeCallbacks;
    KConfigLoader *m_config;
    int m_chainPosition;
    QHash<int, QAction*> m_touchScreenEdgeCallbacks;
};

}

#endif // KWIN_SCRIPTEDEFFECT_H
