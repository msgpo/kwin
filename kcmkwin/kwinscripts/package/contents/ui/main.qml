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

import org.kde.kcm 1.2
import org.kde.kconfig 1.0
import org.kde.kirigami 2.10 as Kirigami
import org.kde.private.kcms.kwin.scripts 1.0 as Private

ScrollViewKCM {
    ConfigModule.quickHelp: i18n("This module lets you configure KWin scripts.")

    header: ColumnLayout {
        Kirigami.SearchField {
            id: searchField
            Layout.fillWidth: true
        }
    }

    view: ListView {
        id: scriptsList
        currentIndex: -1

        model: Private.ScriptsFilterProxyModel {
            id: searchModel
            query: searchField.text
            sourceModel: kcm.scriptsModel
        }

        delegate: Script {
            width: scriptsList.width
        }
    }

    footer: ColumnLayout {
        RowLayout {
            Layout.alignment: Qt.AlignRight

            Button {
                icon.name: "document-import"
                text: i18n("Install from File...")
                onClicked: kcm.installFromFile(this)
            }

            Button {
                icon.name: "get-hot-new-stuff"
                text: i18n("Get New Scripts...")
                visible: KAuthorized.authorize("ghns")
                onClicked: kcm.openGHNS(this)
            }
        }
    }
}
