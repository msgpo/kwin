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

#pragma once

// Qt
#include <QHash>
#include <QObject>

namespace Plasma
{
class Theme;
}

namespace KWin
{

class KeyboardLayoutIconProvider : public QObject
{
    Q_OBJECT

public:
    explicit KeyboardLayoutIconProvider(QObject *parent = nullptr);
    ~KeyboardLayoutIconProvider() override;

    enum class Mode {
        Flag,       ///< Show the country flag.
        Label,      ///< Show the country code.
        LabelOnFlag ///< Show the country code on the country flag.
    };

    /**
     * Returns the display mode.
     *
     * The display mode specifies what the keyboard layout indicator
     * should display (e.g. the country flag or the country code).
     *
     * @returns The current display mode.
     * @see icon
     **/
    Mode mode() const;

    /**
     * Sets the display mode.
     *
     * @param mode The display mode.
     * @see icon
     **/
    void setMode(Mode mode);

    /**
     * Returns the name of the fallback icon.
     *
     * The fallback icon will be used, for example, when icon of the country
     * flag for a given keyboard layout cannot be found.
     *
     * @returns The name of the fallback icon.
     **/
    QString fallbackIcon() const;

    /**
     * Sets the name of the fallback icon.
     *
     * @param name The name of the icon in the current icon theme.
     **/
    void setFallbackIcon(const QString &name);

    /**
     * Returns icon of the country flag for a given layout.
     *
     * @param layout The name of the keyboard layout.
     * @returns Icon of the country flag.
     **/
    QIcon flag(const QString &layout);

    /**
     * Returns icon of the keyboard layout indicator for a given layout.
     *
     * @param layout The name of the keyboard layout.
     * @returns Icon for the given keyboard layout.
     * @see mode
     * @see setMode
     **/
    QIcon icon(const QString &layout);

Q_SIGNALS:
    /**
     * This signal is emitted when provided icons are no longer actual.
     *
     * Icons can become invalid, for example, when user changes the desktop
     * theme. In that case, we have to re-render icons because the text
     * color might have been changed.
     **/
    void iconsInvalidated();

private Q_SLOTS:
    void invalidateIcons();

private:
    QIcon renderFlag(const QString &layout);
    QIcon renderLabel(const QString &layout);
    QIcon renderLabelOnFlag(const QString &layout);
    QIcon getOrRenderFlag(const QString &layout);

    Plasma::Theme *m_theme;
    Mode m_mode = Mode::Flag;
    QHash<QString, QIcon> m_flags;
    QHash<QString, QIcon> m_icons;
    QString m_fallbackIcon;

    Q_DISABLE_COPY(KeyboardLayoutIconProvider)
};

} // namespace KWin
