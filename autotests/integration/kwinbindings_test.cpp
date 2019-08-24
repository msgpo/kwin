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
#include "input.h"
#include "platform.h"
#include "screens.h"
#include "shell_client.h"
#include "scripting/scripting.h"
#include "useractions.h"
#include "virtualdesktops.h"
#include "wayland_server.h"
#include "workspace.h"

#include <KWayland/Client/shell.h>
#include <KWayland/Client/surface.h>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingReply>

using namespace KWin;
using namespace KWayland::Client;

static const QString s_socketName = QStringLiteral("wayland_test_kwin_kwinbindings-0");

class KWinBindingsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void testSwitchWindow();
    void testSwitchWindowScript();
    void testWindowToDesktop_data();
    void testWindowToDesktop();
};


void KWinBindingsTest::initTestCase()
{
    qRegisterMetaType<KWin::ShellClient*>();
    qRegisterMetaType<KWin::AbstractClient*>();
    QSignalSpy workspaceCreatedSpy(kwinApp(), &Application::workspaceCreated);
    QVERIFY(workspaceCreatedSpy.isValid());
    kwinApp()->platform()->setInitialWindowSize(QSize(1280, 1024));
    QVERIFY(waylandServer()->init(s_socketName.toLocal8Bit()));

    kwinApp()->setConfig(KSharedConfig::openConfig(QString(), KConfig::SimpleConfig));

    kwinApp()->start();
    QVERIFY(workspaceCreatedSpy.wait());
    waylandServer()->initWorkspace();
}

void KWinBindingsTest::init()
{
    QVERIFY(Test::setupWaylandConnection());
    screens()->setCurrent(0);
    KWin::Cursor::setPos(QPoint(640, 512));
}

void KWinBindingsTest::cleanup()
{
    Test::destroyWaylandConnection();
}

void KWinBindingsTest::testSwitchWindow()
{
    // first create windows
    QScopedPointer<Surface> surface1(Test::createSurface());
    QScopedPointer<ShellSurface> shellSurface1(Test::createShellSurface(surface1.data()));
    auto c1 = Test::renderAndWaitForShown(surface1.data(), QSize(100, 50), Qt::blue);
    QScopedPointer<Surface> surface2(Test::createSurface());
    QScopedPointer<ShellSurface> shellSurface2(Test::createShellSurface(surface2.data()));
    auto c2 = Test::renderAndWaitForShown(surface2.data(), QSize(100, 50), Qt::blue);
    QScopedPointer<Surface> surface3(Test::createSurface());
    QScopedPointer<ShellSurface> shellSurface3(Test::createShellSurface(surface3.data()));
    auto c3 = Test::renderAndWaitForShown(surface3.data(), QSize(100, 50), Qt::blue);
    QScopedPointer<Surface> surface4(Test::createSurface());
    QScopedPointer<ShellSurface> shellSurface4(Test::createShellSurface(surface4.data()));
    auto c4 = Test::renderAndWaitForShown(surface4.data(), QSize(100, 50), Qt::blue);

    QVERIFY(c4->isActive());
    QVERIFY(c4 != c3);
    QVERIFY(c3 != c2);
    QVERIFY(c2 != c1);

    // let's position all windows
    c1->move(QPoint(0, 0));
    c2->move(QPoint(200, 0));
    c3->move(QPoint(200, 200));
    c4->move(QPoint(0, 200));

    // now let's trigger the shortcuts

    // invoke global shortcut through dbus
    auto invokeShortcut = [] (const QString &shortcut) {
        auto msg = QDBusMessage::createMethodCall(
            QStringLiteral("org.kde.kglobalaccel"),
            QStringLiteral("/component/kwin"),
            QStringLiteral("org.kde.kglobalaccel.Component"),
            QStringLiteral("invokeShortcut"));
        msg.setArguments(QList<QVariant>{shortcut});
        QDBusConnection::sessionBus().asyncCall(msg);
    };
    invokeShortcut(QStringLiteral("Switch Window Up"));
    QTRY_COMPARE(workspace()->activeClient(), c1);
    invokeShortcut(QStringLiteral("Switch Window Right"));
    QTRY_COMPARE(workspace()->activeClient(), c2);
    invokeShortcut(QStringLiteral("Switch Window Down"));
    QTRY_COMPARE(workspace()->activeClient(), c3);
    invokeShortcut(QStringLiteral("Switch Window Left"));
    QTRY_COMPARE(workspace()->activeClient(), c4);
    // test opposite direction
    invokeShortcut(QStringLiteral("Switch Window Left"));
    QTRY_COMPARE(workspace()->activeClient(), c3);
    invokeShortcut(QStringLiteral("Switch Window Down"));
    QTRY_COMPARE(workspace()->activeClient(), c2);
    invokeShortcut(QStringLiteral("Switch Window Right"));
    QTRY_COMPARE(workspace()->activeClient(), c1);
    invokeShortcut(QStringLiteral("Switch Window Up"));
    QTRY_COMPARE(workspace()->activeClient(), c4);
}

void KWinBindingsTest::testSwitchWindowScript()
{
    QVERIFY(Scripting::self());

    // first create windows
    QScopedPointer<Surface> surface1(Test::createSurface());
    QScopedPointer<ShellSurface> shellSurface1(Test::createShellSurface(surface1.data()));
    auto c1 = Test::renderAndWaitForShown(surface1.data(), QSize(100, 50), Qt::blue);
    QScopedPointer<Surface> surface2(Test::createSurface());
    QScopedPointer<ShellSurface> shellSurface2(Test::createShellSurface(surface2.data()));
    auto c2 = Test::renderAndWaitForShown(surface2.data(), QSize(100, 50), Qt::blue);
    QScopedPointer<Surface> surface3(Test::createSurface());
    QScopedPointer<ShellSurface> shellSurface3(Test::createShellSurface(surface3.data()));
    auto c3 = Test::renderAndWaitForShown(surface3.data(), QSize(100, 50), Qt::blue);
    QScopedPointer<Surface> surface4(Test::createSurface());
    QScopedPointer<ShellSurface> shellSurface4(Test::createShellSurface(surface4.data()));
    auto c4 = Test::renderAndWaitForShown(surface4.data(), QSize(100, 50), Qt::blue);

    QVERIFY(c4->isActive());
    QVERIFY(c4 != c3);
    QVERIFY(c3 != c2);
    QVERIFY(c2 != c1);

    // let's position all windows
    c1->move(QPoint(0, 0));
    c2->move(QPoint(200, 0));
    c3->move(QPoint(200, 200));
    c4->move(QPoint(0, 200));

    auto runScript = [] (const QString &slot) {
        QTemporaryFile tmpFile;
        QVERIFY(tmpFile.open());
        QTextStream out(&tmpFile);
        out << "workspace." << slot << "()";
        out.flush();

        const int id = Scripting::self()->loadScript(tmpFile.fileName());
        QVERIFY(id != -1);
        QVERIFY(Scripting::self()->isScriptLoaded(tmpFile.fileName()));
        auto s = Scripting::self()->findScript(tmpFile.fileName());
        QVERIFY(s);
        QSignalSpy runningChangedSpy(s, &AbstractScript::runningChanged);
        QVERIFY(runningChangedSpy.isValid());
        s->run();
        QTRY_COMPARE(runningChangedSpy.count(), 1);
    };

    runScript(QStringLiteral("slotSwitchWindowUp"));
    QTRY_COMPARE(workspace()->activeClient(), c1);
    runScript(QStringLiteral("slotSwitchWindowRight"));
    QTRY_COMPARE(workspace()->activeClient(), c2);
    runScript(QStringLiteral("slotSwitchWindowDown"));
    QTRY_COMPARE(workspace()->activeClient(), c3);
    runScript(QStringLiteral("slotSwitchWindowLeft"));
    QTRY_COMPARE(workspace()->activeClient(), c4);
}

void KWinBindingsTest::testWindowToDesktop_data()
{
    QTest::addColumn<int>("desktop");

    QTest::newRow("2") << 2;
    QTest::newRow("3") << 3;
    QTest::newRow("4") << 4;
    QTest::newRow("5") << 5;
    QTest::newRow("6") << 6;
    QTest::newRow("7") << 7;
    QTest::newRow("8") << 8;
    QTest::newRow("9") << 9;
    QTest::newRow("10") << 10;
    QTest::newRow("11") << 11;
    QTest::newRow("12") << 12;
    QTest::newRow("13") << 13;
    QTest::newRow("14") << 14;
    QTest::newRow("15") << 15;
    QTest::newRow("16") << 16;
    QTest::newRow("17") << 17;
    QTest::newRow("18") << 18;
    QTest::newRow("19") << 19;
    QTest::newRow("20") << 20;
}

void KWinBindingsTest::testWindowToDesktop()
{
    // first go to desktop one
    VirtualDesktopManager::self()->setCurrent(VirtualDesktopManager::self()->desktops().first());

    // now create a window
    QScopedPointer<Surface> surface(Test::createSurface());
    QScopedPointer<ShellSurface> shellSurface(Test::createShellSurface(surface.data()));
    auto c = Test::renderAndWaitForShown(surface.data(), QSize(100, 50), Qt::blue);
    QSignalSpy desktopChangedSpy(c, &AbstractClient::desktopChanged);
    QVERIFY(desktopChangedSpy.isValid());
    QCOMPARE(workspace()->activeClient(), c);

    QFETCH(int, desktop);
    VirtualDesktopManager::self()->setCount(desktop);

    // now trigger the shortcut
    auto invokeShortcut = [] (int desktop) {
        auto msg = QDBusMessage::createMethodCall(
            QStringLiteral("org.kde.kglobalaccel"),
            QStringLiteral("/component/kwin"),
            QStringLiteral("org.kde.kglobalaccel.Component"),
            QStringLiteral("invokeShortcut"));
        msg.setArguments(QList<QVariant>{QStringLiteral("Window to Desktop %1").arg(desktop)});
        QDBusConnection::sessionBus().asyncCall(msg);
    };
    invokeShortcut(desktop);
    QVERIFY(desktopChangedSpy.wait());
    QCOMPARE(c->desktop(), desktop);
    // back to desktop 1
    invokeShortcut(1);
    QVERIFY(desktopChangedSpy.wait());
    QCOMPARE(c->desktop(), 1);
    // invoke with one desktop too many
    invokeShortcut(desktop + 1);
    // that should fail
    QVERIFY(!desktopChangedSpy.wait(100));
}

WAYLANDTEST_MAIN(KWinBindingsTest)
#include "kwinbindings_test.moc"
