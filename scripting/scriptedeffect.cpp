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

#include "scriptedeffect.h"
#include "meta.h"
#include "scriptingutils.h"
#include "workspace_wrapper.h"
#include "../screens.h"
#include "../screenedge.h"
#include "scripting_logging.h"
// KDE
#include <KConfigGroup>
#include <kconfigloader.h>
#include <KPluginMetaData>
// Qt
#include <QFile>
#include <QQmlEngine>
#include <QStandardPaths>

typedef KWin::EffectWindow* KEffectWindowRef;

Q_DECLARE_METATYPE(KSharedConfigPtr)

namespace KWin
{

struct AnimationSettings {
    enum { Type = 1<<0, Curve = 1<<1, Delay = 1<<2, Duration = 1<<3 };
    AnimationEffect::Attribute type;
    QEasingCurve::Type curve;
    QJSValue from; //should be a KFx2
    QJSValue to;
    int delay;
    uint duration;
    uint set;
    uint metaData;
};

AnimationSettings animationSettingsFromObject(const QJSValue &object)
{
    AnimationSettings settings;
    settings.set = 0;
    settings.metaData = 0;

    settings.to = object.property(QStringLiteral("to"));
    settings.from = object.property(QStringLiteral("from"));

    QJSValue duration = object.property(QStringLiteral("duration"));
    if (duration.isNumber()) {
        settings.duration = duration.toUInt();
        settings.set |= AnimationSettings::Duration;
    } else {
        settings.duration = 0;
    }

    QJSValue delay = object.property(QStringLiteral("delay"));
    if (delay.isNumber()) {
        settings.delay = delay.toInt();
        settings.set |= AnimationSettings::Delay;
    } else {
        settings.delay = 0;
    }

    QJSValue curve = object.property(QStringLiteral("curve"));
    if (curve.isNumber()) {
        settings.curve = static_cast<QEasingCurve::Type>(curve.toInt());
        settings.set |= AnimationSettings::Curve;
    } else {
        settings.curve = QEasingCurve::Linear;
    }

    QJSValue type = object.property(QStringLiteral("type"));
    if (type.isNumber()) {
        settings.type = static_cast<AnimationEffect::Attribute>(type.toInt());
        settings.set |= AnimationSettings::Type;
    } else {
        settings.type = static_cast<AnimationEffect::Attribute>(-1);
    }
    return settings;
}

KWin::FPx2 fpx2FromScriptValue(const QJSValue &value)
{
    if (value.isNull()) {
        return FPx2();
    }
    if (value.isNumber()) {
        return FPx2(value.toNumber());
    }
    if (value.isObject()) {
        QJSValue value1 = value.property(QStringLiteral("value1"));
        QJSValue value2 = value.property(QStringLiteral("value2"));
        if (!value1.isNumber() || !value2.isNumber()) {
            qCDebug(KWIN_SCRIPTING) << "Cannot cast scripted FPx2 to C++";
            return FPx2();
        }
        return FPx2(value1.toNumber(), value2.toNumber());
    }
    return FPx2();
}

ScriptedEffect *ScriptedEffect::create(const KPluginMetaData &effect)
{
    const QString name = effect.pluginId();
    const QString scriptName = effect.value(QStringLiteral("X-Plasma-MainScript"));
    if (scriptName.isEmpty()) {
        qCDebug(KWIN_SCRIPTING) << "X-Plasma-MainScript not set";
        return nullptr;
    }
    const QString scriptFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                      QLatin1String(KWIN_NAME "/effects/") + name + QLatin1String("/contents/") + scriptName);
    if (scriptFile.isNull()) {
        qCDebug(KWIN_SCRIPTING) << "Could not locate the effect script";
        return nullptr;
    }
    return ScriptedEffect::create(name, scriptFile, effect.value(QStringLiteral("X-KDE-Ordering")).toInt());
}

ScriptedEffect *ScriptedEffect::create(const QString& effectName, const QString& pathToScript, int chainPosition)
{
    ScriptedEffect *effect = new ScriptedEffect();
    if (!effect->init(effectName, pathToScript)) {
        delete effect;
        return nullptr;
    }
    effect->m_chainPosition = chainPosition;
    return effect;
}

bool ScriptedEffect::supported()
{
    return effects->animationsSupported();
}

ScriptedEffect::ScriptedEffect()
    : AnimationEffect()
    , m_engine(new QJSEngine(this))
    , m_scriptFile(QString())
    , m_config(nullptr)
    , m_chainPosition(0)
{
}

ScriptedEffect::~ScriptedEffect()
{
}

bool ScriptedEffect::init(const QString &effectName, const QString &pathToScript)
{
    qRegisterMetaType<QJSValueList>();
    QFile scriptFile(pathToScript);
    if (!scriptFile.open(QIODevice::ReadOnly)) {
        qCDebug(KWIN_SCRIPTING) << "Could not open script file: " << pathToScript;
        return false;
    }
    m_engine->installExtensions(QJSEngine::ConsoleExtension);
    m_effectName = effectName;
    m_scriptFile = pathToScript;

    // does the effect contain an KConfigXT file?
    const QString kconfigXTFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String(KWIN_NAME "/effects/") + m_effectName + QLatin1String("/contents/config/main.xml"));
    if (!kconfigXTFile.isNull()) {
        KConfigGroup cg = QCoreApplication::instance()->property("config").value<KSharedConfigPtr>()->group(QStringLiteral("Effect-%1").arg(m_effectName));
        QFile xmlFile(kconfigXTFile);
        m_config = new KConfigLoader(cg, &xmlFile, this);
        m_config->load();
    }

    QJSValue effectsObject = m_engine->newQObject(effects);
    QQmlEngine::setObjectOwnership(effects, QQmlEngine::CppOwnership);
    m_engine->globalObject().setProperty(QStringLiteral("effects"), effectsObject);

    //desktopChanged is overloaded, which is problematic
    //old code exposed the signal also with parameters. QJSEngine does not so we have to fake it
    effectsObject.setProperty("desktopChanged(int,int)", effectsObject.property("desktopChangedCompat"));
    effectsObject.setProperty("desktopChanged", effectsObject.property("desktopChangedCompat"));

    QJSValue selfWrapper = m_engine->newQObject(this);
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    m_engine->globalObject().setProperty(QStringLiteral("Effect"), m_engine->newQMetaObject(&ScriptedEffect::staticMetaObject));
#ifndef KWIN_UNIT_TEST
    m_engine->globalObject().setProperty(QStringLiteral("KWin"), m_engine->newQMetaObject(&QtScriptWorkspaceWrapper::staticMetaObject));
#endif

    m_engine->globalObject().setProperty(QStringLiteral("QEasingCurve"), m_engine->newQMetaObject(&QEasingCurve::staticMetaObject));

    m_engine->globalObject().setProperty("effect", selfWrapper);

    //expose functions at the root level for compatibility
    m_engine->globalObject().setProperty("displayWidth", selfWrapper.property("displayWidth"));
    m_engine->globalObject().setProperty("displayHeight", selfWrapper.property("displayHeight"));
    m_engine->globalObject().setProperty("animationTime", selfWrapper.property("animationTime"));

    m_engine->globalObject().setProperty("registerShortcut", selfWrapper.property("registerShortcut"));
    m_engine->globalObject().setProperty("registerScreenEdge", selfWrapper.property("registerScreenEdge"));
    m_engine->globalObject().setProperty("unregisterScreenEdge", selfWrapper.property("unregisterScreenEdge"));
    m_engine->globalObject().setProperty("registerTouchScreenEdge", selfWrapper.property("registerTouchScreenEdge"));
    m_engine->globalObject().setProperty("unregisterTouchScreenEdge", selfWrapper.property("unregisterTouchScreenEdge"));

    m_engine->globalObject().setProperty("animate", selfWrapper.property("animate"));
    m_engine->globalObject().setProperty("set", selfWrapper.property("set"));
    m_engine->globalObject().setProperty("retarget", selfWrapper.property("retarget"));
    m_engine->globalObject().setProperty("cancel", selfWrapper.property("cancel"));

    QJSValue ret = m_engine->evaluate(QString::fromUtf8(scriptFile.readAll()));

    if (ret.isError()) {
        qCDebug(KWIN_SCRIPTING) << "KWin Effect script encountered an error at [Line " << ret.property("lineNumber").toString() << "]";
        qCDebug(KWIN_SCRIPTING) << "Message: " << ret.property("message").toString();
        qCDebug(KWIN_SCRIPTING) << ": " << ret.toString();
        return false;
    }
    scriptFile.close();
    return true;
}

int ScriptedEffect::displayHeight() const
{
    return screens()->displaySize().height();
}

int ScriptedEffect::animationTime(int defaultTime) const
{
    return Effect::animationTime(defaultTime);
}

int ScriptedEffect::displayWidth() const
{
    return screens()->displaySize().width();
}

void ScriptedEffect::animationEnded(KWin::EffectWindow *w, Attribute a, uint meta)
{
    AnimationEffect::animationEnded(w, a, meta);
    emit animationEnded(w, 0);
}

int ScriptedEffect::animate(KWin::EffectWindow* w, KWin::AnimationEffect::Attribute a, int ms, const QJSValue &to, const QJSValue &from, uint metaData, int curve, int delay)
{
    QEasingCurve qec;
    if (curve < QEasingCurve::Custom)
        qec.setType(static_cast<QEasingCurve::Type>(curve));
    else if (curve == GaussianCurve)
        qec.setCustomType(qecGaussian);
    return AnimationEffect::animate(w, a, metaData, ms, fpx2FromScriptValue(to), qec, delay, fpx2FromScriptValue(from));
}

int ScriptedEffect::set(KWin::EffectWindow* w, KWin::AnimationEffect::Attribute a, int ms, const QJSValue &to, const QJSValue &from, uint metaData, int curve, int delay)
{
    QEasingCurve qec;
    if (curve < QEasingCurve::Custom)
        qec.setType(static_cast<QEasingCurve::Type>(curve));
    else if (curve == GaussianCurve)
        qec.setCustomType(qecGaussian);
    return AnimationEffect::set(w, a, metaData, ms, fpx2FromScriptValue(to), qec, delay, fpx2FromScriptValue(from));
}

bool ScriptedEffect::retarget(int animationId, const QJSValue &newTarget, int newRemainingTime)
{
    return AnimationEffect::retarget(animationId, fpx2FromScriptValue(newTarget), newRemainingTime);
}

bool ScriptedEffect::retarget(const QList<int> &animationIds, const QJSValue &newTarget, int newRemainingTime)
{
    bool ok = true;
    for (int animationId: qAsConst(animationIds)) {
        ok |= retarget(animationId, newTarget, newRemainingTime);
    }
    return ok;
}

QJSValue ScriptedEffect::animate(const QJSValue &args)
{
    return startAnimation(args, false);
}

QJSValue ScriptedEffect::set(const QJSValue &object)
{
    return startAnimation(object, true);
}

QJSValue ScriptedEffect::createError(const QString &errorMessage) {
    return m_engine->evaluate(QString("new Error('%1');").arg(errorMessage));
}

QJSValue ScriptedEffect::startAnimation(const QJSValue &object, bool settingPersists)
{
    QVector<AnimationSettings> settings;
    QJSValue windowProperty = object.property(QStringLiteral("window"));
    if (!windowProperty.isObject()) {
        return createError(QStringLiteral("Window property missing in animation options"));
    }
    auto window = qobject_cast<EffectWindow*>(windowProperty.toQObject());
    if (!window) {
        return createError(QStringLiteral("Window property references invalid window"));
    }

    settings << animationSettingsFromObject(object); // global

    QJSValue animations = object.property(QStringLiteral("animations")); // array
    if (!animations.isNull()) {
        if (!animations.isArray()) {
            return createError(QStringLiteral("Animations provided but not an array"));
        }
        const int length = static_cast<int>(animations.property(QStringLiteral("length")).toInt());
        for (int i = 0; i < length; ++i) {
            QJSValue value = animations.property(QString::number(i));
            if (value.isObject()) {
                AnimationSettings s = animationSettingsFromObject(value);
                const uint set = s.set | settings.at(0).set;
                // Catch show stoppers (incompletable animation)
                if (!(set & AnimationSettings::Type)) {
                    return createError(QStringLiteral("Type property missing in animation options"));
                }
                if (!(set & AnimationSettings::Duration)) {
                    return createError(QStringLiteral("Duration property missing in animation options"));
                }
                // Complete local animations from global settings
                if (!(s.set & AnimationSettings::Duration)) {
                    s.duration = settings.at(0).duration;
                }
                if (!(s.set & AnimationSettings::Curve)) {
                    s.curve = settings.at(0).curve;
                }
                if (!(s.set & AnimationSettings::Delay)) {
                    s.delay = settings.at(0).delay;
                }

                s.metaData = 0;
                typedef QMap<AnimationEffect::MetaType, QString> MetaTypeMap;
                static MetaTypeMap metaTypes({
                                                 {AnimationEffect::SourceAnchor, QStringLiteral("sourceAnchor")},
                                                 {AnimationEffect::TargetAnchor, QStringLiteral("targetAnchor")},
                                                 {AnimationEffect::RelativeSourceX, QStringLiteral("relativeSourceX")},
                                                 {AnimationEffect::RelativeSourceY, QStringLiteral("relativeSourceY")},
                                                 {AnimationEffect::RelativeTargetX, QStringLiteral("relativeTargetX")},
                                                 {AnimationEffect::RelativeTargetY, QStringLiteral("relativeTargetY")},
                                                 {AnimationEffect::Axis, QStringLiteral("axis")}
                                             });

                for (auto it = metaTypes.constBegin(),
                     end = metaTypes.constEnd(); it != end; ++it) {
                    QJSValue metaVal = value.property(*it);
                    if (metaVal.isNumber()) {
                        AnimationEffect::setMetaData(it.key(), metaVal.toInt(), s.metaData);
                    }
                }

                settings << s;
            }
        }
    }

    if (settings.count() == 1) {
        const uint set = settings.at(0).set;
        if (!(set & AnimationSettings::Type)) {
            return createError(QStringLiteral("Type property missing in animation options"));
        }
        if (!(set & AnimationSettings::Duration)) {
            return createError(QStringLiteral("Duration property missing in animation options"));
        }
    } else if (!(settings.at(0).set & AnimationSettings::Type)) { // invalid global
        settings.removeAt(0); // -> get rid of it, only used to complete the others
    }

    if (settings.isEmpty()) {
        return createError(QStringLiteral("No animations provided"));
    }

    QJSValue array = m_engine->newArray(settings.length());
    for (int i = 0; i < settings.count(); i++) {
        const AnimationSettings &setting = settings[i];
        int animationId;
        if (settingPersists) {
            animationId = set(window,
                              setting.type,
                              setting.duration,
                              setting.to,
                              setting.from,
                              setting.metaData,
                              setting.curve,
                              setting.delay);
        } else {
            animationId = animate(window,
                                  setting.type,
                                  setting.duration,
                                  setting.to,
                                  setting.from,
                                  setting.metaData,
                                  setting.curve,
                                  setting.delay);
        }
        array.setProperty(i, animationId);
    }
    return array;
}

bool ScriptedEffect::cancel(int animationId)
{
    return AnimationEffect::cancel(animationId);
}

bool ScriptedEffect::cancel(const QList<int> &animationIds)
{
    bool ok = true;
    for (int animationId: qAsConst(animationIds)) {
        ok |= cancel(animationId);
    }
    return ok;
}

bool ScriptedEffect::isGrabbed(EffectWindow* w, ScriptedEffect::DataRole grabRole)
{
    void *e = w->data(static_cast<KWin::DataRole>(grabRole)).value<void*>();
    if (e) {
        return e != this;
    } else {
        return false;
    }
}

void ScriptedEffect::reconfigure(ReconfigureFlags flags)
{
    AnimationEffect::reconfigure(flags);
    if (m_config) {
        m_config->read();
    }
    emit configChanged();
}

bool ScriptedEffect::registerShortcut(const QString &objectName, const QString &text, const QString &keySequence, const QJSValue &callback)
{
    QAction *a = new QAction(this);
    a->setObjectName(objectName);
    a->setText(text);
    const QKeySequence shortcut = QKeySequence(keySequence);
    KGlobalAccel::self()->setShortcut(a, QList<QKeySequence>() << shortcut);
    input()->registerShortcut(shortcut, a);

    connect(a, &QAction::triggered, this, [this, a, callback]() {
        QJSValue c(callback);

        QJSValue actionObject = m_engine->newQObject(a);
        QQmlEngine::setObjectOwnership(a, QQmlEngine::CppOwnership);
        c.call(QJSValueList({actionObject}));
    });
    return true;
}

bool ScriptedEffect::borderActivated(ElectricBorder edge)
{
    auto it = screenEdgeCallbacks().constFind(edge);
    if (it != screenEdgeCallbacks().constEnd()) {
        for(const QJSValue &value: qAsConst(it.value())) {
            QJSValue callback(value);
            callback.call();
        }
    }
    return true;
}

QVariant ScriptedEffect::readConfig(const QString &key, const QVariant defaultValue)
{
    if (!m_config) {
        return defaultValue;
    }
    return m_config->property(key);
}

bool ScriptedEffect::registerScreenEdge(int edge, const QJSValue &callback)
{
    auto it = screenEdgeCallbacks().find(edge);
    if (it == screenEdgeCallbacks().end()) {
        // not yet registered
        ScreenEdges::self()->reserve(static_cast<KWin::ElectricBorder>(edge), this, "borderActivated");
        screenEdgeCallbacks().insert(edge, QList<QJSValue>() << callback);
    } else {
        it->append(callback);
    }
    return true;
}

bool ScriptedEffect::unregisterScreenEdge(int edge)
{
    auto it = screenEdgeCallbacks().find(edge);
    if (it == screenEdgeCallbacks().end()) {
        //not previously registered
        return false;
    }
    ScreenEdges::self()->unreserve(static_cast<KWin::ElectricBorder>(edge), this);
    screenEdgeCallbacks().erase(it);
    return true;
}

bool ScriptedEffect::registerTouchScreenEdge(int edge, const QJSValue &callback)
{
    if (m_touchScreenEdgeCallbacks.constFind(edge) != m_touchScreenEdgeCallbacks.constEnd()) {
        return false;
    }
    QAction *action = new QAction(this);
    connect(action, &QAction::triggered, this,
            [callback] {
        QJSValue invoke(callback);
        invoke.call();
    }
    );
    ScreenEdges::self()->reserveTouch(KWin::ElectricBorder(edge), action);
    m_touchScreenEdgeCallbacks.insert(edge, action);
    return true;
}

bool ScriptedEffect::unregisterTouchScreenEdge(int edge)
{
    auto it = m_touchScreenEdgeCallbacks.find(edge);
    if (it == m_touchScreenEdgeCallbacks.end()) {
        return false;
    }
    delete it.value();
    m_touchScreenEdgeCallbacks.erase(it);
    return true;
}

QJSEngine *ScriptedEffect::engine() const
{
    return m_engine;
}

} // namespace
