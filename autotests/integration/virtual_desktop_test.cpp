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
#include "main.h"
#include "platform.h"
#include "screens.h"
#include "xdgshellclient.h"
#include "wayland_server.h"
#include "virtualdesktops.h"

#include <KWayland/Client/surface.h>

using namespace KWin;
using namespace KWayland::Client;

static const QString s_socketName = QStringLiteral("wayland_test_kwin_virtualdesktop-0");

class VirtualDesktopTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void testNetCurrentDesktop();
    void testLastDesktopRemoved_data();
    void testLastDesktopRemoved();
    void testWindowOnMultipleDesktops_data();
    void testWindowOnMultipleDesktops();
    void testRemoveDesktopWithWindow_data();
    void testRemoveDesktopWithWindow();
};

void VirtualDesktopTest::initTestCase()
{
    qRegisterMetaType<KWin::XdgShellClient *>();
    qRegisterMetaType<KWin::AbstractClient*>();
    QSignalSpy workspaceCreatedSpy(kwinApp(), &Application::workspaceCreated);
    QVERIFY(workspaceCreatedSpy.isValid());
    kwinApp()->platform()->setInitialWindowSize(QSize(1280, 1024));
    QVERIFY(waylandServer()->init(s_socketName.toLocal8Bit()));

    kwinApp()->setConfig(KSharedConfig::openConfig(QString(), KConfig::SimpleConfig));
    qputenv("KWIN_XKB_DEFAULT_KEYMAP", "1");
    qputenv("XKB_DEFAULT_RULES", "evdev");

    kwinApp()->start();
    QVERIFY(workspaceCreatedSpy.wait());
    waylandServer()->initWorkspace();

    if (kwinApp()->x11Connection()) {
        // verify the current desktop x11 property on startup, see BUG: 391034
        Xcb::Atom currentDesktopAtom("_NET_CURRENT_DESKTOP");
        QVERIFY(currentDesktopAtom.isValid());
        Xcb::Property currentDesktop(0, kwinApp()->x11RootWindow(), currentDesktopAtom, XCB_ATOM_CARDINAL, 0, 1);
        bool ok = true;
        QCOMPARE(currentDesktop.value(0, &ok), 0);
        QVERIFY(ok);
    }
}

void VirtualDesktopTest::init()
{
    QVERIFY(Test::setupWaylandConnection());
    screens()->setCurrent(0);
    VirtualDesktopManager::self()->setCount(1);
}

void VirtualDesktopTest::cleanup()
{
    Test::destroyWaylandConnection();
}

void VirtualDesktopTest::testNetCurrentDesktop()
{
    if (!kwinApp()->x11Connection()) {
        QSKIP("Skipped on Wayland only");
    }
    QCOMPARE(VirtualDesktopManager::self()->count(), 1u);
    VirtualDesktopManager::self()->setCount(4);
    QCOMPARE(VirtualDesktopManager::self()->count(), 4u);

    Xcb::Atom currentDesktopAtom("_NET_CURRENT_DESKTOP");
    QVERIFY(currentDesktopAtom.isValid());
    Xcb::Property currentDesktop(0, kwinApp()->x11RootWindow(), currentDesktopAtom, XCB_ATOM_CARDINAL, 0, 1);
    bool ok = true;
    QCOMPARE(currentDesktop.value(0, &ok), 0);
    QVERIFY(ok);

    // go to desktop 2
    VirtualDesktopManager::self()->setCurrent(2);
    currentDesktop = Xcb::Property(0, kwinApp()->x11RootWindow(), currentDesktopAtom, XCB_ATOM_CARDINAL, 0, 1);
    QCOMPARE(currentDesktop.value(0, &ok), 1);
    QVERIFY(ok);

    // go to desktop 3
    VirtualDesktopManager::self()->setCurrent(3);
    currentDesktop = Xcb::Property(0, kwinApp()->x11RootWindow(), currentDesktopAtom, XCB_ATOM_CARDINAL, 0, 1);
    QCOMPARE(currentDesktop.value(0, &ok), 2);
    QVERIFY(ok);

    // go to desktop 4
    VirtualDesktopManager::self()->setCurrent(4);
    currentDesktop = Xcb::Property(0, kwinApp()->x11RootWindow(), currentDesktopAtom, XCB_ATOM_CARDINAL, 0, 1);
    QCOMPARE(currentDesktop.value(0, &ok), 3);
    QVERIFY(ok);

    // and back to first
    VirtualDesktopManager::self()->setCurrent(1);
    currentDesktop = Xcb::Property(0, kwinApp()->x11RootWindow(), currentDesktopAtom, XCB_ATOM_CARDINAL, 0, 1);
    QCOMPARE(currentDesktop.value(0, &ok), 0);
    QVERIFY(ok);
}

void VirtualDesktopTest::testLastDesktopRemoved_data()
{
    QTest::addColumn<Test::XdgShellSurfaceType>("type");

    QTest::newRow("xdgShellV6") << Test::XdgShellSurfaceType::XdgShellV6;
    QTest::newRow("xdgWmBase") << Test::XdgShellSurfaceType::XdgShellStable;
}

void VirtualDesktopTest::testLastDesktopRemoved()
{
    // first create a new desktop
    QCOMPARE(VirtualDesktopManager::self()->count(), 1u);
    VirtualDesktopManager::self()->setCount(2);
    QCOMPARE(VirtualDesktopManager::self()->count(), 2u);

    // switch to last desktop
    VirtualDesktopManager::self()->setCurrent(VirtualDesktopManager::self()->desktops().last());
    QCOMPARE(VirtualDesktopManager::self()->current(), 2u);

    // now create a window on this desktop
    QScopedPointer<Surface> surface(Test::createSurface());
    QFETCH(Test::XdgShellSurfaceType, type);
    QScopedPointer<XdgShellSurface> shellSurface(Test::createXdgShellSurface(type, surface.data()));
    auto client = Test::renderAndWaitForShown(surface.data(), QSize(100, 50), Qt::blue);

    QVERIFY(client);
    QCOMPARE(client->desktop(), 2);
    QSignalSpy desktopPresenceChangedSpy(client, &XdgShellClient::desktopPresenceChanged);
    QVERIFY(desktopPresenceChangedSpy.isValid());

    QCOMPARE(client->desktops().count(), 1u);
    QCOMPARE(VirtualDesktopManager::self()->currentDesktop(), client->desktops().first());

    // and remove last desktop
    VirtualDesktopManager::self()->setCount(1);
    QCOMPARE(VirtualDesktopManager::self()->count(), 1u);
    // now the client should be moved as well
    QTRY_COMPARE(desktopPresenceChangedSpy.count(), 1);
    QCOMPARE(client->desktop(), 1);

    QCOMPARE(client->desktops().count(), 1u);
    QCOMPARE(VirtualDesktopManager::self()->currentDesktop(), client->desktops().first());
}

void VirtualDesktopTest::testWindowOnMultipleDesktops_data()
{
    QTest::addColumn<Test::XdgShellSurfaceType>("type");

    QTest::newRow("xdgShellV6") << Test::XdgShellSurfaceType::XdgShellV6;
    QTest::newRow("xdgWmBase") << Test::XdgShellSurfaceType::XdgShellStable;
}

void VirtualDesktopTest::testWindowOnMultipleDesktops()
{
    // first create two new desktops
    QCOMPARE(VirtualDesktopManager::self()->count(), 1u);
    VirtualDesktopManager::self()->setCount(3);
    QCOMPARE(VirtualDesktopManager::self()->count(), 3u);

    // switch to last desktop
    VirtualDesktopManager::self()->setCurrent(VirtualDesktopManager::self()->desktops().last());
    QCOMPARE(VirtualDesktopManager::self()->current(), 3u);

    // now create a window on this desktop
    QScopedPointer<Surface> surface(Test::createSurface());
    QFETCH(Test::XdgShellSurfaceType, type);
    QScopedPointer<XdgShellSurface> shellSurface(Test::createXdgShellSurface(type, surface.data()));
    auto client = Test::renderAndWaitForShown(surface.data(), QSize(100, 50), Qt::blue);

    QVERIFY(client);
    QCOMPARE(client->desktop(), 3u);
    QSignalSpy desktopPresenceChangedSpy(client, &XdgShellClient::desktopPresenceChanged);
    QVERIFY(desktopPresenceChangedSpy.isValid());

    QCOMPARE(client->desktops().count(), 1u);
    QCOMPARE(VirtualDesktopManager::self()->currentDesktop(), client->desktops().first());

    //Set the window on desktop 2 as well
    client->enterDesktop(VirtualDesktopManager::self()->desktopForX11Id(2));
    QCOMPARE(client->desktops().count(), 2u);
    QCOMPARE(VirtualDesktopManager::self()->desktops()[2], client->desktops()[0]);
    QCOMPARE(VirtualDesktopManager::self()->desktops()[1], client->desktops()[1]);
    QVERIFY(client->isOnDesktop(2));
    QVERIFY(client->isOnDesktop(3));

    //leave desktop 3
    client->leaveDesktop(VirtualDesktopManager::self()->desktopForX11Id(3));
    QCOMPARE(client->desktops().count(), 1u);
    //leave desktop 2
    client->leaveDesktop(VirtualDesktopManager::self()->desktopForX11Id(2));
    QCOMPARE(client->desktops().count(), 0u);
    //we should be on all desktops now
    QVERIFY(client->isOnAllDesktops());
    //put on desktop 1
    client->enterDesktop(VirtualDesktopManager::self()->desktopForX11Id(1));
    QVERIFY(client->isOnDesktop(1));
    QVERIFY(!client->isOnDesktop(2));
    QVERIFY(!client->isOnDesktop(3));
    QCOMPARE(client->desktops().count(), 1u);
    //put on desktop 2
    client->enterDesktop(VirtualDesktopManager::self()->desktopForX11Id(2));
    QVERIFY(client->isOnDesktop(1));
    QVERIFY(client->isOnDesktop(2));
    QVERIFY(!client->isOnDesktop(3));
    QCOMPARE(client->desktops().count(), 2u);
    //put on desktop 3
    client->enterDesktop(VirtualDesktopManager::self()->desktopForX11Id(3));
    QVERIFY(client->isOnDesktop(1));
    QVERIFY(client->isOnDesktop(2));
    QVERIFY(client->isOnDesktop(3));
    QCOMPARE(client->desktops().count(), 3u);

    //entering twice dooes nothing
    client->enterDesktop(VirtualDesktopManager::self()->desktopForX11Id(3));
    QCOMPARE(client->desktops().count(), 3u);

    //adding to "all desktops" results in just that one desktop
    client->setOnAllDesktops(true);
    QCOMPARE(client->desktops().count(), 0u);
    client->enterDesktop(VirtualDesktopManager::self()->desktopForX11Id(3));
    QVERIFY(client->isOnDesktop(3));
    QCOMPARE(client->desktops().count(), 1u);

    //leaving a desktop on "all desktops" puts on everything else
    client->setOnAllDesktops(true);
    QCOMPARE(client->desktops().count(), 0u);
    client->leaveDesktop(VirtualDesktopManager::self()->desktopForX11Id(3));
    QVERIFY(client->isOnDesktop(1));
    QVERIFY(client->isOnDesktop(2));
    QCOMPARE(client->desktops().count(), 2u);
}

void VirtualDesktopTest::testRemoveDesktopWithWindow_data()
{
    QTest::addColumn<Test::XdgShellSurfaceType>("type");

    QTest::newRow("xdgShellV6") << Test::XdgShellSurfaceType::XdgShellV6;
    QTest::newRow("xdgWmBase") << Test::XdgShellSurfaceType::XdgShellStable;
}

void VirtualDesktopTest::testRemoveDesktopWithWindow()
{
    // first create two new desktops
    QCOMPARE(VirtualDesktopManager::self()->count(), 1u);
    VirtualDesktopManager::self()->setCount(3);
    QCOMPARE(VirtualDesktopManager::self()->count(), 3u);

    // switch to last desktop
    VirtualDesktopManager::self()->setCurrent(VirtualDesktopManager::self()->desktops().last());
    QCOMPARE(VirtualDesktopManager::self()->current(), 3u);

    // now create a window on this desktop
    QScopedPointer<Surface> surface(Test::createSurface());
    QFETCH(Test::XdgShellSurfaceType, type);
    QScopedPointer<XdgShellSurface> shellSurface(Test::createXdgShellSurface(type, surface.data()));
    auto client = Test::renderAndWaitForShown(surface.data(), QSize(100, 50), Qt::blue);

    QVERIFY(client);
    QCOMPARE(client->desktop(), 3u);
    QSignalSpy desktopPresenceChangedSpy(client, &XdgShellClient::desktopPresenceChanged);
    QVERIFY(desktopPresenceChangedSpy.isValid());

    QCOMPARE(client->desktops().count(), 1u);
    QCOMPARE(VirtualDesktopManager::self()->currentDesktop(), client->desktops().first());

    //Set the window on desktop 2 as well
    client->enterDesktop(VirtualDesktopManager::self()->desktops()[1]);
    QCOMPARE(client->desktops().count(), 2u);
    QCOMPARE(VirtualDesktopManager::self()->desktops()[2], client->desktops()[0]);
    QCOMPARE(VirtualDesktopManager::self()->desktops()[1], client->desktops()[1]);
    QVERIFY(client->isOnDesktop(2));
    QVERIFY(client->isOnDesktop(3));

    //remove desktop 3
    VirtualDesktopManager::self()->setCount(2);
    QCOMPARE(client->desktops().count(), 1u);
    //window is only on desktop 2
    QCOMPARE(VirtualDesktopManager::self()->desktops()[1], client->desktops()[0]);

    //Again 3 desktops
    VirtualDesktopManager::self()->setCount(3);
    //move window to be only on desktop 3
    client->enterDesktop(VirtualDesktopManager::self()->desktops()[2]);
    client->leaveDesktop(VirtualDesktopManager::self()->desktops()[1]);
    QCOMPARE(client->desktops().count(), 1u);
    //window is only on desktop 3
    QCOMPARE(VirtualDesktopManager::self()->desktops()[2], client->desktops()[0]);

    //remove desktop 3
    VirtualDesktopManager::self()->setCount(2);
    QCOMPARE(client->desktops().count(), 1u);
    //window is only on desktop 2
    QCOMPARE(VirtualDesktopManager::self()->desktops()[1], client->desktops()[0]);
}

WAYLANDTEST_MAIN(VirtualDesktopTest)
#include "virtual_desktop_test.moc"
