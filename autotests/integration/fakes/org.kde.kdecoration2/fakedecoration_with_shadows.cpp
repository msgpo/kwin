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

#include <QPainter>

#include <KDecoration2/Decoration>
#include <KPluginFactory>


class FakeDecoWithShadows : public KDecoration2::Decoration
{
    Q_OBJECT
public:
    explicit FakeDecoWithShadows(QObject *parent = nullptr, const QVariantList &args = QVariantList())
        : Decoration(parent, args) {}
    virtual ~FakeDecoWithShadows() {}

    void paint(QPainter *painter, const QRect &repaintRegion) override {
        Q_UNUSED(painter);
        Q_UNUSED(repaintRegion);
    }

public Q_SLOTS:
    void init() override {
        int shadowSize = 128;
        int offsetTop = 64;
        int offsetLeft = 48;
        QRect shadowRect(0, 0, 4 * shadowSize + 1, 4 * shadowSize + 1);

        QImage shadowTexture(shadowRect.size(), QImage::Format_ARGB32_Premultiplied);
        shadowTexture.fill(Qt::transparent);

        QMargins padding(shadowSize - offsetLeft,
                         shadowSize - offsetTop,
                         shadowSize + offsetLeft,
                         shadowSize + offsetTop);

        auto decoShadow = QSharedPointer<KDecoration2::DecorationShadow>::create();
        decoShadow->setPadding(padding);
        decoShadow->setInnerShadowRect(QRect(shadowRect.center(), QSize(1, 1)));
        decoShadow->setShadow(shadowTexture);

        setShadow(decoShadow);
    }
};

K_PLUGIN_FACTORY_WITH_JSON(
    FakeDecoWithShadowsFactory,
    "fakedecoration_with_shadows.json",
    registerPlugin<FakeDecoWithShadows>();
)

#include "fakedecoration_with_shadows.moc"
