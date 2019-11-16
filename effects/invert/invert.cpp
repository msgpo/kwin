/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2007 Rivo Laks <rivolaks@hot.ee>
Copyright (C) 2008 Lucas Murray <lmurray@undefinedfire.com>

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

#include "invert.h"
#include "invertconfig.h"

#include <kwinglutils.h>
#include <kwinglplatform.h>

#include <KGlobalAccel>
#include <KLocalizedString>

#include <QAction>
#include <QFile>
#include <QMatrix4x4>
#include <QStandardPaths>

namespace KWin
{

InvertEffect::InvertEffect()
{
    initConfig<InvertConfig>();
    reconfigure(ReconfigureAll);

    const QKeySequence defaultToggleScreenShortcut { Qt::CTRL + Qt::META + Qt::Key_I };
    const QKeySequence defaultToggleWindowShortcut { Qt::CTRL + Qt::META + Qt::Key_U };

    QAction *toggleScreenAction = new QAction(this);
    toggleScreenAction->setObjectName(QStringLiteral("Invert"));
    toggleScreenAction->setText(i18n("Toggle Invert Effect"));
    KGlobalAccel::self()->setDefaultShortcut(toggleScreenAction, { defaultToggleScreenShortcut });
    KGlobalAccel::self()->setShortcut(toggleScreenAction, { defaultToggleScreenShortcut });
    effects->registerGlobalShortcut(defaultToggleScreenShortcut, toggleScreenAction);
    connect(toggleScreenAction, &QAction::triggered, this, &InvertEffect::toggleScreenInversion);

    QAction *toggleWindowAction = new QAction(this);
    toggleWindowAction->setObjectName(QStringLiteral("InvertWindow"));
    toggleWindowAction->setText(i18n("Toggle Invert Effect on Window"));
    KGlobalAccel::self()->setDefaultShortcut(toggleWindowAction, { defaultToggleWindowShortcut });
    KGlobalAccel::self()->setShortcut(toggleWindowAction, { defaultToggleWindowShortcut });
    effects->registerGlobalShortcut(defaultToggleWindowShortcut, toggleWindowAction);
    connect(toggleWindowAction, &QAction::triggered, this, &InvertEffect::toggleWindowInversion);

    connect(effects, &EffectsHandler::windowClosed, this, &InvertEffect::slotWindowClosed);
}

InvertEffect::~InvertEffect()
{
    delete m_shader;
}

void InvertEffect::reconfigure(ReconfigureFlags flags)
{
    Q_UNUSED(flags)

    delete m_shader;
    m_shader = nullptr;
    m_isInited = false;
    m_isValid = true;

    InvertConfig::self()->read();
    m_mode = Mode(InvertConfig::self()->mode());
}

bool InvertEffect::supported()
{
    return effects->compositingType() == OpenGL2Compositing;
}

static QString shaderFileName(InvertEffect::Mode mode)
{
    switch (mode) {
    case InvertEffect::NaiveMode:
        return QStringLiteral("invert-naive.frag");
    case InvertEffect::ShiftMode:
    default:
        return QStringLiteral("invert-shift.frag");
    }
}

bool InvertEffect::initializeShader()
{
    const QString fragmentShaderFileName = shaderFileName(m_mode);

    m_isInited = true;

    m_shader = ShaderManager::instance()->generateShaderFromResources(ShaderTrait::MapTexture, QString(),
                                                                      fragmentShaderFileName);
    if (!m_shader->isValid()) {
        qCCritical(KWINEFFECTS) << "Could not load fragment shader" << fragmentShaderFileName;
        return false;
    }

    return true;
}

void InvertEffect::drawWindow(EffectWindow *w, int mask, const QRegion &region, WindowPaintData &data)
{
    // Load if we haven't already
    if (m_isValid && !m_isInited) {
        m_isValid = initializeShader();
    }

    const bool useShader = m_isValid && (m_invertAllWindows != m_windows.contains(w));
    if (useShader) {
        ShaderManager *shaderManager = ShaderManager::instance();
        shaderManager->pushShader(m_shader);
        data.shader = m_shader;
    }

    effects->drawWindow(w, mask, region, data);

    if (useShader) {
        ShaderManager::instance()->popShader();
    }
}

void InvertEffect::paintEffectFrame(EffectFrame *frame, const QRegion &region, double opacity, double frameOpacity)
{
    if (m_isValid && m_invertAllWindows) {
        frame->setShader(m_shader);
        ShaderBinder binder(m_shader);
        effects->paintEffectFrame(frame, region, opacity, frameOpacity);
    } else {
        effects->paintEffectFrame(frame, region, opacity, frameOpacity);
    }
}

void InvertEffect::slotWindowClosed(EffectWindow *w)
{
    m_windows.removeOne(w);
}

void InvertEffect::toggleScreenInversion()
{
    m_invertAllWindows = !m_invertAllWindows;
    effects->addRepaintFull();
}

void InvertEffect::toggleWindowInversion()
{
    if (!effects->activeWindow()) {
        return;
    }
    if (!m_windows.contains(effects->activeWindow())) {
        m_windows.append(effects->activeWindow());
    } else {
        m_windows.removeOne(effects->activeWindow());
    }
    effects->activeWindow()->addRepaintFull();
}

bool InvertEffect::isActive() const
{
    return m_isValid && (m_invertAllWindows || !m_windows.isEmpty());
}

bool InvertEffect::provides(Feature feature)
{
    return feature == ScreenInversion;
}

} // namespace KWin

