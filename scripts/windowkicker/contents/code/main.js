/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

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

var s_ignoreGeometryUpdates = false;

workspace.aboutToResizeWorkspace.connect(function () {
    s_ignoreGeometryUpdates = true;
});

workspace.workspaceResized.connect(function () {
    var clients = workspace.clientList();

    for (var i = 0; i < clients.length; ++i) {
        var client = clients[i];

        if (client.specialWindow) {
            continue;
        }

        var centerX = client.lastGeometry.x + client.lastGeometry.width / 2;
        var centerY = client.lastGeometry.y + client.lastGeometry.height / 2;

        var screen = workspace.screenAt(centerX, centerY);
        if (screen === -1 || client.screen === screen) {
            continue;
        }

        client.geometry = client.lastGeometry;
    }

    s_ignoreGeometryUpdates = false;
});

function handleClientAdded(client) {
    if (client.specialWindow) {
        return;
    }

    client.frameGeometryChanged.connect(function () {
        if (s_ignoreGeometryUpdates) {
            return;
        }
        client.lastGeometry = client.geometry;
    });

    client.lastGeometry = client.geometry;
}

var clients = workspace.clientList();
clients.forEach(handleClientAdded);

workspace.clientAdded.connect(handleClientAdded);
