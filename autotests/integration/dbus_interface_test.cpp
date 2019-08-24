/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2018 Martin Flöser <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "kwin_wayland_test.h"
#include "atoms.h"
#include "client.h"
#include "deleted.h"
#include "platform.h"
#include "rules.h"
#include "screens.h"
#include "shell_client.h"
#include "virtualdesktops.h"
#include "wayland_server.h"
#include "workspace.h"

#include <KWayland/Client/shell.h>
#include <KWayland/Client/surface.h>
#include <KWayland/Client/xdgshell.h>

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingReply>
#include <QUuid>

#include <netwm.h>
#include <xcb/xcb_icccm.h>

using namespace KWin;
using namespace KWayland::Client;

static const QString s_socketName = QStringLiteral("wayland_test_kwin_dbus_interface-0");

const QString s_destination{QStringLiteral("org.kde.KWin")};
const QString s_path{QStringLiteral("/KWin")};
const QString s_interface{QStringLiteral("org.kde.KWin")};

class TestDbusInterface : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void testGetWindowInfoInvalidUuid();
    void testGetWindowInfoShellClient_data();
    void testGetWindowInfoShellClient();
    void testGetWindowInfoX11Client();
};

void TestDbusInterface::initTestCase()
{
    qRegisterMetaType<KWin::Deleted*>();
    qRegisterMetaType<KWin::ShellClient*>();
    qRegisterMetaType<KWin::AbstractClient*>();

    QSignalSpy workspaceCreatedSpy(kwinApp(), &Application::workspaceCreated);
    QVERIFY(workspaceCreatedSpy.isValid());
    kwinApp()->platform()->setInitialWindowSize(QSize(1280, 1024));
    QVERIFY(waylandServer()->init(s_socketName.toLocal8Bit()));

    kwinApp()->start();
    QVERIFY(workspaceCreatedSpy.wait());
    waylandServer()->initWorkspace();
    VirtualDesktopManager::self()->setCount(4);
}

void TestDbusInterface::init()
{
    QVERIFY(Test::setupWaylandConnection());
}

void TestDbusInterface::cleanup()
{
    Test::destroyWaylandConnection();
}

namespace {
QDBusPendingCall getWindowInfo(const QUuid &uuid)
{
    auto msg = QDBusMessage::createMethodCall(s_destination, s_path, s_interface, QStringLiteral("getWindowInfo"));
    msg.setArguments({uuid.toString()});
    return QDBusConnection::sessionBus().asyncCall(msg);
}
}

void TestDbusInterface::testGetWindowInfoInvalidUuid()
{
    QDBusPendingReply<QVariantMap> reply{getWindowInfo(QUuid::createUuid())};
    reply.waitForFinished();
    QVERIFY(reply.isValid());
    QVERIFY(!reply.isError());
    const auto windowData = reply.value();
    QVERIFY(windowData.empty());
}

void TestDbusInterface::testGetWindowInfoShellClient_data()
{
    QTest::addColumn<Test::ShellSurfaceType>("type");

    QTest::newRow("wlShell") << Test::ShellSurfaceType::WlShell;
    QTest::newRow("xdgShellV5") << Test::ShellSurfaceType::XdgShellV5;
    QTest::newRow("xdgShellV6") << Test::ShellSurfaceType::XdgShellV6;
    QTest::newRow("xdgWmBase") << Test::ShellSurfaceType::XdgShellStable;
}

void TestDbusInterface::testGetWindowInfoShellClient()
{
    QSignalSpy clientAddedSpy(waylandServer(), &WaylandServer::shellClientAdded);
    QVERIFY(clientAddedSpy.isValid());

    QScopedPointer<Surface> surface(Test::createSurface());
    QFETCH(Test::ShellSurfaceType, type);
    QScopedPointer<QObject> shellSurface(Test::createShellSurface(type, surface.data()));
    if (type != Test::ShellSurfaceType::WlShell) {
        qobject_cast<XdgShellSurface*>(shellSurface.data())->setAppId(QByteArrayLiteral("org.kde.foo"));
        qobject_cast<XdgShellSurface*>(shellSurface.data())->setTitle(QStringLiteral("Test window"));
    }

    // now let's render
    Test::render(surface.data(), QSize(100, 50), Qt::blue);
    QVERIFY(clientAddedSpy.isEmpty());
    QVERIFY(clientAddedSpy.wait());
    auto client = clientAddedSpy.first().first().value<ShellClient*>();
    QVERIFY(client);

    // let's get the window info
    QDBusPendingReply<QVariantMap> reply{getWindowInfo(client->internalId())};
    reply.waitForFinished();
    QVERIFY(reply.isValid());
    QVERIFY(!reply.isError());
    auto windowData = reply.value();
    QVERIFY(!windowData.isEmpty());
    QCOMPARE(windowData.size(), 24);
    QCOMPARE(windowData.value(QStringLiteral("type")).toInt(), NET::Normal);
    QCOMPARE(windowData.value(QStringLiteral("x")).toInt(), client->x());
    QCOMPARE(windowData.value(QStringLiteral("y")).toInt(), client->y());
    QCOMPARE(windowData.value(QStringLiteral("width")).toInt(), client->width());
    QCOMPARE(windowData.value(QStringLiteral("height")).toInt(), client->height());
    QCOMPARE(windowData.value(QStringLiteral("x11DesktopNumber")).toInt(), 1);
    QCOMPARE(windowData.value(QStringLiteral("minimized")).toBool(), false);
    QCOMPARE(windowData.value(QStringLiteral("shaded")).toBool(), false);
    QCOMPARE(windowData.value(QStringLiteral("fullscreen")).toBool(), false);
    QCOMPARE(windowData.value(QStringLiteral("keepAbove")).toBool(), false);
    QCOMPARE(windowData.value(QStringLiteral("keepBelow")).toBool(), false);
    QCOMPARE(windowData.value(QStringLiteral("skipTaskbar")).toBool(), false);
    QCOMPARE(windowData.value(QStringLiteral("skipPager")).toBool(), false);
    QCOMPARE(windowData.value(QStringLiteral("skipSwitcher")).toBool(), false);
    QCOMPARE(windowData.value(QStringLiteral("maximizeHorizontal")).toBool(), false);
    QCOMPARE(windowData.value(QStringLiteral("maximizeVertical")).toBool(), false);
    QCOMPARE(windowData.value(QStringLiteral("noBorder")).toBool(), true);
    QCOMPARE(windowData.value(QStringLiteral("clientMachine")).toString(), QString());
    QCOMPARE(windowData.value(QStringLiteral("localhost")).toBool(), true);
    QCOMPARE(windowData.value(QStringLiteral("role")).toString(), QString());
    QCOMPARE(windowData.value(QStringLiteral("resourceName")).toString(), QStringLiteral("testDbusInterface"));
    if (type == Test::ShellSurfaceType::WlShell) {
        QCOMPARE(windowData.value(QStringLiteral("resourceClass")).toString(), QString());
        QCOMPARE(windowData.value(QStringLiteral("desktopFile")).toString(), QString());
        QCOMPARE(windowData.value(QStringLiteral("caption")).toString(), QString());
    } else {
        QCOMPARE(windowData.value(QStringLiteral("resourceClass")).toString(), QStringLiteral("org.kde.foo"));
        QCOMPARE(windowData.value(QStringLiteral("desktopFile")).toString(), QStringLiteral("org.kde.foo"));
        QCOMPARE(windowData.value(QStringLiteral("caption")).toString(), QStringLiteral("Test window"));
    }

    auto verifyProperty = [client] (const QString &name) {
        QDBusPendingReply<QVariantMap> reply{getWindowInfo(client->internalId())};
        reply.waitForFinished();
        return reply.value().value(name).toBool();
    };

    QVERIFY(!client->isMinimized());
    client->setMinimized(true);
    QVERIFY(client->isMinimized());
    QCOMPARE(verifyProperty(QStringLiteral("minimized")), true);

    QVERIFY(!client->keepAbove());
    client->setKeepAbove(true);
    QVERIFY(client->keepAbove());
    QCOMPARE(verifyProperty(QStringLiteral("keepAbove")), true);

    QVERIFY(!client->keepBelow());
    client->setKeepBelow(true);
    QVERIFY(client->keepBelow());
    QCOMPARE(verifyProperty(QStringLiteral("keepBelow")), true);

    QVERIFY(!client->skipTaskbar());
    client->setSkipTaskbar(true);
    QVERIFY(client->skipTaskbar());
    QCOMPARE(verifyProperty(QStringLiteral("skipTaskbar")), true);

    QVERIFY(!client->skipPager());
    client->setSkipPager(true);
    QVERIFY(client->skipPager());
    QCOMPARE(verifyProperty(QStringLiteral("skipPager")), true);

    QVERIFY(!client->skipSwitcher());
    client->setSkipSwitcher(true);
    QVERIFY(client->skipSwitcher());
    QCOMPARE(verifyProperty(QStringLiteral("skipSwitcher")), true);

    // not testing shaded as that's X11
    // not testing fullscreen, maximizeHorizontal, maximizeVertical and noBorder as those require window geometry changes

    QCOMPARE(client->desktop(), 1);
    workspace()->sendClientToDesktop(client, 2, false);
    QCOMPARE(client->desktop(), 2);
    reply = getWindowInfo(client->internalId());
    reply.waitForFinished();
    QCOMPARE(reply.value().value(QStringLiteral("x11DesktopNumber")).toInt(), 2);

    client->move(QPoint(10, 20));
    reply = getWindowInfo(client->internalId());
    reply.waitForFinished();
    QCOMPARE(reply.value().value(QStringLiteral("x")).toInt(), client->x());
    QCOMPARE(reply.value().value(QStringLiteral("y")).toInt(), client->y());
    // not testing width, height as that would require window geometry change

    // finally close window
    const auto id = client->internalId();
    QSignalSpy windowClosedSpy(client, &ShellClient::windowClosed);
    QVERIFY(windowClosedSpy.isValid());
    shellSurface.reset();
    surface.reset();
    QVERIFY(windowClosedSpy.wait());
    QCOMPARE(windowClosedSpy.count(), 1);

    reply = getWindowInfo(id);
    reply.waitForFinished();
    QVERIFY(reply.value().empty());
}


struct XcbConnectionDeleter
{
    static inline void cleanup(xcb_connection_t *pointer)
    {
        xcb_disconnect(pointer);
    }
};

void TestDbusInterface::testGetWindowInfoX11Client()
{
    QScopedPointer<xcb_connection_t, XcbConnectionDeleter> c(xcb_connect(nullptr, nullptr));
    QVERIFY(!xcb_connection_has_error(c.data()));
    const QRect windowGeometry(0, 0, 600, 400);
    xcb_window_t w = xcb_generate_id(c.data());
    xcb_create_window(c.data(), XCB_COPY_FROM_PARENT, w, rootWindow(),
                      windowGeometry.x(),
                      windowGeometry.y(),
                      windowGeometry.width(),
                      windowGeometry.height(),
                      0, XCB_WINDOW_CLASS_INPUT_OUTPUT, XCB_COPY_FROM_PARENT, 0, nullptr);
    xcb_size_hints_t hints;
    memset(&hints, 0, sizeof(hints));
    xcb_icccm_size_hints_set_position(&hints, 1, windowGeometry.x(), windowGeometry.y());
    xcb_icccm_size_hints_set_size(&hints, 1, windowGeometry.width(), windowGeometry.height());
    xcb_icccm_set_wm_normal_hints(c.data(), w, &hints);
    xcb_icccm_set_wm_class(c.data(), w, 7, "foo\0bar");
    NETWinInfo winInfo(c.data(), w, rootWindow(), NET::Properties(), NET::Properties2());
    winInfo.setName("Some caption");
    winInfo.setDesktopFileName("org.kde.foo");
    xcb_map_window(c.data(), w);
    xcb_flush(c.data());

    // we should get a client for it
    QSignalSpy windowCreatedSpy(workspace(), &Workspace::clientAdded);
    QVERIFY(windowCreatedSpy.isValid());
    QVERIFY(windowCreatedSpy.wait());
    Client *client = windowCreatedSpy.first().first().value<Client*>();
    QVERIFY(client);
    QCOMPARE(client->window(), w);
    QCOMPARE(client->clientSize(), windowGeometry.size());

    // let's get the window info
    QDBusPendingReply<QVariantMap> reply{getWindowInfo(client->internalId())};
    reply.waitForFinished();
    QVERIFY(reply.isValid());
    QVERIFY(!reply.isError());
    auto windowData = reply.value();
    QVERIFY(!windowData.isEmpty());
    QCOMPARE(windowData.size(), 24);
    QCOMPARE(windowData.value(QStringLiteral("type")).toInt(), NET::Normal);
    QCOMPARE(windowData.value(QStringLiteral("x")).toInt(), client->x());
    QCOMPARE(windowData.value(QStringLiteral("y")).toInt(), client->y());
    QCOMPARE(windowData.value(QStringLiteral("width")).toInt(), client->width());
    QCOMPARE(windowData.value(QStringLiteral("height")).toInt(), client->height());
    QCOMPARE(windowData.value(QStringLiteral("x11DesktopNumber")).toInt(), 1);
    QCOMPARE(windowData.value(QStringLiteral("minimized")).toBool(), false);
    QCOMPARE(windowData.value(QStringLiteral("shaded")).toBool(), false);
    QCOMPARE(windowData.value(QStringLiteral("fullscreen")).toBool(), false);
    QCOMPARE(windowData.value(QStringLiteral("keepAbove")).toBool(), false);
    QCOMPARE(windowData.value(QStringLiteral("keepBelow")).toBool(), false);
    QCOMPARE(windowData.value(QStringLiteral("skipTaskbar")).toBool(), false);
    QCOMPARE(windowData.value(QStringLiteral("skipPager")).toBool(), false);
    QCOMPARE(windowData.value(QStringLiteral("skipSwitcher")).toBool(), false);
    QCOMPARE(windowData.value(QStringLiteral("maximizeHorizontal")).toBool(), false);
    QCOMPARE(windowData.value(QStringLiteral("maximizeVertical")).toBool(), false);
    QCOMPARE(windowData.value(QStringLiteral("noBorder")).toBool(), false);
    QCOMPARE(windowData.value(QStringLiteral("role")).toString(), QString());
    QCOMPARE(windowData.value(QStringLiteral("resourceName")).toString(), QStringLiteral("foo"));
    QCOMPARE(windowData.value(QStringLiteral("resourceClass")).toString(), QStringLiteral("bar"));
    QCOMPARE(windowData.value(QStringLiteral("desktopFile")).toString(), QStringLiteral("org.kde.foo"));
    QCOMPARE(windowData.value(QStringLiteral("caption")).toString(), QStringLiteral("Some caption"));
    // not testing clientmachine as that is system dependent
    // due to that also not testing localhost

    auto verifyProperty = [client] (const QString &name) {
        QDBusPendingReply<QVariantMap> reply{getWindowInfo(client->internalId())};
        reply.waitForFinished();
        return reply.value().value(name).toBool();
    };

    QVERIFY(!client->isMinimized());
    client->setMinimized(true);
    QVERIFY(client->isMinimized());
    QCOMPARE(verifyProperty(QStringLiteral("minimized")), true);

    QVERIFY(!client->keepAbove());
    client->setKeepAbove(true);
    QVERIFY(client->keepAbove());
    QCOMPARE(verifyProperty(QStringLiteral("keepAbove")), true);

    QVERIFY(!client->keepBelow());
    client->setKeepBelow(true);
    QVERIFY(client->keepBelow());
    QCOMPARE(verifyProperty(QStringLiteral("keepBelow")), true);

    QVERIFY(!client->skipTaskbar());
    client->setSkipTaskbar(true);
    QVERIFY(client->skipTaskbar());
    QCOMPARE(verifyProperty(QStringLiteral("skipTaskbar")), true);

    QVERIFY(!client->skipPager());
    client->setSkipPager(true);
    QVERIFY(client->skipPager());
    QCOMPARE(verifyProperty(QStringLiteral("skipPager")), true);

    QVERIFY(!client->skipSwitcher());
    client->setSkipSwitcher(true);
    QVERIFY(client->skipSwitcher());
    QCOMPARE(verifyProperty(QStringLiteral("skipSwitcher")), true);

    QVERIFY(!client->isShade());
    client->setShade(ShadeNormal);
    QVERIFY(client->isShade());
    QCOMPARE(verifyProperty(QStringLiteral("shaded")), true);
    client->setShade(ShadeNone);
    QVERIFY(!client->isShade());

    QVERIFY(!client->noBorder());
    client->setNoBorder(true);
    QVERIFY(client->noBorder());
    QCOMPARE(verifyProperty(QStringLiteral("noBorder")), true);
    client->setNoBorder(false);
    QVERIFY(!client->noBorder());

    QVERIFY(!client->isFullScreen());
    client->setFullScreen(true);
    QVERIFY(client->isFullScreen());
    QVERIFY(client->clientSize() != windowGeometry.size());
    QCOMPARE(verifyProperty(QStringLiteral("fullscreen")), true);
    reply = getWindowInfo(client->internalId());
    reply.waitForFinished();
    QCOMPARE(reply.value().value(QStringLiteral("width")).toInt(), client->width());
    QCOMPARE(reply.value().value(QStringLiteral("height")).toInt(), client->height());
    client->setFullScreen(false);
    QVERIFY(!client->isFullScreen());

    // maximize
    client->setMaximize(true, false);
    QCOMPARE(verifyProperty(QStringLiteral("maximizeVertical")), true);
    QCOMPARE(verifyProperty(QStringLiteral("maximizeHorizontal")), false);
    client->setMaximize(false, true);
    QCOMPARE(verifyProperty(QStringLiteral("maximizeVertical")), false);
    QCOMPARE(verifyProperty(QStringLiteral("maximizeHorizontal")), true);

    const auto id = client->internalId();
    // destroy the window
    xcb_unmap_window(c.data(), w);
    xcb_flush(c.data());

    QSignalSpy windowClosedSpy(client, &Client::windowClosed);
    QVERIFY(windowClosedSpy.isValid());
    QVERIFY(windowClosedSpy.wait());
    xcb_destroy_window(c.data(), w);
    c.reset();

    reply = getWindowInfo(id);
    reply.waitForFinished();
    QVERIFY(reply.value().empty());
}

WAYLANDTEST_MAIN(TestDbusInterface)
#include "dbus_interface_test.moc"
