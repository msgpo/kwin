/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2019 Vlad Zagorodniy <vladzzag@gmail.com>

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
#include "platform.h"
#include "screens.h"
#include "shell_client.h"
#include "wayland_server.h"
#include "workspace.h"

#include <KWayland/Client/surface.h>

namespace KWin
{

static const QString s_socketName = QStringLiteral("wayland_test_activation-0");

class ActivationTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void testSwitchToWindowToLeft();
    void testSwitchToWindowToRight();
    void testSwitchToWindowAbove();
    void testSwitchToWindowBelow();
    void testSwitchToWindowMaximized();
    void testSwitchToWindowFullScreen();

private:
    void stackScreensHorizontally();
    void stackScreensVertically();
};

void ActivationTest::initTestCase()
{
    qRegisterMetaType<AbstractClient *>();
    qRegisterMetaType<ShellClient *>();

    QSignalSpy workspaceCreatedSpy(kwinApp(), &Application::workspaceCreated);
    QVERIFY(workspaceCreatedSpy.isValid());
    kwinApp()->platform()->setInitialWindowSize(QSize(1280, 1024));
    QVERIFY(waylandServer()->init(s_socketName.toLocal8Bit()));
    QMetaObject::invokeMethod(kwinApp()->platform(), "setVirtualOutputs", Qt::DirectConnection, Q_ARG(int, 2));

    kwinApp()->start();
    QVERIFY(workspaceCreatedSpy.wait());
    QCOMPARE(screens()->count(), 2);
    QCOMPARE(screens()->geometry(0), QRect(0, 0, 1280, 1024));
    QCOMPARE(screens()->geometry(1), QRect(1280, 0, 1280, 1024));
    waylandServer()->initWorkspace();
}

void ActivationTest::init()
{
    QVERIFY(Test::setupWaylandConnection());

    screens()->setCurrent(0);
    Cursor::setPos(QPoint(640, 512));
}

void ActivationTest::cleanup()
{
    Test::destroyWaylandConnection();

    stackScreensHorizontally();
}

void ActivationTest::testSwitchToWindowToLeft()
{
    // This test verifies that "Switch to Window to the Left" shortcut works.

    using namespace KWayland::Client;

    // Prepare the test environment.
    stackScreensHorizontally();

    // Create several clients on the left screen.
    QScopedPointer<Surface> surface1(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface1(Test::createXdgShellStableSurface(surface1.data()));
    ShellClient *client1 = Test::renderAndWaitForShown(surface1.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client1);
    QVERIFY(client1->isActive());

    QScopedPointer<Surface> surface2(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface2(Test::createXdgShellStableSurface(surface2.data()));
    ShellClient *client2 = Test::renderAndWaitForShown(surface2.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client2);
    QVERIFY(client2->isActive());

    client1->move(QPoint(300, 200));
    client2->move(QPoint(500, 200));

    // Create several clients on the right screen.
    QScopedPointer<Surface> surface3(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface3(Test::createXdgShellStableSurface(surface3.data()));
    ShellClient *client3 = Test::renderAndWaitForShown(surface3.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client3);
    QVERIFY(client3->isActive());

    QScopedPointer<Surface> surface4(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface4(Test::createXdgShellStableSurface(surface4.data()));
    ShellClient *client4 = Test::renderAndWaitForShown(surface4.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client4);
    QVERIFY(client4->isActive());

    client3->move(QPoint(1380, 200));
    client4->move(QPoint(1580, 200));

    // Switch to window to the left.
    workspace()->switchWindow(Workspace::DirectionWest);
    QVERIFY(client3->isActive());

    // Switch to window to the left.
    workspace()->switchWindow(Workspace::DirectionWest);
    QVERIFY(client2->isActive());

    // Switch to window to the left.
    workspace()->switchWindow(Workspace::DirectionWest);
    QVERIFY(client1->isActive());

    // Switch to window to the left.
    workspace()->switchWindow(Workspace::DirectionWest);
    QVERIFY(client4->isActive());

    // Destroy all clients.
    shellSurface1.reset();
    QVERIFY(Test::waitForWindowDestroyed(client1));
    shellSurface2.reset();
    QVERIFY(Test::waitForWindowDestroyed(client2));
    shellSurface3.reset();
    QVERIFY(Test::waitForWindowDestroyed(client3));
    shellSurface4.reset();
    QVERIFY(Test::waitForWindowDestroyed(client4));
}

void ActivationTest::testSwitchToWindowToRight()
{
    // This test verifies that "Switch to Window to the Right" shortcut works.

    using namespace KWayland::Client;

    // Prepare the test environment.
    stackScreensHorizontally();

    // Create several clients on the left screen.
    QScopedPointer<Surface> surface1(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface1(Test::createXdgShellStableSurface(surface1.data()));
    ShellClient *client1 = Test::renderAndWaitForShown(surface1.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client1);
    QVERIFY(client1->isActive());

    QScopedPointer<Surface> surface2(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface2(Test::createXdgShellStableSurface(surface2.data()));
    ShellClient *client2 = Test::renderAndWaitForShown(surface2.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client2);
    QVERIFY(client2->isActive());

    client1->move(QPoint(300, 200));
    client2->move(QPoint(500, 200));

    // Create several clients on the right screen.
    QScopedPointer<Surface> surface3(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface3(Test::createXdgShellStableSurface(surface3.data()));
    ShellClient *client3 = Test::renderAndWaitForShown(surface3.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client3);
    QVERIFY(client3->isActive());

    QScopedPointer<Surface> surface4(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface4(Test::createXdgShellStableSurface(surface4.data()));
    ShellClient *client4 = Test::renderAndWaitForShown(surface4.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client4);
    QVERIFY(client4->isActive());

    client3->move(QPoint(1380, 200));
    client4->move(QPoint(1580, 200));

    // Switch to window to the right.
    workspace()->switchWindow(Workspace::DirectionEast);
    QVERIFY(client1->isActive());

    // Switch to window to the right.
    workspace()->switchWindow(Workspace::DirectionEast);
    QVERIFY(client2->isActive());

    // Switch to window to the right.
    workspace()->switchWindow(Workspace::DirectionEast);
    QVERIFY(client3->isActive());

    // Switch to window to the right.
    workspace()->switchWindow(Workspace::DirectionEast);
    QVERIFY(client4->isActive());

    // Destroy all clients.
    shellSurface1.reset();
    QVERIFY(Test::waitForWindowDestroyed(client1));
    shellSurface2.reset();
    QVERIFY(Test::waitForWindowDestroyed(client2));
    shellSurface3.reset();
    QVERIFY(Test::waitForWindowDestroyed(client3));
    shellSurface4.reset();
    QVERIFY(Test::waitForWindowDestroyed(client4));
}

void ActivationTest::testSwitchToWindowAbove()
{
    // This test verifies that "Switch to Window Above" shortcut works.

    using namespace KWayland::Client;

    // Prepare the test environment.
    stackScreensVertically();

    // Create several clients on the top screen.
    QScopedPointer<Surface> surface1(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface1(Test::createXdgShellStableSurface(surface1.data()));
    ShellClient *client1 = Test::renderAndWaitForShown(surface1.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client1);
    QVERIFY(client1->isActive());

    QScopedPointer<Surface> surface2(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface2(Test::createXdgShellStableSurface(surface2.data()));
    ShellClient *client2 = Test::renderAndWaitForShown(surface2.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client2);
    QVERIFY(client2->isActive());

    client1->move(QPoint(200, 300));
    client2->move(QPoint(200, 500));

    // Create several clients on the bottom screen.
    QScopedPointer<Surface> surface3(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface3(Test::createXdgShellStableSurface(surface3.data()));
    ShellClient *client3 = Test::renderAndWaitForShown(surface3.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client3);
    QVERIFY(client3->isActive());

    QScopedPointer<Surface> surface4(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface4(Test::createXdgShellStableSurface(surface4.data()));
    ShellClient *client4 = Test::renderAndWaitForShown(surface4.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client4);
    QVERIFY(client4->isActive());

    client3->move(QPoint(200, 1224));
    client4->move(QPoint(200, 1424));

    // Switch to window above.
    workspace()->switchWindow(Workspace::DirectionNorth);
    QVERIFY(client3->isActive());

    // Switch to window above.
    workspace()->switchWindow(Workspace::DirectionNorth);
    QVERIFY(client2->isActive());

    // Switch to window above.
    workspace()->switchWindow(Workspace::DirectionNorth);
    QVERIFY(client1->isActive());

    // Switch to window above.
    workspace()->switchWindow(Workspace::DirectionNorth);
    QVERIFY(client4->isActive());

    // Destroy all clients.
    shellSurface1.reset();
    QVERIFY(Test::waitForWindowDestroyed(client1));
    shellSurface2.reset();
    QVERIFY(Test::waitForWindowDestroyed(client2));
    shellSurface3.reset();
    QVERIFY(Test::waitForWindowDestroyed(client3));
    shellSurface4.reset();
    QVERIFY(Test::waitForWindowDestroyed(client4));
}

void ActivationTest::testSwitchToWindowBelow()
{
    // This test verifies that "Switch to Window Bottom" shortcut works.

    using namespace KWayland::Client;

    // Prepare the test environment.
    stackScreensVertically();

    // Create several clients on the top screen.
    QScopedPointer<Surface> surface1(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface1(Test::createXdgShellStableSurface(surface1.data()));
    ShellClient *client1 = Test::renderAndWaitForShown(surface1.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client1);
    QVERIFY(client1->isActive());

    QScopedPointer<Surface> surface2(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface2(Test::createXdgShellStableSurface(surface2.data()));
    ShellClient *client2 = Test::renderAndWaitForShown(surface2.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client2);
    QVERIFY(client2->isActive());

    client1->move(QPoint(200, 300));
    client2->move(QPoint(200, 500));

    // Create several clients on the bottom screen.
    QScopedPointer<Surface> surface3(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface3(Test::createXdgShellStableSurface(surface3.data()));
    ShellClient *client3 = Test::renderAndWaitForShown(surface3.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client3);
    QVERIFY(client3->isActive());

    QScopedPointer<Surface> surface4(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface4(Test::createXdgShellStableSurface(surface4.data()));
    ShellClient *client4 = Test::renderAndWaitForShown(surface4.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client4);
    QVERIFY(client4->isActive());

    client3->move(QPoint(200, 1224));
    client4->move(QPoint(200, 1424));

    // Switch to window below.
    workspace()->switchWindow(Workspace::DirectionSouth);
    QVERIFY(client1->isActive());

    // Switch to window below.
    workspace()->switchWindow(Workspace::DirectionSouth);
    QVERIFY(client2->isActive());

    // Switch to window below.
    workspace()->switchWindow(Workspace::DirectionSouth);
    QVERIFY(client3->isActive());

    // Switch to window below.
    workspace()->switchWindow(Workspace::DirectionSouth);
    QVERIFY(client4->isActive());

    // Destroy all clients.
    shellSurface1.reset();
    QVERIFY(Test::waitForWindowDestroyed(client1));
    shellSurface2.reset();
    QVERIFY(Test::waitForWindowDestroyed(client2));
    shellSurface3.reset();
    QVERIFY(Test::waitForWindowDestroyed(client3));
    shellSurface4.reset();
    QVERIFY(Test::waitForWindowDestroyed(client4));
}

void ActivationTest::testSwitchToWindowMaximized()
{
    // This test verifies that we switch to the top-most maximized client, i.e.
    // the one that user sees at the moment. See bug 411356.

    using namespace KWayland::Client;

    // Prepare the test environment.
    stackScreensHorizontally();

    // Create several maximized clients on the left screen.
    QScopedPointer<Surface> surface1(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface1(Test::createXdgShellStableSurface(surface1.data()));
    ShellClient *client1 = Test::renderAndWaitForShown(surface1.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client1);
    QVERIFY(client1->isActive());
    QSignalSpy configureRequestedSpy1(shellSurface1.data(), &XdgShellSurface::configureRequested);
    QVERIFY(configureRequestedSpy1.wait());
    workspace()->slotWindowMaximize();
    QVERIFY(configureRequestedSpy1.wait());
    QSignalSpy geometryChangedSpy1(client1, &ShellClient::geometryChanged);
    QVERIFY(geometryChangedSpy1.isValid());
    shellSurface1->ackConfigure(configureRequestedSpy1.last().at(2).value<quint32>());
    Test::render(surface1.data(), configureRequestedSpy1.last().at(0).toSize(), Qt::red);
    QVERIFY(geometryChangedSpy1.wait());

    QScopedPointer<Surface> surface2(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface2(Test::createXdgShellStableSurface(surface2.data()));
    ShellClient *client2 = Test::renderAndWaitForShown(surface2.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client2);
    QVERIFY(client2->isActive());
    QSignalSpy configureRequestedSpy2(shellSurface2.data(), &XdgShellSurface::configureRequested);
    QVERIFY(configureRequestedSpy2.wait());
    workspace()->slotWindowMaximize();
    QVERIFY(configureRequestedSpy2.wait());
    QSignalSpy geometryChangedSpy2(client2, &ShellClient::geometryChanged);
    QVERIFY(geometryChangedSpy2.isValid());
    shellSurface2->ackConfigure(configureRequestedSpy2.last().at(2).value<quint32>());
    Test::render(surface2.data(), configureRequestedSpy2.last().at(0).toSize(), Qt::red);
    QVERIFY(geometryChangedSpy2.wait());

    const ToplevelList stackingOrder = workspace()->stackingOrder();
    QVERIFY(stackingOrder.indexOf(client1) < stackingOrder.indexOf(client2));
    QCOMPARE(client1->maximizeMode(), MaximizeFull);
    QCOMPARE(client2->maximizeMode(), MaximizeFull);

    // Create several clients on the right screen.
    QScopedPointer<Surface> surface3(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface3(Test::createXdgShellStableSurface(surface3.data()));
    ShellClient *client3 = Test::renderAndWaitForShown(surface3.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client3);
    QVERIFY(client3->isActive());

    QScopedPointer<Surface> surface4(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface4(Test::createXdgShellStableSurface(surface4.data()));
    ShellClient *client4 = Test::renderAndWaitForShown(surface4.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client4);
    QVERIFY(client4->isActive());

    client3->move(QPoint(1380, 200));
    client4->move(QPoint(1580, 200));

    // Switch to window to the left.
    workspace()->switchWindow(Workspace::DirectionWest);
    QVERIFY(client3->isActive());

    // Switch to window to the left.
    workspace()->switchWindow(Workspace::DirectionWest);
    QVERIFY(client2->isActive());

    // Switch to window to the left.
    workspace()->switchWindow(Workspace::DirectionWest);
    QVERIFY(client4->isActive());

    // Destroy all clients.
    shellSurface1.reset();
    QVERIFY(Test::waitForWindowDestroyed(client1));
    shellSurface2.reset();
    QVERIFY(Test::waitForWindowDestroyed(client2));
    shellSurface3.reset();
    QVERIFY(Test::waitForWindowDestroyed(client3));
    shellSurface4.reset();
    QVERIFY(Test::waitForWindowDestroyed(client4));
}

void ActivationTest::testSwitchToWindowFullScreen()
{
    // This test verifies that we switch to the top-most fullscreen client, i.e.
    // the one that user sees at the moment. See bug 411356.

    using namespace KWayland::Client;

    // Prepare the test environment.
    stackScreensVertically();

    // Create several maximized clients on the top screen.
    QScopedPointer<Surface> surface1(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface1(Test::createXdgShellStableSurface(surface1.data()));
    ShellClient *client1 = Test::renderAndWaitForShown(surface1.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client1);
    QVERIFY(client1->isActive());
    QSignalSpy configureRequestedSpy1(shellSurface1.data(), &XdgShellSurface::configureRequested);
    QVERIFY(configureRequestedSpy1.wait());
    workspace()->slotWindowFullScreen();
    QVERIFY(configureRequestedSpy1.wait());
    QSignalSpy geometryChangedSpy1(client1, &ShellClient::geometryChanged);
    QVERIFY(geometryChangedSpy1.isValid());
    shellSurface1->ackConfigure(configureRequestedSpy1.last().at(2).value<quint32>());
    Test::render(surface1.data(), configureRequestedSpy1.last().at(0).toSize(), Qt::red);
    QVERIFY(geometryChangedSpy1.wait());

    QScopedPointer<Surface> surface2(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface2(Test::createXdgShellStableSurface(surface2.data()));
    ShellClient *client2 = Test::renderAndWaitForShown(surface2.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client2);
    QVERIFY(client2->isActive());
    QSignalSpy configureRequestedSpy2(shellSurface2.data(), &XdgShellSurface::configureRequested);
    QVERIFY(configureRequestedSpy2.wait());
    workspace()->slotWindowFullScreen();
    QVERIFY(configureRequestedSpy2.wait());
    QSignalSpy geometryChangedSpy2(client2, &ShellClient::geometryChanged);
    QVERIFY(geometryChangedSpy2.isValid());
    shellSurface2->ackConfigure(configureRequestedSpy2.last().at(2).value<quint32>());
    Test::render(surface2.data(), configureRequestedSpy2.last().at(0).toSize(), Qt::red);
    QVERIFY(geometryChangedSpy2.wait());

    const ToplevelList stackingOrder = workspace()->stackingOrder();
    QVERIFY(stackingOrder.indexOf(client1) < stackingOrder.indexOf(client2));
    QVERIFY(client1->isFullScreen());
    QVERIFY(client2->isFullScreen());

    // Create several clients on the bottom screen.
    QScopedPointer<Surface> surface3(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface3(Test::createXdgShellStableSurface(surface3.data()));
    ShellClient *client3 = Test::renderAndWaitForShown(surface3.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client3);
    QVERIFY(client3->isActive());

    QScopedPointer<Surface> surface4(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface4(Test::createXdgShellStableSurface(surface4.data()));
    ShellClient *client4 = Test::renderAndWaitForShown(surface4.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client4);
    QVERIFY(client4->isActive());

    client3->move(QPoint(200, 1224));
    client4->move(QPoint(200, 1424));

    // Switch to window above.
    workspace()->switchWindow(Workspace::DirectionNorth);
    QVERIFY(client3->isActive());

    // Switch to window above.
    workspace()->switchWindow(Workspace::DirectionNorth);
    QVERIFY(client2->isActive());

    // Switch to window above.
    workspace()->switchWindow(Workspace::DirectionNorth);
    QVERIFY(client4->isActive());

    // Destroy all clients.
    shellSurface1.reset();
    QVERIFY(Test::waitForWindowDestroyed(client1));
    shellSurface2.reset();
    QVERIFY(Test::waitForWindowDestroyed(client2));
    shellSurface3.reset();
    QVERIFY(Test::waitForWindowDestroyed(client3));
    shellSurface4.reset();
    QVERIFY(Test::waitForWindowDestroyed(client4));
}

void ActivationTest::stackScreensHorizontally()
{
    // Process pending wl_output bind requests before destroying all outputs.
    QTest::qWait(1);

    const QVector<QRect> screenGeometries {
        QRect(0, 0, 1280, 1024),
        QRect(1280, 0, 1280, 1024),
    };

    const QVector<int> screenScales {
        1,
        1,
    };

    QMetaObject::invokeMethod(kwinApp()->platform(),
        "setVirtualOutputs",
        Qt::DirectConnection,
        Q_ARG(int, screenGeometries.count()),
        Q_ARG(QVector<QRect>, screenGeometries),
        Q_ARG(QVector<int>, screenScales)
    );
}

void ActivationTest::stackScreensVertically()
{
    // Process pending wl_output bind requests before destroys all outputs.
    QTest::qWait(1);

    const QVector<QRect> screenGeometries {
        QRect(0, 0, 1280, 1024),
        QRect(0, 1024, 1280, 1024),
    };

    const QVector<int> screenScales {
        1,
        1,
    };

    QMetaObject::invokeMethod(kwinApp()->platform(),
        "setVirtualOutputs",
        Qt::DirectConnection,
        Q_ARG(int, screenGeometries.count()),
        Q_ARG(QVector<QRect>, screenGeometries),
        Q_ARG(QVector<int>, screenScales)
    );
}

}

WAYLANDTEST_MAIN(KWin::ActivationTest)
#include "activation_test.moc"
