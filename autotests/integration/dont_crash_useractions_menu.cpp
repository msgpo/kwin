/********************************************************************
KWin - the KDE window manager
This file is part of the KDE project.

Copyright (C) 2017 Martin Flöser <mgraesslin@kde.org>

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
#include "kwin_wayland_test.h"
#include "cursor.h"
#include "keyboard_input.h"
#include "platform.h"
#include "pointer_input.h"
#include "xdgshellclient.h"
#include "screens.h"
#include "useractions.h"
#include "wayland_server.h"
#include "workspace.h"

#include <KWayland/Client/compositor.h>
#include <KWayland/Client/keyboard.h>
#include <KWayland/Client/pointer.h>
#include <KWayland/Client/seat.h>
#include <KWayland/Client/shm_pool.h>
#include <KWayland/Client/surface.h>
#include <KWayland/Client/touch.h>

#include <linux/input.h>

using namespace KWin;
using namespace KWayland::Client;

static const QString s_socketName = QStringLiteral("wayland_test_kwin_dont_crash_useractions_menu-0");

class TestDontCrashUseractionsMenu : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void testShowHideShowUseractionsMenu();
};

void TestDontCrashUseractionsMenu::initTestCase()
{
    qRegisterMetaType<KWin::XdgShellClient *>();
    qRegisterMetaType<KWin::AbstractClient*>();
    QSignalSpy workspaceCreatedSpy(kwinApp(), &Application::workspaceCreated);
    QVERIFY(workspaceCreatedSpy.isValid());
    kwinApp()->platform()->setInitialWindowSize(QSize(1280, 1024));
    QVERIFY(waylandServer()->init(s_socketName.toLocal8Bit()));
    QMetaObject::invokeMethod(kwinApp()->platform(), "setVirtualOutputs", Qt::DirectConnection, Q_ARG(int, 2));

    // force style to breeze as that's the one which triggered the crash
    QVERIFY(kwinApp()->setStyle(QStringLiteral("breeze")));

    kwinApp()->start();
    QVERIFY(workspaceCreatedSpy.wait());
    QCOMPARE(screens()->count(), 2);
    QCOMPARE(screens()->geometry(0), QRect(0, 0, 1280, 1024));
    QCOMPARE(screens()->geometry(1), QRect(1280, 0, 1280, 1024));
    waylandServer()->initWorkspace();
}

void TestDontCrashUseractionsMenu::init()
{
    QVERIFY(Test::setupWaylandConnection());

    screens()->setCurrent(0);
    KWin::Cursor::setPos(QPoint(1280, 512));
}

void TestDontCrashUseractionsMenu::cleanup()
{
    Test::destroyWaylandConnection();
}

void TestDontCrashUseractionsMenu::testShowHideShowUseractionsMenu()
{
    // this test creates the condition of BUG 382063
    QScopedPointer<Surface> surface1(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface1(Test::createXdgShellStableSurface(surface1.data()));
    auto client = Test::renderAndWaitForShown(surface1.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client);

    workspace()->showWindowMenu(QRect(), client);
    auto userActionsMenu = workspace()->userActionsMenu();
    QTRY_VERIFY(userActionsMenu->isShown());
    QVERIFY(userActionsMenu->hasClient());

    kwinApp()->platform()->keyboardKeyPressed(KEY_ESC, 0);
    kwinApp()->platform()->keyboardKeyReleased(KEY_ESC, 1);
    QTRY_VERIFY(!userActionsMenu->isShown());
    QVERIFY(!userActionsMenu->hasClient());

    // and show again, this triggers BUG 382063
    workspace()->showWindowMenu(QRect(), client);
    QTRY_VERIFY(userActionsMenu->isShown());
    QVERIFY(userActionsMenu->hasClient());
}

WAYLANDTEST_MAIN(TestDontCrashUseractionsMenu)
#include "dont_crash_useractions_menu.moc"
