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

#ifndef KWIN_INVERT_H
#define KWIN_INVERT_H

#include <kwineffects.h>

namespace KWin
{

class GLShader;

/**
 * Inverts desktop's colors
 */
class InvertEffect : public Effect
{
    Q_OBJECT

public:
    InvertEffect();
    ~InvertEffect() override;

    /**
     * This enum type is used to specify how exactly colors have to be inverted.
     */
    enum Mode {
        /**
         * Colors are inverted by flipping values in each color channel.
         */
        NaiveMode = 0,
        /**
         * The algorithm is taken from https://github.com/vn971/linux-color-inversion.
         */
        ShiftMode = 1,
    };

    void reconfigure(ReconfigureFlags flags) override;
    void drawWindow(EffectWindow *w, int mask, const QRegion &region, WindowPaintData &data) override;
    void paintEffectFrame(EffectFrame *frame, const QRegion &region, double opacity, double frameOpacity) override;
    bool isActive() const override;
    bool provides(Feature) override;
    int requestedEffectChainPosition() const override;

    static bool supported();

public Q_SLOTS:
    void toggleScreenInversion();
    void toggleWindowInversion();
    void slotWindowClosed(EffectWindow *w);

private:
    bool initializeShader();

    GLShader *m_shader = nullptr;
    QList<EffectWindow *> m_windows;
    Mode m_mode = NaiveMode;
    bool m_isInited = false;
    bool m_isValid = true;
    bool m_invertAllWindows = false;
};

inline int InvertEffect::requestedEffectChainPosition() const
{
    return 99;
}

} // namespace KWin

#endif
