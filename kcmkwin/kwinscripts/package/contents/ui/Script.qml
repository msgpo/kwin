/*
 * Copyright (C) 2019 Vlad Zahorodnii <vladzzag@gmail.com>
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

import QtQuick 2.5
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.5 as Kirigami

Kirigami.SwipeListItem {
    id: listItem
    hoverEnabled: true

    onClicked: view.currentIndex = index

    actions: [
        Kirigami.Action {
            visible: model.configurable
            enabled: model.enabled
            icon.name: "configure"
            tooltip: i18nc("@info:tooltip", "Configure...")
            onTriggered: kcm.configure(model.serviceName, this)
        }
    ]

    contentItem: RowLayout {
        id: row

        CheckBox {
            checkState: model.enabled ? CheckBox.Checked : CheckBox.Unchecked
            onToggled: model.enabled = checkState == CheckBox.Checked
        }

        ColumnLayout {
            Layout.topMargin: Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing
            spacing: 0

            Kirigami.Heading {
                Layout.fillWidth: true
                level: 5
                text: model.name
                wrapMode: Text.Wrap
            }

            Label {
                Layout.fillWidth: true
                text: model.description
                opacity: listItem.hovered ? 0.8 : 0.6
                wrapMode: Text.Wrap
            }

            Label {
                Layout.fillWidth: true
                text: i18n("Author: %1\nLicense: %2", model.authorName, model.license)
                opacity: listItem.hovered ? 0.8 : 0.6
                visible: view.currentIndex == index
                wrapMode: Text.Wrap
            }
        }
    }
}
