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

#include "keyboard_layout_icon_provider.h"

// KF
#include <KFontUtils>
#include <Plasma/Theme>

// Qt
#include <QIcon>
#include <QPainter>
#include <QStandardPaths>

namespace KWin
{

KeyboardLayoutIconProvider::KeyboardLayoutIconProvider(QObject *parent)
    : QObject(parent)
    , m_theme(new Plasma::Theme(this))
{
    connect(m_theme, &Plasma::Theme::themeChanged,
            this, &KeyboardLayoutIconProvider::invalidateIcons);
}

KeyboardLayoutIconProvider::~KeyboardLayoutIconProvider()
{
}

KeyboardLayoutIconProvider::Mode
KeyboardLayoutIconProvider::mode() const
{
    return m_mode;
}

void KeyboardLayoutIconProvider::setMode(Mode mode)
{
    if (m_mode == mode) {
        return;
    }

    m_mode = mode;

    invalidateIcons();
}

QString KeyboardLayoutIconProvider::fallbackIcon() const
{
    return m_fallbackIcon;
}

void KeyboardLayoutIconProvider::setFallbackIcon(const QString &name)
{
    m_fallbackIcon = name;
}

QIcon KeyboardLayoutIconProvider::flag(const QString &layout)
{
    QIcon icon = getOrRenderFlag(layout);
    if (!icon.isNull()) {
        return icon;
    }

    if (!m_fallbackIcon.isEmpty()) {
        return QIcon::fromTheme(m_fallbackIcon);
    }

    return QIcon();
}

QIcon KeyboardLayoutIconProvider::icon(const QString &layout)
{
    if (m_mode == Mode::Flag) {
        return flag(layout);
    }

    auto iconIt = m_icons.constFind(layout);
    if (iconIt != m_icons.constEnd()) {
        return *iconIt;
    }

    QIcon icon;
    switch (m_mode) {
    case Mode::Label:
        icon = renderLabel(layout);
        break;

    case Mode::LabelOnFlag:
        icon = renderLabelOnFlag(layout);
        break;

    default:
        Q_UNREACHABLE();
        break;
    }

    m_icons[layout] = icon;

    return icon;
}

void KeyboardLayoutIconProvider::invalidateIcons()
{
    m_icons.clear();
    emit iconsInvalidated();
}

static QString layoutToCountryCode(const QString &layout)
{
    // Handle the special case: nec_vndr/jp.
    const int slashIndex = layout.indexOf('/');
    if (slashIndex != -1) {
        return layout.mid(slashIndex + 1);
    }

    return layout;
}

static QSize snapToIconSize(const QSize &size)
{
    const int s = qMax(size.width(), size.height());
    if (s <= 16) {
        return QSize(16, 16);
    }
    if (s <= 22) {
        return QSize(22, 22);
    }
    if (s <= 32) {
        return QSize(32, 32);
    }
    if (s <= 48) {
        return QSize(48, 48);
    }
    if (s <= 64) {
        return QSize(64, 64);
    }
    if (s <= 128) {
        return QSize(128, 128);
    }
    return QSize(256, 256);
}

QIcon KeyboardLayoutIconProvider::renderFlag(const QString &layout)
{
    const QString countryCode = layoutToCountryCode(layout);
    const QString iconPath = QStandardPaths::locate(
        QStandardPaths::GenericDataLocation,
        QStringLiteral("kf5/locale/countries/%1/flag.png").arg(countryCode)
    );
    if (iconPath.isEmpty()) {
        return QIcon();
    }

    const QImage image(iconPath);
    if (image.isNull()) {
        return QIcon();
    }

    QPixmap pixmap(snapToIconSize(image.size()));
    pixmap.fill(Qt::transparent);

    QRect flagRect(image.rect());
    flagRect.moveCenter(pixmap.rect().center());

    QPainter painter(&pixmap);
    painter.drawImage(flagRect, image);
    painter.end();

    return QIcon(pixmap);
}

QIcon KeyboardLayoutIconProvider::renderLabel(const QString &layout)
{
    const QString label = layoutToCountryCode(layout);

    // TODO: Generate pixmaps for different icon sizes.
    QPixmap pixmap(QSize(128, 128));
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(m_theme->color(Plasma::Theme::TextColor));
    KFontUtils::adaptFontSize(painter, label, pixmap.size(), pixmap.height());
    painter.drawText(pixmap.rect(), Qt::AlignCenter, label);
    painter.end();

    return QIcon(pixmap);
}

QIcon KeyboardLayoutIconProvider::renderLabelOnFlag(const QString &layout)
{
    const QIcon flagIcon = getOrRenderFlag(layout);
    if (flagIcon.isNull()) {
        return renderLabel(layout);
    }

    QIcon icon;

    const QString label = layoutToCountryCode(layout);
    const auto sizes = flagIcon.availableSizes();
    for (const QSize &size : sizes) {
        const QPixmap flagPixmap = flagIcon.pixmap(size);

        QPixmap pixmap(flagPixmap.size());
        pixmap.setDevicePixelRatio(flagPixmap.devicePixelRatio());
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.drawPixmap(0, 0, flagPixmap);

        const QRect textRect = QRect(QPoint(0, 0), pixmap.size() / pixmap.devicePixelRatio());

        // TODO: Don't hardcode the text color. A better approach would be to
        // find the median color(by using the median cut algorithm?) and use
        // the complementary color to it as the text color.
        painter.setBrush(Qt::NoBrush);
        painter.setPen(Qt::black);
        KFontUtils::adaptFontSize(painter, label, textRect.size(), textRect.height());
        painter.drawText(textRect, Qt::AlignCenter, label);
        painter.end();

        icon.addPixmap(pixmap);
    }

    return icon;
}

QIcon KeyboardLayoutIconProvider::getOrRenderFlag(const QString &layout)
{
    auto flagIt = m_flags.constFind(layout);
    if (flagIt != m_flags.constEnd()) {
        return *flagIt;
    }

    QIcon icon = renderFlag(layout);
    if (icon.isNull()) {
        return QIcon();
    }

    m_flags[layout] = icon;

    return icon;
}

} // namespace KWin
