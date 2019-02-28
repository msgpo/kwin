/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2017 Martin Flöser <mgraesslin@kde.org>
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
#include "rules.h"
#include "screens.h"
#include "shell_client.h"
#include "virtualdesktops.h"
#include "wayland_server.h"
#include "workspace.h"

#include <KWayland/Client/surface.h>
#include <KWayland/Client/xdgshell.h>

#include <linux/input.h>

using namespace KWin;
using namespace KWayland::Client;

static const QString s_socketName = QStringLiteral("wayland_test_kwin_shell_client_rules-0");


class TestShellClientRules : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void testPositionDontAffect_data();
    void testPositionDontAffect();
    void testPositionApply_data();
    void testPositionApply();
    void testPositionRemember_data();
    void testPositionRemember();
    void testPositionForce_data();
    void testPositionForce();
    void testPositionApplyNow_data();
    void testPositionApplyNow();
    void testPositionForceTemporarily_data();
    void testPositionForceTemporarily();

    void testSizeDontAffect_data();
    void testSizeDontAffect();
    void testSizeApply_data();
    void testSizeApply();
    void testSizeRemember_data();
    void testSizeRemember();
    void testSizeForce_data();
    void testSizeForce();
    void testSizeApplyNow_data();
    void testSizeApplyNow();
    void testSizeForceTemporarily_data();
    void testSizeForceTemporarily();

    void testMaximizeDontAffect_data();
    void testMaximizeDontAffect();
    void testMaximizeApply_data();
    void testMaximizeApply();
    void testMaximizeRemember_data();
    void testMaximizeRemember();
    void testMaximizeForce_data();
    void testMaximizeForce();
    void testMaximizeApplyNow_data();
    void testMaximizeApplyNow();
    void testMaximizeForceTemporarily_data();
    void testMaximizeForceTemporarily();

    void testDesktopDontAffect_data();
    void testDesktopDontAffect();
    void testDesktopApply_data();
    void testDesktopApply();
    void testDesktopRemember_data();
    void testDesktopRemember();
    void testDesktopForce_data();
    void testDesktopForce();
    void testDesktopApplyNow_data();
    void testDesktopApplyNow();
    void testDesktopForceTemporarily_data();
    void testDesktopForceTemporarily();

    void testMinimizeDontAffect_data();
    void testMinimizeDontAffect();
    void testMinimizeApply_data();
    void testMinimizeApply();
    void testMinimizeRemember_data();
    void testMinimizeRemember();
    void testMinimizeForce_data();
    void testMinimizeForce();
    void testMinimizeApplyNow_data();
    void testMinimizeApplyNow();
    void testMinimizeForceTemporarily_data();
    void testMinimizeForceTemporarily();

    void testSkipTaskbarDontAffect_data();
    void testSkipTaskbarDontAffect();
    void testSkipTaskbarApply_data();
    void testSkipTaskbarApply();
    void testSkipTaskbarRemember_data();
    void testSkipTaskbarRemember();
    void testSkipTaskbarForce_data();
    void testSkipTaskbarForce();
    void testSkipTaskbarApplyNow_data();
    void testSkipTaskbarApplyNow();
    void testSkipTaskbarForceTemporarily_data();
    void testSkipTaskbarForceTemporarily();

    void testSkipPagerDontAffect_data();
    void testSkipPagerDontAffect();
    void testSkipPagerApply_data();
    void testSkipPagerApply();
    void testSkipPagerRemember_data();
    void testSkipPagerRemember();
    void testSkipPagerForce_data();
    void testSkipPagerForce();
    void testSkipPagerApplyNow_data();
    void testSkipPagerApplyNow();
    void testSkipPagerForceTemporarily_data();
    void testSkipPagerForceTemporarily();

    void testSkipSwitcherDontAffect_data();
    void testSkipSwitcherDontAffect();
    void testSkipSwitcherApply_data();
    void testSkipSwitcherApply();
    void testSkipSwitcherRemember_data();
    void testSkipSwitcherRemember();
    void testSkipSwitcherForce_data();
    void testSkipSwitcherForce();
    void testSkipSwitcherApplyNow_data();
    void testSkipSwitcherApplyNow();
    void testSkipSwitcherForceTemporarily_data();
    void testSkipSwitcherForceTemporarily();

    void testKeepAboveDontAffect_data();
    void testKeepAboveDontAffect();
    void testKeepAboveApply_data();
    void testKeepAboveApply();
    void testKeepAboveRemember_data();
    void testKeepAboveRemember();
    void testKeepAboveForce_data();
    void testKeepAboveForce();
    void testKeepAboveApplyNow_data();
    void testKeepAboveApplyNow();
    void testKeepAboveForceTemporarily_data();
    void testKeepAboveForceTemporarily();

    void testKeepBelowDontAffect_data();
    void testKeepBelowDontAffect();
    void testKeepBelowApply_data();
    void testKeepBelowApply();
    void testKeepBelowRemember_data();
    void testKeepBelowRemember();
    void testKeepBelowForce_data();
    void testKeepBelowForce();
    void testKeepBelowApplyNow_data();
    void testKeepBelowApplyNow();
    void testKeepBelowForceTemporarily_data();
    void testKeepBelowForceTemporarily();

    void testShortcutDontAffect_data();
    void testShortcutDontAffect();
    void testShortcutApply_data();
    void testShortcutApply();
    void testShortcutRemember_data();
    void testShortcutRemember();
    void testShortcutForce_data();
    void testShortcutForce();
    void testShortcutApplyNow_data();
    void testShortcutApplyNow();
    void testShortcutForceTemporarily_data();
    void testShortcutForceTemporarily();

    void testDesktopFileDontAffect_data();
    void testDesktopFileDontAffect();
    void testDesktopFileApply_data();
    void testDesktopFileApply();
    void testDesktopFileRemember_data();
    void testDesktopFileRemember();
    void testDesktopFileForce_data();
    void testDesktopFileForce();
    void testDesktopFileApplyNow_data();
    void testDesktopFileApplyNow();
    void testDesktopFileForceTemporarily_data();
    void testDesktopFileForceTemporarily();

    void testActiveOpacityDontAffect_data();
    void testActiveOpacityDontAffect();
    void testActiveOpacityForce_data();
    void testActiveOpacityForce();
    void testActiveOpacityForceTemporarily_data();
    void testActiveOpacityForceTemporarily();

    void testInactiveOpacityDontAffect_data();
    void testInactiveOpacityDontAffect();
    void testInactiveOpacityForce_data();
    void testInactiveOpacityForce();
    void testInactiveOpacityForceTemporarily_data();
    void testInactiveOpacityForceTemporarily();

    void testMatchAfterNameChange();
};

void TestShellClientRules::initTestCase()
{
    qRegisterMetaType<KWin::ShellClient*>();
    qRegisterMetaType<KWin::AbstractClient*>();

    QSignalSpy workspaceCreatedSpy(kwinApp(), &Application::workspaceCreated);
    QVERIFY(workspaceCreatedSpy.isValid());
    kwinApp()->platform()->setInitialWindowSize(QSize(1280, 1024));
    QMetaObject::invokeMethod(kwinApp()->platform(), "setVirtualOutputs", Qt::DirectConnection, Q_ARG(int, 2));
    QVERIFY(waylandServer()->init(s_socketName.toLocal8Bit()));

    kwinApp()->start();
    QVERIFY(workspaceCreatedSpy.wait());
    QCOMPARE(screens()->count(), 2);
    QCOMPARE(screens()->geometry(0), QRect(0, 0, 1280, 1024));
    QCOMPARE(screens()->geometry(1), QRect(1280, 0, 1280, 1024));
    waylandServer()->initWorkspace();
}

void TestShellClientRules::init()
{
    VirtualDesktopManager::self()->setCurrent(VirtualDesktopManager::self()->desktops().first());
    QVERIFY(Test::setupWaylandConnection(Test::AdditionalWaylandInterface::Decoration));

    screens()->setCurrent(0);
}

void TestShellClientRules::cleanup()
{
    Test::destroyWaylandConnection();

    // Unreference the previous config.
    RuleBook::self()->setConfig({});
    workspace()->slotReconfigure();

    // Restore virtual desktops to the initial state.
    VirtualDesktopManager::self()->setCount(1);
    QCOMPARE(VirtualDesktopManager::self()->count(), 1u);
}

#define TEST_DATA(name) \
void TestShellClientRules::name##_data() \
{ \
    QTest::addColumn<Test::ShellSurfaceType>("type"); \
    QTest::newRow("XdgShellV5") << Test::ShellSurfaceType::XdgShellV5; \
    QTest::newRow("XdgShellV6") << Test::ShellSurfaceType::XdgShellV6; \
    QTest::newRow("XdgWmBase") << Test::ShellSurfaceType::XdgShellStable; \
}

std::tuple<ShellClient *, Surface *, XdgShellSurface *> createWindow(Test::ShellSurfaceType type, const QByteArray &appId)
{
    // Create an xdg surface.
    Surface *surface = Test::createSurface();
    XdgShellSurface *shellSurface = Test::createXdgShellSurface(type, surface, surface, Test::CreationSetup::CreateOnly);

    // Assign the desired app id.
    shellSurface->setAppId(appId);

    // Wait for the initial configure event.
    QSignalSpy configureRequestedSpy(shellSurface, &XdgShellSurface::configureRequested);
    surface->commit(Surface::CommitFlag::None);
    configureRequestedSpy.wait();

    // Draw content of the surface.
    shellSurface->ackConfigure(configureRequestedSpy.last().at(2).value<quint32>());
    ShellClient *client = Test::renderAndWaitForShown(surface, QSize(100, 50), Qt::blue);

    return {client, surface, shellSurface};
}

TEST_DATA(testPositionDontAffect)

void TestShellClientRules::testPositionDontAffect()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("position", QPoint(42, 42));
    group.writeEntry("positionrule", int(Rules::DontAffect));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());

    // The position of the client should not be affected by the rule. The default
    // placement policy will put the client in the top-left corner of the screen.
    QVERIFY(client->isMovable());
    QVERIFY(client->isMovableAcrossScreens());
    QCOMPARE(client->pos(), QPoint(0, 0));

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testPositionApply)

void TestShellClientRules::testPositionApply()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("position", QPoint(42, 42));
    group.writeEntry("positionrule", int(Rules::Apply));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());

    // The client should be moved to the position specified by the rule.
    QVERIFY(client->isMovable());
    QVERIFY(client->isMovableAcrossScreens());
    QCOMPARE(client->pos(), QPoint(42, 42));

    // One should still be able to move the client around.
    QSignalSpy clientStartMoveResizedSpy(client, &AbstractClient::clientStartUserMovedResized);
    QVERIFY(clientStartMoveResizedSpy.isValid());
    QSignalSpy clientStepUserMovedResizedSpy(client, &AbstractClient::clientStepUserMovedResized);
    QVERIFY(clientStepUserMovedResizedSpy.isValid());
    QSignalSpy clientFinishUserMovedResizedSpy(client, &AbstractClient::clientFinishUserMovedResized);
    QVERIFY(clientFinishUserMovedResizedSpy.isValid());

    QCOMPARE(workspace()->getMovingClient(), nullptr);
    QVERIFY(!client->isMove());
    QVERIFY(!client->isResize());
    workspace()->slotWindowMove();
    QCOMPARE(workspace()->getMovingClient(), client);
    QCOMPARE(clientStartMoveResizedSpy.count(), 1);
    QVERIFY(client->isMove());
    QVERIFY(!client->isResize());

    const QPoint cursorPos = KWin::Cursor::pos();
    client->keyPressEvent(Qt::Key_Right);
    client->updateMoveResize(KWin::Cursor::pos());
    QCOMPARE(KWin::Cursor::pos(), cursorPos + QPoint(8, 0));
    QCOMPARE(clientStepUserMovedResizedSpy.count(), 1);
    QCOMPARE(client->pos(), QPoint(50, 42));

    client->keyPressEvent(Qt::Key_Enter);
    QCOMPARE(clientFinishUserMovedResizedSpy.count(), 1);
    QCOMPARE(workspace()->getMovingClient(), nullptr);
    QVERIFY(!client->isMove());
    QVERIFY(!client->isResize());
    QCOMPARE(client->pos(), QPoint(50, 42));

    // The rule should be applied again if the client appears after it's been closed.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(client->isMovable());
    QVERIFY(client->isMovableAcrossScreens());
    QCOMPARE(client->pos(), QPoint(42, 42));

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testPositionRemember)

void TestShellClientRules::testPositionRemember()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("position", QPoint(42, 42));
    group.writeEntry("positionrule", int(Rules::Remember));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());

    // The client should be moved to the position specified by the rule.
    QVERIFY(client->isMovable());
    QVERIFY(client->isMovableAcrossScreens());
    QCOMPARE(client->pos(), QPoint(42, 42));

    // One should still be able to move the client around.
    QSignalSpy clientStartMoveResizedSpy(client, &AbstractClient::clientStartUserMovedResized);
    QVERIFY(clientStartMoveResizedSpy.isValid());
    QSignalSpy clientStepUserMovedResizedSpy(client, &AbstractClient::clientStepUserMovedResized);
    QVERIFY(clientStepUserMovedResizedSpy.isValid());
    QSignalSpy clientFinishUserMovedResizedSpy(client, &AbstractClient::clientFinishUserMovedResized);
    QVERIFY(clientFinishUserMovedResizedSpy.isValid());

    QCOMPARE(workspace()->getMovingClient(), nullptr);
    QVERIFY(!client->isMove());
    QVERIFY(!client->isResize());
    workspace()->slotWindowMove();
    QCOMPARE(workspace()->getMovingClient(), client);
    QCOMPARE(clientStartMoveResizedSpy.count(), 1);
    QVERIFY(client->isMove());
    QVERIFY(!client->isResize());

    const QPoint cursorPos = KWin::Cursor::pos();
    client->keyPressEvent(Qt::Key_Right);
    client->updateMoveResize(KWin::Cursor::pos());
    QCOMPARE(KWin::Cursor::pos(), cursorPos + QPoint(8, 0));
    QCOMPARE(clientStepUserMovedResizedSpy.count(), 1);
    QCOMPARE(client->pos(), QPoint(50, 42));

    client->keyPressEvent(Qt::Key_Enter);
    QCOMPARE(clientFinishUserMovedResizedSpy.count(), 1);
    QCOMPARE(workspace()->getMovingClient(), nullptr);
    QVERIFY(!client->isMove());
    QVERIFY(!client->isResize());
    QCOMPARE(client->pos(), QPoint(50, 42));

    // The client should be placed at the last know position if we reopen it.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(client->isMovable());
    QVERIFY(client->isMovableAcrossScreens());
    QCOMPARE(client->pos(), QPoint(50, 42));

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testPositionForce)

void TestShellClientRules::testPositionForce()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("position", QPoint(42, 42));
    group.writeEntry("positionrule", int(Rules::Force));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());

    // The client should be moved to the position specified by the rule.
    QVERIFY(!client->isMovable());
    QVERIFY(!client->isMovableAcrossScreens());
    QCOMPARE(client->pos(), QPoint(42, 42));

    // User should not be able to move the client.
    QSignalSpy clientStartMoveResizedSpy(client, &AbstractClient::clientStartUserMovedResized);
    QVERIFY(clientStartMoveResizedSpy.isValid());
    QCOMPARE(workspace()->getMovingClient(), nullptr);
    QVERIFY(!client->isMove());
    QVERIFY(!client->isResize());
    workspace()->slotWindowMove();
    QCOMPARE(workspace()->getMovingClient(), nullptr);
    QCOMPARE(clientStartMoveResizedSpy.count(), 0);
    QVERIFY(!client->isMove());
    QVERIFY(!client->isResize());

    // The position should still be forced if we reopen the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(!client->isMovable());
    QVERIFY(!client->isMovableAcrossScreens());
    QCOMPARE(client->pos(), QPoint(42, 42));

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testPositionApplyNow)

void TestShellClientRules::testPositionApplyNow()
{
    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());

    // The position of the client isn't set by any rule, thus the default placement
    // policy will try to put the client in the top-left corner of the screen.
    QVERIFY(client->isMovable());
    QVERIFY(client->isMovableAcrossScreens());
    QCOMPARE(client->pos(), QPoint(0, 0));

    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("position", QPoint(42, 42));
    group.writeEntry("positionrule", int(Rules::ApplyNow));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);

    // The client should be moved to the position specified by the rule.
    QSignalSpy geometryChangedSpy(client, &AbstractClient::geometryChanged);
    QVERIFY(geometryChangedSpy.isValid());
    workspace()->slotReconfigure();
    QCOMPARE(geometryChangedSpy.count(), 1);
    QCOMPARE(client->pos(), QPoint(42, 42));

    // We still have to be able to move the client around.
    QVERIFY(client->isMovable());
    QVERIFY(client->isMovableAcrossScreens());
    QSignalSpy clientStartMoveResizedSpy(client, &AbstractClient::clientStartUserMovedResized);
    QVERIFY(clientStartMoveResizedSpy.isValid());
    QSignalSpy clientStepUserMovedResizedSpy(client, &AbstractClient::clientStepUserMovedResized);
    QVERIFY(clientStepUserMovedResizedSpy.isValid());
    QSignalSpy clientFinishUserMovedResizedSpy(client, &AbstractClient::clientFinishUserMovedResized);
    QVERIFY(clientFinishUserMovedResizedSpy.isValid());

    QCOMPARE(workspace()->getMovingClient(), nullptr);
    QVERIFY(!client->isMove());
    QVERIFY(!client->isResize());
    workspace()->slotWindowMove();
    QCOMPARE(workspace()->getMovingClient(), client);
    QCOMPARE(clientStartMoveResizedSpy.count(), 1);
    QVERIFY(client->isMove());
    QVERIFY(!client->isResize());

    const QPoint cursorPos = KWin::Cursor::pos();
    client->keyPressEvent(Qt::Key_Right);
    client->updateMoveResize(KWin::Cursor::pos());
    QCOMPARE(KWin::Cursor::pos(), cursorPos + QPoint(8, 0));
    QCOMPARE(clientStepUserMovedResizedSpy.count(), 1);
    QCOMPARE(client->pos(), QPoint(50, 42));

    client->keyPressEvent(Qt::Key_Enter);
    QCOMPARE(clientFinishUserMovedResizedSpy.count(), 1);
    QCOMPARE(workspace()->getMovingClient(), nullptr);
    QVERIFY(!client->isMove());
    QVERIFY(!client->isResize());
    QCOMPARE(client->pos(), QPoint(50, 42));

    // The rule should not be applied again.
    client->evaluateWindowRules();
    QCOMPARE(client->pos(), QPoint(50, 42));

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testPositionForceTemporarily)

void TestShellClientRules::testPositionForceTemporarily()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("position", QPoint(42, 42));
    group.writeEntry("positionrule", int(Rules::ForceTemporarily));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());

    // The client should be moved to the position specified by the rule.
    QVERIFY(!client->isMovable());
    QVERIFY(!client->isMovableAcrossScreens());
    QCOMPARE(client->pos(), QPoint(42, 42));

    // User should not be able to move the client.
    QSignalSpy clientStartMoveResizedSpy(client, &AbstractClient::clientStartUserMovedResized);
    QVERIFY(clientStartMoveResizedSpy.isValid());
    QCOMPARE(workspace()->getMovingClient(), nullptr);
    QVERIFY(!client->isMove());
    QVERIFY(!client->isResize());
    workspace()->slotWindowMove();
    QCOMPARE(workspace()->getMovingClient(), nullptr);
    QCOMPARE(clientStartMoveResizedSpy.count(), 0);
    QVERIFY(!client->isMove());
    QVERIFY(!client->isResize());

    // The rule should be discarded if we close the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(client->isMovable());
    QVERIFY(client->isMovableAcrossScreens());
    QCOMPARE(client->pos(), QPoint(0, 0));

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSizeDontAffect)

void TestShellClientRules::testSizeDontAffect()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("size", QSize(480, 640));
    group.writeEntry("sizerule", int(Rules::DontAffect));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    QScopedPointer<Surface> surface;
    surface.reset(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface;
    shellSurface.reset(createXdgShellSurface(type, surface.data(), surface.data(), Test::CreationSetup::CreateOnly));
    QScopedPointer<QSignalSpy> configureRequestedSpy;
    configureRequestedSpy.reset(new QSignalSpy(shellSurface.data(), &XdgShellSurface::configureRequested));
    shellSurface->setAppId("org.kde.foo");
    surface->commit(Surface::CommitFlag::None);

    // The window size shouldn't be enforced by the rule.
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 1);
    QCOMPARE(configureRequestedSpy->last().first().toSize(), QSize(0, 0));

    // Map the client.
    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    ShellClient *client = Test::renderAndWaitForShown(surface.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(client->isResizable());
    QCOMPARE(client->size(), QSize(100, 50));

    // We should receive a configure event when the client becomes active.
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 2);

    // Destroy the client.
    shellSurface.reset();
    surface.reset();
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSizeApply)

void TestShellClientRules::testSizeApply()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("size", QSize(480, 640));
    group.writeEntry("sizerule", int(Rules::Apply));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    QScopedPointer<Surface> surface;
    surface.reset(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface;
    shellSurface.reset(createXdgShellSurface(type, surface.data(), surface.data(), Test::CreationSetup::CreateOnly));
    QScopedPointer<QSignalSpy> configureRequestedSpy;
    configureRequestedSpy.reset(new QSignalSpy(shellSurface.data(), &XdgShellSurface::configureRequested));
    shellSurface->setAppId("org.kde.foo");
    surface->commit(Surface::CommitFlag::None);

    // The initial configure event should contain size hint set by the rule.
    XdgShellSurface::States states;
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 1);
    QCOMPARE(configureRequestedSpy->last().at(0).toSize(), QSize(480, 640));
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(!states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(!states.testFlag(XdgShellSurface::State::Resizing));

    // Map the client.
    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    ShellClient *client = Test::renderAndWaitForShown(surface.data(), QSize(480, 640), Qt::blue);
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(client->isResizable());
    QCOMPARE(client->size(), QSize(480, 640));

    // We should receive a configure event when the client becomes active.
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 2);
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(!states.testFlag(XdgShellSurface::State::Resizing));

    // One still should be able to resize the client.
    QSignalSpy geometryChangedSpy(client, &AbstractClient::geometryChanged);
    QVERIFY(geometryChangedSpy.isValid());
    QSignalSpy clientStartMoveResizedSpy(client, &AbstractClient::clientStartUserMovedResized);
    QVERIFY(clientStartMoveResizedSpy.isValid());
    QSignalSpy clientStepUserMovedResizedSpy(client, &AbstractClient::clientStepUserMovedResized);
    QVERIFY(clientStepUserMovedResizedSpy.isValid());
    QSignalSpy clientFinishUserMovedResizedSpy(client, &AbstractClient::clientFinishUserMovedResized);
    QVERIFY(clientFinishUserMovedResizedSpy.isValid());
    QSignalSpy surfaceSizeChangedSpy(shellSurface.data(), &XdgShellSurface::sizeChanged);
    QVERIFY(surfaceSizeChangedSpy.isValid());

    QCOMPARE(workspace()->getMovingClient(), nullptr);
    QVERIFY(!client->isMove());
    QVERIFY(!client->isResize());
    workspace()->slotWindowResize();
    QCOMPARE(workspace()->getMovingClient(), client);
    QCOMPARE(clientStartMoveResizedSpy.count(), 1);
    QVERIFY(!client->isMove());
    QVERIFY(client->isResize());
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 3);
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(states.testFlag(XdgShellSurface::State::Resizing));
    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());

    const QPoint cursorPos = KWin::Cursor::pos();
    client->keyPressEvent(Qt::Key_Right);
    client->updateMoveResize(KWin::Cursor::pos());
    QCOMPARE(KWin::Cursor::pos(), cursorPos + QPoint(8, 0));
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 4);
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(states.testFlag(XdgShellSurface::State::Resizing));
    QCOMPARE(surfaceSizeChangedSpy.count(), 1);
    QCOMPARE(surfaceSizeChangedSpy.last().first().toSize(), QSize(488, 640));
    QCOMPARE(clientStepUserMovedResizedSpy.count(), 0);
    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    Test::render(surface.data(), QSize(488, 640), Qt::blue);
    QVERIFY(geometryChangedSpy.wait());
    QCOMPARE(client->size(), QSize(488, 640));
    QCOMPARE(clientStepUserMovedResizedSpy.count(), 1);

    client->keyPressEvent(Qt::Key_Enter);
    QCOMPARE(clientFinishUserMovedResizedSpy.count(), 1);
    QCOMPARE(workspace()->getMovingClient(), nullptr);
    QVERIFY(!client->isMove());
    QVERIFY(!client->isResize());

    QEXPECT_FAIL("", "Interactive resize is not spec-compliant", Continue);
    QVERIFY(configureRequestedSpy->wait(10));
    QEXPECT_FAIL("", "Interactive resize is not spec-compliant", Continue);
    QCOMPARE(configureRequestedSpy->count(), 5);

    // The rule should be applied again if the client appears after it's been closed.
    shellSurface.reset();
    surface.reset();
    QVERIFY(Test::waitForWindowDestroyed(client));
    surface.reset(Test::createSurface());
    shellSurface.reset(createXdgShellSurface(type, surface.data(), surface.data(), Test::CreationSetup::CreateOnly));
    configureRequestedSpy.reset(new QSignalSpy(shellSurface.data(), &XdgShellSurface::configureRequested));
    shellSurface->setAppId("org.kde.foo");
    surface->commit(Surface::CommitFlag::None);

    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 1);
    QCOMPARE(configureRequestedSpy->last().first().toSize(), QSize(480, 640));

    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    client = Test::renderAndWaitForShown(surface.data(), QSize(480, 640), Qt::blue);
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(client->isResizable());
    QCOMPARE(client->size(), QSize(480, 640));

    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 2);

    // Destroy the client.
    shellSurface.reset();
    surface.reset();
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSizeRemember)

void TestShellClientRules::testSizeRemember()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("size", QSize(480, 640));
    group.writeEntry("sizerule", int(Rules::Remember));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    QScopedPointer<Surface> surface;
    surface.reset(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface;
    shellSurface.reset(createXdgShellSurface(type, surface.data(), surface.data(), Test::CreationSetup::CreateOnly));
    QScopedPointer<QSignalSpy> configureRequestedSpy;
    configureRequestedSpy.reset(new QSignalSpy(shellSurface.data(), &XdgShellSurface::configureRequested));
    shellSurface->setAppId("org.kde.foo");
    surface->commit(Surface::CommitFlag::None);

    // The initial configure event should contain size hint set by the rule.
    XdgShellSurface::States states;
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 1);
    QCOMPARE(configureRequestedSpy->last().first().toSize(), QSize(480, 640));
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(!states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(!states.testFlag(XdgShellSurface::State::Resizing));

    // Map the client.
    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    ShellClient *client = Test::renderAndWaitForShown(surface.data(), QSize(480, 640), Qt::blue);
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(client->isResizable());
    QCOMPARE(client->size(), QSize(480, 640));

    // We should receive a configure event when the client becomes active.
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 2);
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(!states.testFlag(XdgShellSurface::State::Resizing));

    // One should still be able to resize the client.
    QSignalSpy geometryChangedSpy(client, &AbstractClient::geometryChanged);
    QVERIFY(geometryChangedSpy.isValid());
    QSignalSpy clientStartMoveResizedSpy(client, &AbstractClient::clientStartUserMovedResized);
    QVERIFY(clientStartMoveResizedSpy.isValid());
    QSignalSpy clientStepUserMovedResizedSpy(client, &AbstractClient::clientStepUserMovedResized);
    QVERIFY(clientStepUserMovedResizedSpy.isValid());
    QSignalSpy clientFinishUserMovedResizedSpy(client, &AbstractClient::clientFinishUserMovedResized);
    QVERIFY(clientFinishUserMovedResizedSpy.isValid());
    QSignalSpy surfaceSizeChangedSpy(shellSurface.data(), &XdgShellSurface::sizeChanged);
    QVERIFY(surfaceSizeChangedSpy.isValid());

    QCOMPARE(workspace()->getMovingClient(), nullptr);
    QVERIFY(!client->isMove());
    QVERIFY(!client->isResize());
    workspace()->slotWindowResize();
    QCOMPARE(workspace()->getMovingClient(), client);
    QCOMPARE(clientStartMoveResizedSpy.count(), 1);
    QVERIFY(!client->isMove());
    QVERIFY(client->isResize());
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 3);
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(states.testFlag(XdgShellSurface::State::Resizing));
    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());

    const QPoint cursorPos = KWin::Cursor::pos();
    client->keyPressEvent(Qt::Key_Right);
    client->updateMoveResize(KWin::Cursor::pos());
    QCOMPARE(KWin::Cursor::pos(), cursorPos + QPoint(8, 0));
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 4);
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(states.testFlag(XdgShellSurface::State::Resizing));
    QCOMPARE(surfaceSizeChangedSpy.count(), 1);
    QCOMPARE(surfaceSizeChangedSpy.last().first().toSize(), QSize(488, 640));
    QCOMPARE(clientStepUserMovedResizedSpy.count(), 0);
    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    Test::render(surface.data(), QSize(488, 640), Qt::blue);
    QVERIFY(geometryChangedSpy.wait());
    QCOMPARE(client->size(), QSize(488, 640));
    QCOMPARE(clientStepUserMovedResizedSpy.count(), 1);

    client->keyPressEvent(Qt::Key_Enter);
    QCOMPARE(clientFinishUserMovedResizedSpy.count(), 1);
    QCOMPARE(workspace()->getMovingClient(), nullptr);
    QVERIFY(!client->isMove());
    QVERIFY(!client->isResize());

    QEXPECT_FAIL("", "Interactive resize is not spec-compliant", Continue);
    QVERIFY(configureRequestedSpy->wait(10));
    QEXPECT_FAIL("", "Interactive resize is not spec-compliant", Continue);
    QCOMPARE(configureRequestedSpy->count(), 5);

    // If the client appears again, it should have the last known size.
    shellSurface.reset();
    surface.reset();
    QVERIFY(Test::waitForWindowDestroyed(client));
    surface.reset(Test::createSurface());
    shellSurface.reset(createXdgShellSurface(type, surface.data(), surface.data(), Test::CreationSetup::CreateOnly));
    configureRequestedSpy.reset(new QSignalSpy(shellSurface.data(), &XdgShellSurface::configureRequested));
    shellSurface->setAppId("org.kde.foo");
    surface->commit(Surface::CommitFlag::None);

    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 1);
    QCOMPARE(configureRequestedSpy->last().first().toSize(), QSize(488, 640));

    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    client = Test::renderAndWaitForShown(surface.data(), QSize(488, 640), Qt::blue);
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(client->isResizable());
    QCOMPARE(client->size(), QSize(488, 640));

    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 2);

    // Destroy the client.
    shellSurface.reset();
    surface.reset();
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSizeForce)

void TestShellClientRules::testSizeForce()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("size", QSize(480, 640));
    group.writeEntry("sizerule", int(Rules::Force));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    QScopedPointer<Surface> surface;
    surface.reset(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface;
    shellSurface.reset(createXdgShellSurface(type, surface.data(), surface.data(), Test::CreationSetup::CreateOnly));
    QScopedPointer<QSignalSpy> configureRequestedSpy;
    configureRequestedSpy.reset(new QSignalSpy(shellSurface.data(), &XdgShellSurface::configureRequested));
    shellSurface->setAppId("org.kde.foo");
    surface->commit(Surface::CommitFlag::None);

    // The initial configure event should contain size hint set by the rule.
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 1);
    QCOMPARE(configureRequestedSpy->last().first().toSize(), QSize(480, 640));

    // Map the client.
    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    ShellClient *client = Test::renderAndWaitForShown(surface.data(), QSize(480, 640), Qt::blue);
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(!client->isResizable());
    QCOMPARE(client->size(), QSize(480, 640));

    // We should receive a configure event when the client becomes active.
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 2);

    // Any attempt to resize the client should not succeed.
    QSignalSpy clientStartMoveResizedSpy(client, &AbstractClient::clientStartUserMovedResized);
    QVERIFY(clientStartMoveResizedSpy.isValid());
    QCOMPARE(workspace()->getMovingClient(), nullptr);
    QVERIFY(!client->isMove());
    QVERIFY(!client->isResize());
    workspace()->slotWindowResize();
    QCOMPARE(workspace()->getMovingClient(), nullptr);
    QCOMPARE(clientStartMoveResizedSpy.count(), 0);
    QVERIFY(!client->isMove());
    QVERIFY(!client->isResize());
    QVERIFY(!configureRequestedSpy->wait(100));

    // If the client appears again, the size should still be forced.
    shellSurface.reset();
    surface.reset();
    QVERIFY(Test::waitForWindowDestroyed(client));
    surface.reset(Test::createSurface());
    shellSurface.reset(createXdgShellSurface(type, surface.data(), surface.data(), Test::CreationSetup::CreateOnly));
    configureRequestedSpy.reset(new QSignalSpy(shellSurface.data(), &XdgShellSurface::configureRequested));
    shellSurface->setAppId("org.kde.foo");
    surface->commit(Surface::CommitFlag::None);

    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 1);
    QCOMPARE(configureRequestedSpy->last().first().toSize(), QSize(480, 640));

    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    client = Test::renderAndWaitForShown(surface.data(), QSize(480, 640), Qt::blue);
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(!client->isResizable());
    QCOMPARE(client->size(), QSize(480, 640));

    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 2);

    // Destroy the client.
    shellSurface.reset();
    surface.reset();
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSizeApplyNow)

void TestShellClientRules::testSizeApplyNow()
{
    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    QScopedPointer<Surface> surface;
    surface.reset(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface;
    shellSurface.reset(createXdgShellSurface(type, surface.data(), surface.data(), Test::CreationSetup::CreateOnly));
    QScopedPointer<QSignalSpy> configureRequestedSpy;
    configureRequestedSpy.reset(new QSignalSpy(shellSurface.data(), &XdgShellSurface::configureRequested));
    shellSurface->setAppId("org.kde.foo");
    surface->commit(Surface::CommitFlag::None);

    // The expected surface dimensions should be set by the rule.
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 1);
    QCOMPARE(configureRequestedSpy->last().first().toSize(), QSize(0, 0));

    // Map the client.
    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    ShellClient *client = Test::renderAndWaitForShown(surface.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(client->isResizable());
    QCOMPARE(client->size(), QSize(100, 50));

    // We should receive a configure event when the client becomes active.
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 2);

    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("size", QSize(480, 640));
    group.writeEntry("sizerule", int(Rules::ApplyNow));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // The compositor should send a configure event with a new size.
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 3);
    QCOMPARE(configureRequestedSpy->last().first().toSize(), QSize(480, 640));

    // Draw the surface with the new size.
    QSignalSpy geometryChangedSpy(client, &AbstractClient::geometryChanged);
    QVERIFY(geometryChangedSpy.isValid());
    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    Test::render(surface.data(), QSize(480, 640), Qt::blue);
    QVERIFY(geometryChangedSpy.wait());
    QCOMPARE(client->size(), QSize(480, 640));
    QVERIFY(!configureRequestedSpy->wait(100));

    // The rule should not be applied again.
    client->evaluateWindowRules();
    QVERIFY(!configureRequestedSpy->wait(100));

    // Destroy the client.
    shellSurface.reset();
    surface.reset();
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSizeForceTemporarily)

void TestShellClientRules::testSizeForceTemporarily()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("size", QSize(480, 640));
    group.writeEntry("sizerule", int(Rules::ForceTemporarily));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    QScopedPointer<Surface> surface;
    surface.reset(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface;
    shellSurface.reset(createXdgShellSurface(type, surface.data(), surface.data(), Test::CreationSetup::CreateOnly));
    QScopedPointer<QSignalSpy> configureRequestedSpy;
    configureRequestedSpy.reset(new QSignalSpy(shellSurface.data(), &XdgShellSurface::configureRequested));
    shellSurface->setAppId("org.kde.foo");
    surface->commit(Surface::CommitFlag::None);

    // The initial configure event should contain size hint set by the rule.
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 1);
    QCOMPARE(configureRequestedSpy->last().first().toSize(), QSize(480, 640));

    // Map the client.
    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    ShellClient *client = Test::renderAndWaitForShown(surface.data(), QSize(480, 640), Qt::blue);
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(!client->isResizable());
    QCOMPARE(client->size(), QSize(480, 640));

    // We should receive a configure event when the client becomes active.
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 2);

    // Any attempt to resize the client should not succeed.
    QSignalSpy clientStartMoveResizedSpy(client, &AbstractClient::clientStartUserMovedResized);
    QVERIFY(clientStartMoveResizedSpy.isValid());
    QCOMPARE(workspace()->getMovingClient(), nullptr);
    QVERIFY(!client->isMove());
    QVERIFY(!client->isResize());
    workspace()->slotWindowResize();
    QCOMPARE(workspace()->getMovingClient(), nullptr);
    QCOMPARE(clientStartMoveResizedSpy.count(), 0);
    QVERIFY(!client->isMove());
    QVERIFY(!client->isResize());
    QVERIFY(!configureRequestedSpy->wait(100));

    // The rule should be discarded when the client is closed.
    shellSurface.reset();
    surface.reset();
    QVERIFY(Test::waitForWindowDestroyed(client));
    surface.reset(Test::createSurface());
    shellSurface.reset(createXdgShellSurface(type, surface.data(), surface.data(), Test::CreationSetup::CreateOnly));
    configureRequestedSpy.reset(new QSignalSpy(shellSurface.data(), &XdgShellSurface::configureRequested));
    shellSurface->setAppId("org.kde.foo");
    surface->commit(Surface::CommitFlag::None);

    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 1);
    QCOMPARE(configureRequestedSpy->last().first().toSize(), QSize(0, 0));

    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    client = Test::renderAndWaitForShown(surface.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(client->isResizable());
    QCOMPARE(client->size(), QSize(100, 50));

    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 2);

    // Destroy the client.
    shellSurface.reset();
    surface.reset();
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testMaximizeDontAffect)

void TestShellClientRules::testMaximizeDontAffect()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("maximizehoriz", true);
    group.writeEntry("maximizehorizrule", int(Rules::DontAffect));
    group.writeEntry("maximizevert", true);
    group.writeEntry("maximizevertrule", int(Rules::DontAffect));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    QScopedPointer<Surface> surface;
    surface.reset(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface;
    shellSurface.reset(createXdgShellSurface(type, surface.data(), surface.data(), Test::CreationSetup::CreateOnly));
    QScopedPointer<QSignalSpy> configureRequestedSpy;
    configureRequestedSpy.reset(new QSignalSpy(shellSurface.data(), &XdgShellSurface::configureRequested));
    shellSurface->setAppId("org.kde.foo");
    surface->commit(Surface::CommitFlag::None);

    // Wait for the initial configure event.
    XdgShellSurface::States states;
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 1);
    QCOMPARE(configureRequestedSpy->last().at(0).toSize(), QSize(0, 0));
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(!states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(!states.testFlag(XdgShellSurface::State::Maximized));

    // Map the client.
    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    ShellClient *client = Test::renderAndWaitForShown(surface.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(client->isMaximizable());
    QCOMPARE(client->maximizeMode(), MaximizeMode::MaximizeRestore);
    QCOMPARE(client->requestedMaximizeMode(), MaximizeMode::MaximizeRestore);
    QCOMPARE(client->size(), QSize(100, 50));

    // We should receive a configure event when the client becomes active.
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 2);
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(!states.testFlag(XdgShellSurface::State::Maximized));

    // Destroy the client.
    shellSurface.reset();
    surface.reset();
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testMaximizeApply)

void TestShellClientRules::testMaximizeApply()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("maximizehoriz", true);
    group.writeEntry("maximizehorizrule", int(Rules::Apply));
    group.writeEntry("maximizevert", true);
    group.writeEntry("maximizevertrule", int(Rules::Apply));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    QScopedPointer<Surface> surface;
    surface.reset(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface;
    shellSurface.reset(createXdgShellSurface(type, surface.data(), surface.data(), Test::CreationSetup::CreateOnly));
    QScopedPointer<QSignalSpy> configureRequestedSpy;
    configureRequestedSpy.reset(new QSignalSpy(shellSurface.data(), &XdgShellSurface::configureRequested));
    shellSurface->setAppId("org.kde.foo");
    surface->commit(Surface::CommitFlag::None);

    // Wait for the initial configure event.
    XdgShellSurface::States states;
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 1);
    QCOMPARE(configureRequestedSpy->last().at(0).toSize(), QSize(1280, 1024));
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(!states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(states.testFlag(XdgShellSurface::State::Maximized));

    // Map the client.
    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    ShellClient *client = Test::renderAndWaitForShown(surface.data(), QSize(1280, 1024), Qt::blue);
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(client->isMaximizable());
    QCOMPARE(client->maximizeMode(), MaximizeMode::MaximizeFull);
    QCOMPARE(client->requestedMaximizeMode(), MaximizeMode::MaximizeFull);
    QCOMPARE(client->size(), QSize(1280, 1024));

    // We should receive a configure event when the client becomes active.
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 2);
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(states.testFlag(XdgShellSurface::State::Maximized));

    // One should still be able to change the maximized state of the client.
    workspace()->slotWindowMaximize();
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 3);
    QCOMPARE(configureRequestedSpy->last().at(0).toSize(), QSize(0, 0));
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(!states.testFlag(XdgShellSurface::State::Maximized));

    QSignalSpy geometryChangedSpy(client, &AbstractClient::geometryChanged);
    QVERIFY(geometryChangedSpy.isValid());
    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    Test::render(surface.data(), QSize(100, 50), Qt::blue);
    QVERIFY(geometryChangedSpy.wait());
    QCOMPARE(client->size(), QSize(100, 50));
    QCOMPARE(client->maximizeMode(), MaximizeMode::MaximizeRestore);
    QCOMPARE(client->requestedMaximizeMode(), MaximizeMode::MaximizeRestore);

    // If we create the client again, it should be initially maximized.
    shellSurface.reset();
    surface.reset();
    QVERIFY(Test::waitForWindowDestroyed(client));
    surface.reset(Test::createSurface());
    shellSurface.reset(createXdgShellSurface(type, surface.data(), surface.data(), Test::CreationSetup::CreateOnly));
    configureRequestedSpy.reset(new QSignalSpy(shellSurface.data(), &XdgShellSurface::configureRequested));
    shellSurface->setAppId("org.kde.foo");
    surface->commit(Surface::CommitFlag::None);

    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 1);
    QCOMPARE(configureRequestedSpy->last().at(0).toSize(), QSize(1280, 1024));
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(!states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(states.testFlag(XdgShellSurface::State::Maximized));

    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    client = Test::renderAndWaitForShown(surface.data(), QSize(1280, 1024), Qt::blue);
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(client->isMaximizable());
    QCOMPARE(client->maximizeMode(), MaximizeMode::MaximizeFull);
    QCOMPARE(client->requestedMaximizeMode(), MaximizeMode::MaximizeFull);
    QCOMPARE(client->size(), QSize(1280, 1024));

    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 2);
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(states.testFlag(XdgShellSurface::State::Maximized));

    // Destroy the client.
    shellSurface.reset();
    surface.reset();
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testMaximizeRemember)

void TestShellClientRules::testMaximizeRemember()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("maximizehoriz", true);
    group.writeEntry("maximizehorizrule", int(Rules::Remember));
    group.writeEntry("maximizevert", true);
    group.writeEntry("maximizevertrule", int(Rules::Remember));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    QScopedPointer<Surface> surface;
    surface.reset(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface;
    shellSurface.reset(createXdgShellSurface(type, surface.data(), surface.data(), Test::CreationSetup::CreateOnly));
    QScopedPointer<QSignalSpy> configureRequestedSpy;
    configureRequestedSpy.reset(new QSignalSpy(shellSurface.data(), &XdgShellSurface::configureRequested));
    shellSurface->setAppId("org.kde.foo");
    surface->commit(Surface::CommitFlag::None);

    // Wait for the initial configure event.
    XdgShellSurface::States states;
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 1);
    QCOMPARE(configureRequestedSpy->last().at(0).toSize(), QSize(1280, 1024));
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(!states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(states.testFlag(XdgShellSurface::State::Maximized));

    // Map the client.
    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    ShellClient *client = Test::renderAndWaitForShown(surface.data(), QSize(1280, 1024), Qt::blue);
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(client->isMaximizable());
    QCOMPARE(client->maximizeMode(), MaximizeMode::MaximizeFull);
    QCOMPARE(client->requestedMaximizeMode(), MaximizeMode::MaximizeFull);
    QCOMPARE(client->size(), QSize(1280, 1024));

    // We should receive a configure event when the client becomes active.
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 2);
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(states.testFlag(XdgShellSurface::State::Maximized));

    // One should still be able to change the maximized state of the client.
    workspace()->slotWindowMaximize();
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 3);
    QCOMPARE(configureRequestedSpy->last().at(0).toSize(), QSize(0, 0));
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(!states.testFlag(XdgShellSurface::State::Maximized));

    QSignalSpy geometryChangedSpy(client, &AbstractClient::geometryChanged);
    QVERIFY(geometryChangedSpy.isValid());
    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    Test::render(surface.data(), QSize(100, 50), Qt::blue);
    QVERIFY(geometryChangedSpy.wait());
    QCOMPARE(client->size(), QSize(100, 50));
    QCOMPARE(client->maximizeMode(), MaximizeMode::MaximizeRestore);
    QCOMPARE(client->requestedMaximizeMode(), MaximizeMode::MaximizeRestore);

    // If we create the client again, it should not be maximized (because last time it wasn't).
    shellSurface.reset();
    surface.reset();
    QVERIFY(Test::waitForWindowDestroyed(client));
    surface.reset(Test::createSurface());
    shellSurface.reset(createXdgShellSurface(type, surface.data(), surface.data(), Test::CreationSetup::CreateOnly));
    configureRequestedSpy.reset(new QSignalSpy(shellSurface.data(), &XdgShellSurface::configureRequested));
    shellSurface->setAppId("org.kde.foo");
    surface->commit(Surface::CommitFlag::None);

    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 1);
    QCOMPARE(configureRequestedSpy->last().at(0).toSize(), QSize(0, 0));
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(!states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(!states.testFlag(XdgShellSurface::State::Maximized));

    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    client = Test::renderAndWaitForShown(surface.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(client->isMaximizable());
    QCOMPARE(client->maximizeMode(), MaximizeMode::MaximizeRestore);
    QCOMPARE(client->requestedMaximizeMode(), MaximizeMode::MaximizeRestore);
    QCOMPARE(client->size(), QSize(100, 50));

    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 2);
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(!states.testFlag(XdgShellSurface::State::Maximized));

    // Destroy the client.
    shellSurface.reset();
    surface.reset();
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testMaximizeForce)

void TestShellClientRules::testMaximizeForce()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("maximizehoriz", true);
    group.writeEntry("maximizehorizrule", int(Rules::Force));
    group.writeEntry("maximizevert", true);
    group.writeEntry("maximizevertrule", int(Rules::Force));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    QScopedPointer<Surface> surface;
    surface.reset(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface;
    shellSurface.reset(createXdgShellSurface(type, surface.data(), surface.data(), Test::CreationSetup::CreateOnly));
    QScopedPointer<QSignalSpy> configureRequestedSpy;
    configureRequestedSpy.reset(new QSignalSpy(shellSurface.data(), &XdgShellSurface::configureRequested));
    shellSurface->setAppId("org.kde.foo");
    surface->commit(Surface::CommitFlag::None);

    // Wait for the initial configure event.
    XdgShellSurface::States states;
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 1);
    QCOMPARE(configureRequestedSpy->last().at(0).toSize(), QSize(1280, 1024));
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(!states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(states.testFlag(XdgShellSurface::State::Maximized));

    // Map the client.
    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    ShellClient *client = Test::renderAndWaitForShown(surface.data(), QSize(1280, 1024), Qt::blue);
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(!client->isMaximizable());
    QCOMPARE(client->maximizeMode(), MaximizeMode::MaximizeFull);
    QCOMPARE(client->requestedMaximizeMode(), MaximizeMode::MaximizeFull);
    QCOMPARE(client->size(), QSize(1280, 1024));

    // We should receive a configure event when the client becomes active.
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 2);
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(states.testFlag(XdgShellSurface::State::Maximized));

    // Any attempt to change the maximized state should not succeed.
    const QRect oldGeometry = client->geometry();
    workspace()->slotWindowMaximize();
    QVERIFY(!configureRequestedSpy->wait(100));
    QCOMPARE(client->maximizeMode(), MaximizeMode::MaximizeFull);
    QCOMPARE(client->requestedMaximizeMode(), MaximizeMode::MaximizeFull);
    QCOMPARE(client->geometry(), oldGeometry);

    // If we create the client again, the maximized state should still be forced.
    shellSurface.reset();
    surface.reset();
    QVERIFY(Test::waitForWindowDestroyed(client));
    surface.reset(Test::createSurface());
    shellSurface.reset(createXdgShellSurface(type, surface.data(), surface.data(), Test::CreationSetup::CreateOnly));
    configureRequestedSpy.reset(new QSignalSpy(shellSurface.data(), &XdgShellSurface::configureRequested));
    shellSurface->setAppId("org.kde.foo");
    surface->commit(Surface::CommitFlag::None);

    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 1);
    QCOMPARE(configureRequestedSpy->last().at(0).toSize(), QSize(1280, 1024));
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(!states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(states.testFlag(XdgShellSurface::State::Maximized));

    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    client = Test::renderAndWaitForShown(surface.data(), QSize(1280, 1024), Qt::blue);
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(!client->isMaximizable());
    QCOMPARE(client->maximizeMode(), MaximizeMode::MaximizeFull);
    QCOMPARE(client->requestedMaximizeMode(), MaximizeMode::MaximizeFull);
    QCOMPARE(client->size(), QSize(1280, 1024));

    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 2);
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(states.testFlag(XdgShellSurface::State::Maximized));

    // Destroy the client.
    shellSurface.reset();
    surface.reset();
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testMaximizeApplyNow)

void TestShellClientRules::testMaximizeApplyNow()
{
    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    QScopedPointer<Surface> surface;
    surface.reset(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface;
    shellSurface.reset(createXdgShellSurface(type, surface.data(), surface.data(), Test::CreationSetup::CreateOnly));
    QScopedPointer<QSignalSpy> configureRequestedSpy;
    configureRequestedSpy.reset(new QSignalSpy(shellSurface.data(), &XdgShellSurface::configureRequested));
    shellSurface->setAppId("org.kde.foo");
    surface->commit(Surface::CommitFlag::None);

    // Wait for the initial configure event.
    XdgShellSurface::States states;
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 1);
    QCOMPARE(configureRequestedSpy->last().at(0).toSize(), QSize(0, 0));
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(!states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(!states.testFlag(XdgShellSurface::State::Maximized));

    // Map the client.
    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    ShellClient *client = Test::renderAndWaitForShown(surface.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(client->isMaximizable());
    QCOMPARE(client->maximizeMode(), MaximizeMode::MaximizeRestore);
    QCOMPARE(client->requestedMaximizeMode(), MaximizeMode::MaximizeRestore);
    QCOMPARE(client->size(), QSize(100, 50));

    // We should receive a configure event when the client becomes active.
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 2);
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(!states.testFlag(XdgShellSurface::State::Maximized));

    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("maximizehoriz", true);
    group.writeEntry("maximizehorizrule", int(Rules::ApplyNow));
    group.writeEntry("maximizevert", true);
    group.writeEntry("maximizevertrule", int(Rules::ApplyNow));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // We should receive a configure event with a new surface size.
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 3);
    QCOMPARE(configureRequestedSpy->last().at(0).toSize(), QSize(1280, 1024));
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(states.testFlag(XdgShellSurface::State::Maximized));

    // Draw contents of the maximized client.
    QSignalSpy geometryChangedSpy(client, &AbstractClient::geometryChanged);
    QVERIFY(geometryChangedSpy.isValid());
    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    Test::render(surface.data(), QSize(1280, 1024), Qt::blue);
    QVERIFY(geometryChangedSpy.wait());
    QCOMPARE(client->size(), QSize(1280, 1024));
    QCOMPARE(client->maximizeMode(), MaximizeMode::MaximizeFull);
    QCOMPARE(client->requestedMaximizeMode(), MaximizeMode::MaximizeFull);

    // The client still has to be maximizeable.
    QVERIFY(client->isMaximizable());

    // Restore the client.
    workspace()->slotWindowMaximize();
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 4);
    QCOMPARE(configureRequestedSpy->last().at(0).toSize(), QSize(100, 50));
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(!states.testFlag(XdgShellSurface::State::Maximized));

    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    Test::render(surface.data(), QSize(100, 50), Qt::blue);
    QVERIFY(geometryChangedSpy.wait());
    QCOMPARE(client->size(), QSize(100, 50));
    QCOMPARE(client->maximizeMode(), MaximizeMode::MaximizeRestore);
    QCOMPARE(client->requestedMaximizeMode(), MaximizeMode::MaximizeRestore);

    // The rule should be discarded after it's been applied.
    const QRect oldGeometry = client->geometry();
    client->evaluateWindowRules();
    QVERIFY(!configureRequestedSpy->wait(100));
    QCOMPARE(client->maximizeMode(), MaximizeMode::MaximizeRestore);
    QCOMPARE(client->requestedMaximizeMode(), MaximizeMode::MaximizeRestore);
    QCOMPARE(client->geometry(), oldGeometry);

    // Destroy the client.
    shellSurface.reset();
    surface.reset();
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testMaximizeForceTemporarily)

void TestShellClientRules::testMaximizeForceTemporarily()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("maximizehoriz", true);
    group.writeEntry("maximizehorizrule", int(Rules::ForceTemporarily));
    group.writeEntry("maximizevert", true);
    group.writeEntry("maximizevertrule", int(Rules::ForceTemporarily));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    QScopedPointer<Surface> surface;
    surface.reset(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface;
    shellSurface.reset(createXdgShellSurface(type, surface.data(), surface.data(), Test::CreationSetup::CreateOnly));
    QScopedPointer<QSignalSpy> configureRequestedSpy;
    configureRequestedSpy.reset(new QSignalSpy(shellSurface.data(), &XdgShellSurface::configureRequested));
    shellSurface->setAppId("org.kde.foo");
    surface->commit(Surface::CommitFlag::None);

    // Wait for the initial configure event.
    XdgShellSurface::States states;
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 1);
    QCOMPARE(configureRequestedSpy->last().at(0).toSize(), QSize(1280, 1024));
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(!states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(states.testFlag(XdgShellSurface::State::Maximized));

    // Map the client.
    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    ShellClient *client = Test::renderAndWaitForShown(surface.data(), QSize(1280, 1024), Qt::blue);
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(!client->isMaximizable());
    QCOMPARE(client->maximizeMode(), MaximizeMode::MaximizeFull);
    QCOMPARE(client->requestedMaximizeMode(), MaximizeMode::MaximizeFull);
    QCOMPARE(client->size(), QSize(1280, 1024));

    // We should receive a configure event when the client becomes active.
    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 2);
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(states.testFlag(XdgShellSurface::State::Maximized));

    // Any attempt to change the maximized state should not succeed.
    const QRect oldGeometry = client->geometry();
    workspace()->slotWindowMaximize();
    QVERIFY(!configureRequestedSpy->wait(100));
    QCOMPARE(client->maximizeMode(), MaximizeMode::MaximizeFull);
    QCOMPARE(client->requestedMaximizeMode(), MaximizeMode::MaximizeFull);
    QCOMPARE(client->geometry(), oldGeometry);

    // The rule should be discarded if we close the client.
    shellSurface.reset();
    surface.reset();
    QVERIFY(Test::waitForWindowDestroyed(client));
    surface.reset(Test::createSurface());
    shellSurface.reset(createXdgShellSurface(type, surface.data(), surface.data(), Test::CreationSetup::CreateOnly));
    configureRequestedSpy.reset(new QSignalSpy(shellSurface.data(), &XdgShellSurface::configureRequested));
    shellSurface->setAppId("org.kde.foo");
    surface->commit(Surface::CommitFlag::None);

    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 1);
    QCOMPARE(configureRequestedSpy->last().at(0).toSize(), QSize(0, 0));
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(!states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(!states.testFlag(XdgShellSurface::State::Maximized));

    shellSurface->ackConfigure(configureRequestedSpy->last().at(2).value<quint32>());
    client = Test::renderAndWaitForShown(surface.data(), QSize(100, 50), Qt::blue);
    QVERIFY(client);
    QVERIFY(client->isActive());
    QVERIFY(client->isMaximizable());
    QCOMPARE(client->maximizeMode(), MaximizeMode::MaximizeRestore);
    QCOMPARE(client->requestedMaximizeMode(), MaximizeMode::MaximizeRestore);
    QCOMPARE(client->size(), QSize(100, 50));

    QVERIFY(configureRequestedSpy->wait());
    QCOMPARE(configureRequestedSpy->count(), 2);
    states = configureRequestedSpy->last().at(1).value<XdgShellSurface::States>();
    QVERIFY(states.testFlag(XdgShellSurface::State::Activated));
    QVERIFY(!states.testFlag(XdgShellSurface::State::Maximized));

    // Destroy the client.
    shellSurface.reset();
    surface.reset();
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testDesktopDontAffect)

void TestShellClientRules::testDesktopDontAffect()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("desktop", 2);
    group.writeEntry("desktoprule", int(Rules::DontAffect));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // We need at least two virtual desktop for this test.
    VirtualDesktopManager::self()->setCount(2);
    QCOMPARE(VirtualDesktopManager::self()->count(), 2u);
    VirtualDesktopManager::self()->setCurrent(1);
    QCOMPARE(VirtualDesktopManager::self()->current(), 1);

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should appear on the current virtual desktop.
    QCOMPARE(client->desktop(), 1);
    QCOMPARE(VirtualDesktopManager::self()->current(), 1);

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testDesktopApply)

void TestShellClientRules::testDesktopApply()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("desktop", 2);
    group.writeEntry("desktoprule", int(Rules::Apply));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // We need at least two virtual desktop for this test.
    VirtualDesktopManager::self()->setCount(2);
    QCOMPARE(VirtualDesktopManager::self()->count(), 2u);
    VirtualDesktopManager::self()->setCurrent(1);
    QCOMPARE(VirtualDesktopManager::self()->current(), 1);

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should appear on the second virtual desktop.
    QCOMPARE(client->desktop(), 2);
    QCOMPARE(VirtualDesktopManager::self()->current(), 2);

    // We still should be able to move the client between desktops.
    workspace()->sendClientToDesktop(client, 1, true);
    QCOMPARE(client->desktop(), 1);
    QCOMPARE(VirtualDesktopManager::self()->current(), 2);

    // If we re-open the client, it should appear on the second virtual desktop again.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    VirtualDesktopManager::self()->setCurrent(1);
    QCOMPARE(VirtualDesktopManager::self()->current(), 1);
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QCOMPARE(client->desktop(), 2);
    QCOMPARE(VirtualDesktopManager::self()->current(), 2);

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testDesktopRemember)

void TestShellClientRules::testDesktopRemember()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("desktop", 2);
    group.writeEntry("desktoprule", int(Rules::Remember));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // We need at least two virtual desktop for this test.
    VirtualDesktopManager::self()->setCount(2);
    QCOMPARE(VirtualDesktopManager::self()->count(), 2u);
    VirtualDesktopManager::self()->setCurrent(1);
    QCOMPARE(VirtualDesktopManager::self()->current(), 1);

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QCOMPARE(client->desktop(), 2);
    QCOMPARE(VirtualDesktopManager::self()->current(), 2);

    // Move the client to the first virtual desktop.
    workspace()->sendClientToDesktop(client, 1, true);
    QCOMPARE(client->desktop(), 1);
    QCOMPARE(VirtualDesktopManager::self()->current(), 2);

    // If we create the client again, it should appear on the first virtual desktop.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QCOMPARE(client->desktop(), 1);
    QCOMPARE(VirtualDesktopManager::self()->current(), 1);

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testDesktopForce)

void TestShellClientRules::testDesktopForce()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("desktop", 2);
    group.writeEntry("desktoprule", int(Rules::Force));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // We need at least two virtual desktop for this test.
    VirtualDesktopManager::self()->setCount(2);
    QCOMPARE(VirtualDesktopManager::self()->count(), 2u);
    VirtualDesktopManager::self()->setCurrent(1);
    QCOMPARE(VirtualDesktopManager::self()->current(), 1);

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should appear on the second virtual desktop.
    QCOMPARE(client->desktop(), 2);
    QCOMPARE(VirtualDesktopManager::self()->current(), 2);

    // Any attempt to move the client to another virtual desktop should fail.
    workspace()->sendClientToDesktop(client, 1, true);
    QCOMPARE(client->desktop(), 2);
    QCOMPARE(VirtualDesktopManager::self()->current(), 2);

    // If we re-open the client, it should appear on the second virtual desktop again.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    VirtualDesktopManager::self()->setCurrent(1);
    QCOMPARE(VirtualDesktopManager::self()->current(), 1);
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QCOMPARE(client->desktop(), 2);
    QCOMPARE(VirtualDesktopManager::self()->current(), 2);

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testDesktopApplyNow)

void TestShellClientRules::testDesktopApplyNow()
{
    // We need at least two virtual desktop for this test.
    VirtualDesktopManager::self()->setCount(2);
    QCOMPARE(VirtualDesktopManager::self()->count(), 2u);
    VirtualDesktopManager::self()->setCurrent(1);
    QCOMPARE(VirtualDesktopManager::self()->current(), 1);

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QCOMPARE(client->desktop(), 1);
    QCOMPARE(VirtualDesktopManager::self()->current(), 1);

    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("desktop", 2);
    group.writeEntry("desktoprule", int(Rules::ApplyNow));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // The client should have been moved to the second virtual desktop.
    QCOMPARE(client->desktop(), 2);
    QCOMPARE(VirtualDesktopManager::self()->current(), 1);

    // One should still be able to move the client between desktops.
    workspace()->sendClientToDesktop(client, 1, true);
    QCOMPARE(client->desktop(), 1);
    QCOMPARE(VirtualDesktopManager::self()->current(), 1);

    // The rule should not be applied again.
    client->evaluateWindowRules();
    QCOMPARE(client->desktop(), 1);
    QCOMPARE(VirtualDesktopManager::self()->current(), 1);

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testDesktopForceTemporarily)

void TestShellClientRules::testDesktopForceTemporarily()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("desktop", 2);
    group.writeEntry("desktoprule", int(Rules::ForceTemporarily));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // We need at least two virtual desktop for this test.
    VirtualDesktopManager::self()->setCount(2);
    QCOMPARE(VirtualDesktopManager::self()->count(), 2u);
    VirtualDesktopManager::self()->setCurrent(1);
    QCOMPARE(VirtualDesktopManager::self()->current(), 1);

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should appear on the second virtual desktop.
    QCOMPARE(client->desktop(), 2);
    QCOMPARE(VirtualDesktopManager::self()->current(), 2);

    // Any attempt to move the client to another virtual desktop should fail.
    workspace()->sendClientToDesktop(client, 1, true);
    QCOMPARE(client->desktop(), 2);
    QCOMPARE(VirtualDesktopManager::self()->current(), 2);

    // The rule should be discarded when the client is withdrawn.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    VirtualDesktopManager::self()->setCurrent(1);
    QCOMPARE(VirtualDesktopManager::self()->current(), 1);
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QCOMPARE(client->desktop(), 1);
    QCOMPARE(VirtualDesktopManager::self()->current(), 1);

    // One should be able to move the client between desktops.
    workspace()->sendClientToDesktop(client, 2, true);
    QCOMPARE(client->desktop(), 2);
    QCOMPARE(VirtualDesktopManager::self()->current(), 1);
    workspace()->sendClientToDesktop(client, 1, true);
    QCOMPARE(client->desktop(), 1);
    QCOMPARE(VirtualDesktopManager::self()->current(), 1);

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testMinimizeDontAffect)

void TestShellClientRules::testMinimizeDontAffect()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("minimize", true);
    group.writeEntry("minimizerule", int(Rules::DontAffect));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isMinimizable());

    // The client should not be minimized.
    QVERIFY(!client->isMinimized());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testMinimizeApply)

void TestShellClientRules::testMinimizeApply()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("minimize", true);
    group.writeEntry("minimizerule", int(Rules::Apply));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isMinimizable());

    // The client should be minimized.
    QVERIFY(client->isMinimized());

    // We should still be able to unminimize the client.
    client->unminimize();
    QVERIFY(!client->isMinimized());

    // If we re-open the client, it should be minimized back again.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isMinimizable());
    QVERIFY(client->isMinimized());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testMinimizeRemember)

void TestShellClientRules::testMinimizeRemember()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("minimize", false);
    group.writeEntry("minimizerule", int(Rules::Remember));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isMinimizable());
    QVERIFY(!client->isMinimized());

    // Minimize the client.
    client->minimize();
    QVERIFY(client->isMinimized());

    // If we open the client again, it should be minimized.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isMinimizable());
    QVERIFY(client->isMinimized());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testMinimizeForce)

void TestShellClientRules::testMinimizeForce()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("minimize", false);
    group.writeEntry("minimizerule", int(Rules::Force));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(!client->isMinimizable());
    QVERIFY(!client->isMinimized());

    // Any attempt to minimize the client should fail.
    client->minimize();
    QVERIFY(!client->isMinimized());

    // If we re-open the client, the minimized state should still be forced.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(!client->isMinimizable());
    QVERIFY(!client->isMinimized());
    client->minimize();
    QVERIFY(!client->isMinimized());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testMinimizeApplyNow)

void TestShellClientRules::testMinimizeApplyNow()
{
    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isMinimizable());
    QVERIFY(!client->isMinimized());

    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("minimize", true);
    group.writeEntry("minimizerule", int(Rules::ApplyNow));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // The client should be minimized now.
    QVERIFY(client->isMinimizable());
    QVERIFY(client->isMinimized());

    // One is still able to unminimize the client.
    client->unminimize();
    QVERIFY(!client->isMinimized());

    // The rule should not be applied again.
    client->evaluateWindowRules();
    QVERIFY(client->isMinimizable());
    QVERIFY(!client->isMinimized());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testMinimizeForceTemporarily)

void TestShellClientRules::testMinimizeForceTemporarily()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("minimize", false);
    group.writeEntry("minimizerule", int(Rules::ForceTemporarily));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(!client->isMinimizable());
    QVERIFY(!client->isMinimized());

    // Any attempt to minimize the client should fail until the client is closed.
    client->minimize();
    QVERIFY(!client->isMinimized());

    // The rule should be discarded when the client is closed.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isMinimizable());
    QVERIFY(!client->isMinimized());
    client->minimize();
    QVERIFY(client->isMinimized());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSkipTaskbarDontAffect)

void TestShellClientRules::testSkipTaskbarDontAffect()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skiptaskbar", true);
    group.writeEntry("skiptaskbarrule", int(Rules::DontAffect));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should not be affected by the rule.
    QVERIFY(!client->skipTaskbar());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSkipTaskbarApply)

void TestShellClientRules::testSkipTaskbarApply()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skiptaskbar", true);
    group.writeEntry("skiptaskbarrule", int(Rules::Apply));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should not be included on a taskbar.
    QVERIFY(client->skipTaskbar());

    // Though one can change that.
    client->setOriginalSkipTaskbar(false);
    QVERIFY(!client->skipTaskbar());

    // Reopen the client, the rule should be applied again.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->skipTaskbar());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSkipTaskbarRemember)

void TestShellClientRules::testSkipTaskbarRemember()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skiptaskbar", true);
    group.writeEntry("skiptaskbarrule", int(Rules::Remember));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should not be included on a taskbar.
    QVERIFY(client->skipTaskbar());

    // Change the skip-taskbar state.
    client->setOriginalSkipTaskbar(false);
    QVERIFY(!client->skipTaskbar());

    // Reopen the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should be included on a taskbar.
    QVERIFY(!client->skipTaskbar());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSkipTaskbarForce)

void TestShellClientRules::testSkipTaskbarForce()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skiptaskbar", true);
    group.writeEntry("skiptaskbarrule", int(Rules::Force));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should not be included on a taskbar.
    QVERIFY(client->skipTaskbar());

    // Any attempt to change the skip-taskbar state should not succeed.
    client->setOriginalSkipTaskbar(false);
    QVERIFY(client->skipTaskbar());

    // Reopen the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The skip-taskbar state should be still forced.
    QVERIFY(client->skipTaskbar());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSkipTaskbarApplyNow)

void TestShellClientRules::testSkipTaskbarApplyNow()
{
    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(!client->skipTaskbar());

    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skiptaskbar", true);
    group.writeEntry("skiptaskbarrule", int(Rules::ApplyNow));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // The client should not be on a taskbar now.
    QVERIFY(client->skipTaskbar());

    // Also, one change the skip-taskbar state.
    client->setOriginalSkipTaskbar(false);
    QVERIFY(!client->skipTaskbar());

    // The rule should not be applied again.
    client->evaluateWindowRules();
    QVERIFY(!client->skipTaskbar());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSkipTaskbarForceTemporarily)

void TestShellClientRules::testSkipTaskbarForceTemporarily()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skiptaskbar", true);
    group.writeEntry("skiptaskbarrule", int(Rules::ForceTemporarily));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should not be included on a taskbar.
    QVERIFY(client->skipTaskbar());

    // Any attempt to change the skip-taskbar state should not succeed.
    client->setOriginalSkipTaskbar(false);
    QVERIFY(client->skipTaskbar());

    // The rule should be discarded when the client is closed.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(!client->skipTaskbar());

    // The skip-taskbar state is no longer forced.
    client->setOriginalSkipTaskbar(true);
    QVERIFY(client->skipTaskbar());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSkipPagerDontAffect)

void TestShellClientRules::testSkipPagerDontAffect()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skippager", true);
    group.writeEntry("skippagerrule", int(Rules::DontAffect));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should not be affected by the rule.
    QVERIFY(!client->skipPager());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSkipPagerApply)

void TestShellClientRules::testSkipPagerApply()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skippager", true);
    group.writeEntry("skippagerrule", int(Rules::Apply));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should not be included on a pager.
    QVERIFY(client->skipPager());

    // Though one can change that.
    client->setSkipPager(false);
    QVERIFY(!client->skipPager());

    // Reopen the client, the rule should be applied again.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->skipPager());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSkipPagerRemember)

void TestShellClientRules::testSkipPagerRemember()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skippager", true);
    group.writeEntry("skippagerrule", int(Rules::Remember));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should not be included on a pager.
    QVERIFY(client->skipPager());

    // Change the skip-pager state.
    client->setSkipPager(false);
    QVERIFY(!client->skipPager());

    // Reopen the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should be included on a pager.
    QVERIFY(!client->skipPager());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSkipPagerForce)

void TestShellClientRules::testSkipPagerForce()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skippager", true);
    group.writeEntry("skippagerrule", int(Rules::Force));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should not be included on a pager.
    QVERIFY(client->skipPager());

    // Any attempt to change the skip-pager state should not succeed.
    client->setSkipPager(false);
    QVERIFY(client->skipPager());

    // Reopen the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The skip-pager state should be still forced.
    QVERIFY(client->skipPager());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSkipPagerApplyNow)

void TestShellClientRules::testSkipPagerApplyNow()
{
    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(!client->skipPager());

    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skippager", true);
    group.writeEntry("skippagerrule", int(Rules::ApplyNow));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // The client should not be on a pager now.
    QVERIFY(client->skipPager());

    // Also, one change the skip-pager state.
    client->setSkipPager(false);
    QVERIFY(!client->skipPager());

    // The rule should not be applied again.
    client->evaluateWindowRules();
    QVERIFY(!client->skipPager());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSkipPagerForceTemporarily)

void TestShellClientRules::testSkipPagerForceTemporarily()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skippager", true);
    group.writeEntry("skippagerrule", int(Rules::ForceTemporarily));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should not be included on a pager.
    QVERIFY(client->skipPager());

    // Any attempt to change the skip-pager state should not succeed.
    client->setSkipPager(false);
    QVERIFY(client->skipPager());

    // The rule should be discarded when the client is closed.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(!client->skipPager());

    // The skip-pager state is no longer forced.
    client->setSkipPager(true);
    QVERIFY(client->skipPager());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSkipSwitcherDontAffect)

void TestShellClientRules::testSkipSwitcherDontAffect()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skipswitcher", true);
    group.writeEntry("skipswitcherrule", int(Rules::DontAffect));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should not be affected by the rule.
    QVERIFY(!client->skipSwitcher());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSkipSwitcherApply)

void TestShellClientRules::testSkipSwitcherApply()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skipswitcher", true);
    group.writeEntry("skipswitcherrule", int(Rules::Apply));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should be excluded from window switching effects.
    QVERIFY(client->skipSwitcher());

    // Though one can change that.
    client->setSkipSwitcher(false);
    QVERIFY(!client->skipSwitcher());

    // Reopen the client, the rule should be applied again.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->skipSwitcher());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSkipSwitcherRemember)

void TestShellClientRules::testSkipSwitcherRemember()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skipswitcher", true);
    group.writeEntry("skipswitcherrule", int(Rules::Remember));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should be excluded from window switching effects.
    QVERIFY(client->skipSwitcher());

    // Change the skip-switcher state.
    client->setSkipSwitcher(false);
    QVERIFY(!client->skipSwitcher());

    // Reopen the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should be included in window switching effects.
    QVERIFY(!client->skipSwitcher());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSkipSwitcherForce)

void TestShellClientRules::testSkipSwitcherForce()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skipswitcher", true);
    group.writeEntry("skipswitcherrule", int(Rules::Force));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should be excluded from window switching effects.
    QVERIFY(client->skipSwitcher());

    // Any attempt to change the skip-switcher state should not succeed.
    client->setSkipSwitcher(false);
    QVERIFY(client->skipSwitcher());

    // Reopen the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The skip-switcher state should be still forced.
    QVERIFY(client->skipSwitcher());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSkipSwitcherApplyNow)

void TestShellClientRules::testSkipSwitcherApplyNow()
{
    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(!client->skipSwitcher());

    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skipswitcher", true);
    group.writeEntry("skipswitcherrule", int(Rules::ApplyNow));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // The client should be excluded from window switching effects now.
    QVERIFY(client->skipSwitcher());

    // Also, one change the skip-switcher state.
    client->setSkipSwitcher(false);
    QVERIFY(!client->skipSwitcher());

    // The rule should not be applied again.
    client->evaluateWindowRules();
    QVERIFY(!client->skipSwitcher());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSkipSwitcherForceTemporarily)

void TestShellClientRules::testSkipSwitcherForceTemporarily()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skipswitcher", true);
    group.writeEntry("skipswitcherrule", int(Rules::ForceTemporarily));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should be excluded from window switching effects.
    QVERIFY(client->skipSwitcher());

    // Any attempt to change the skip-switcher state should not succeed.
    client->setSkipSwitcher(false);
    QVERIFY(client->skipSwitcher());

    // The rule should be discarded when the client is closed.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(!client->skipSwitcher());

    // The skip-switcher state is no longer forced.
    client->setSkipSwitcher(true);
    QVERIFY(client->skipSwitcher());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testKeepAboveDontAffect)

void TestShellClientRules::testKeepAboveDontAffect()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("above", true);
    group.writeEntry("aboverule", int(Rules::DontAffect));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The keep-above state of the client should not be affected by the rule.
    QVERIFY(!client->keepAbove());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testKeepAboveApply)

void TestShellClientRules::testKeepAboveApply()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("above", true);
    group.writeEntry("aboverule", int(Rules::Apply));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // Initially, the client should be kept above.
    QVERIFY(client->keepAbove());

    // One should also be able to alter the keep-above state.
    client->setKeepAbove(false);
    QVERIFY(!client->keepAbove());

    // If one re-opens the client, it should be kept above back again.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->keepAbove());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testKeepAboveRemember)

void TestShellClientRules::testKeepAboveRemember()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("above", true);
    group.writeEntry("aboverule", int(Rules::Remember));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // Initially, the client should be kept above.
    QVERIFY(client->keepAbove());

    // Unset the keep-above state.
    client->setKeepAbove(false);
    QVERIFY(!client->keepAbove());
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));

    // Re-open the client, it should not be kept above.
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(!client->keepAbove());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testKeepAboveForce)

void TestShellClientRules::testKeepAboveForce()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("above", true);
    group.writeEntry("aboverule", int(Rules::Force));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // Initially, the client should be kept above.
    QVERIFY(client->keepAbove());

    // Any attemt to unset the keep-above should not succeed.
    client->setKeepAbove(false);
    QVERIFY(client->keepAbove());

    // If we re-open the client, it should still be kept above.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->keepAbove());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testKeepAboveApplyNow)

void TestShellClientRules::testKeepAboveApplyNow()
{
    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(!client->keepAbove());

    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("above", true);
    group.writeEntry("aboverule", int(Rules::ApplyNow));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // The client should now be kept above other clients.
    QVERIFY(client->keepAbove());

    // One is still able to change the keep-above state of the client.
    client->setKeepAbove(false);
    QVERIFY(!client->keepAbove());

    // The rule should not be applied again.
    client->evaluateWindowRules();
    QVERIFY(!client->keepAbove());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testKeepAboveForceTemporarily)

void TestShellClientRules::testKeepAboveForceTemporarily()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("above", true);
    group.writeEntry("aboverule", int(Rules::ForceTemporarily));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // Initially, the client should be kept above.
    QVERIFY(client->keepAbove());

    // Any attempt to alter the keep-above state should not succeed.
    client->setKeepAbove(false);
    QVERIFY(client->keepAbove());

    // The rule should be discarded when the client is closed.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(!client->keepAbove());

    // The keep-above state is no longer forced.
    client->setKeepAbove(true);
    QVERIFY(client->keepAbove());
    client->setKeepAbove(false);
    QVERIFY(!client->keepAbove());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testKeepBelowDontAffect)

void TestShellClientRules::testKeepBelowDontAffect()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("below", true);
    group.writeEntry("belowrule", int(Rules::DontAffect));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The keep-below state of the client should not be affected by the rule.
    QVERIFY(!client->keepBelow());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testKeepBelowApply)

void TestShellClientRules::testKeepBelowApply()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("below", true);
    group.writeEntry("belowrule", int(Rules::Apply));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // Initially, the client should be kept below.
    QVERIFY(client->keepBelow());

    // One should also be able to alter the keep-below state.
    client->setKeepBelow(false);
    QVERIFY(!client->keepBelow());

    // If one re-opens the client, it should be kept above back again.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->keepBelow());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testKeepBelowRemember)

void TestShellClientRules::testKeepBelowRemember()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("below", true);
    group.writeEntry("belowrule", int(Rules::Remember));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // Initially, the client should be kept below.
    QVERIFY(client->keepBelow());

    // Unset the keep-below state.
    client->setKeepBelow(false);
    QVERIFY(!client->keepBelow());
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));

    // Re-open the client, it should not be kept below.
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(!client->keepBelow());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testKeepBelowForce)

void TestShellClientRules::testKeepBelowForce()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("below", true);
    group.writeEntry("belowrule", int(Rules::Force));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // Initially, the client should be kept below.
    QVERIFY(client->keepBelow());

    // Any attemt to unset the keep-below should not succeed.
    client->setKeepBelow(false);
    QVERIFY(client->keepBelow());

    // If we re-open the client, it should still be kept below.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->keepBelow());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testKeepBelowApplyNow)

void TestShellClientRules::testKeepBelowApplyNow()
{
    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(!client->keepBelow());

    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("below", true);
    group.writeEntry("belowrule", int(Rules::ApplyNow));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // The client should now be kept below other clients.
    QVERIFY(client->keepBelow());

    // One is still able to change the keep-below state of the client.
    client->setKeepBelow(false);
    QVERIFY(!client->keepBelow());

    // The rule should not be applied again.
    client->evaluateWindowRules();
    QVERIFY(!client->keepBelow());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testKeepBelowForceTemporarily)

void TestShellClientRules::testKeepBelowForceTemporarily()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("below", true);
    group.writeEntry("belowrule", int(Rules::ForceTemporarily));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // Initially, the client should be kept below.
    QVERIFY(client->keepBelow());

    // Any attempt to alter the keep-below state should not succeed.
    client->setKeepBelow(false);
    QVERIFY(client->keepBelow());

    // The rule should be discarded when the client is closed.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(!client->keepBelow());

    // The keep-below state is no longer forced.
    client->setKeepBelow(true);
    QVERIFY(client->keepBelow());
    client->setKeepBelow(false);
    QVERIFY(!client->keepBelow());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testShortcutDontAffect)

void TestShellClientRules::testShortcutDontAffect()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("shortcut", "Ctrl+Alt+1");
    group.writeEntry("shortcutrule", int(Rules::DontAffect));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QCOMPARE(client->shortcut(), QKeySequence());
    client->minimize();
    QVERIFY(client->isMinimized());

    // If we press the window shortcut, nothing should happen.
    QSignalSpy clientUnminimizedSpy(client, &AbstractClient::clientUnminimized);
    QVERIFY(clientUnminimizedSpy.isValid());
    quint32 timestamp = 1;
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTCTRL, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_1, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_1, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTCTRL, timestamp++);
    QVERIFY(!clientUnminimizedSpy.wait(100));
    QVERIFY(client->isMinimized());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testShortcutApply)

void TestShellClientRules::testShortcutApply()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("shortcut", "Ctrl+Alt+1");
    group.writeEntry("shortcutrule", int(Rules::Apply));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // If we press the window shortcut, the window should be brought back to user.
    QSignalSpy clientUnminimizedSpy(client, &AbstractClient::clientUnminimized);
    QVERIFY(clientUnminimizedSpy.isValid());
    quint32 timestamp = 1;
    QCOMPARE(client->shortcut(), (QKeySequence{Qt::CTRL + Qt::ALT + Qt::Key_1}));
    client->minimize();
    QVERIFY(client->isMinimized());
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTCTRL, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_1, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_1, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTCTRL, timestamp++);
    QVERIFY(clientUnminimizedSpy.wait());
    QVERIFY(!client->isMinimized());

    // One can also change the shortcut.
    client->setShortcut(QStringLiteral("Ctrl+Alt+2"));
    QCOMPARE(client->shortcut(), (QKeySequence{Qt::CTRL + Qt::ALT + Qt::Key_2}));
    client->minimize();
    QVERIFY(client->isMinimized());
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTCTRL, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_2, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_2, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTCTRL, timestamp++);
    QVERIFY(clientUnminimizedSpy.wait());
    QVERIFY(!client->isMinimized());

    // The old shortcut should do nothing.
    client->minimize();
    QVERIFY(client->isMinimized());
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTCTRL, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_1, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_1, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTCTRL, timestamp++);
    QVERIFY(!clientUnminimizedSpy.wait(100));
    QVERIFY(client->isMinimized());

    // Reopen the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The window shortcut should be set back to Ctrl+Alt+1.
    QCOMPARE(client->shortcut(), (QKeySequence{Qt::CTRL + Qt::ALT + Qt::Key_1}));

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testShortcutRemember)

void TestShellClientRules::testShortcutRemember()
{
    QSKIP("KWin core doesn't try to save the last used window shortcut");

    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("shortcut", "Ctrl+Alt+1");
    group.writeEntry("shortcutrule", int(Rules::Remember));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // If we press the window shortcut, the window should be brought back to user.
    QSignalSpy clientUnminimizedSpy(client, &AbstractClient::clientUnminimized);
    QVERIFY(clientUnminimizedSpy.isValid());
    quint32 timestamp = 1;
    QCOMPARE(client->shortcut(), (QKeySequence{Qt::CTRL + Qt::ALT + Qt::Key_1}));
    client->minimize();
    QVERIFY(client->isMinimized());
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTCTRL, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_1, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_1, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTCTRL, timestamp++);
    QVERIFY(clientUnminimizedSpy.wait());
    QVERIFY(!client->isMinimized());

    // Change the window shortcut to Ctrl+Alt+2.
    client->setShortcut(QStringLiteral("Ctrl+Alt+2"));
    QCOMPARE(client->shortcut(), (QKeySequence{Qt::CTRL + Qt::ALT + Qt::Key_2}));
    client->minimize();
    QVERIFY(client->isMinimized());
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTCTRL, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_2, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_2, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTCTRL, timestamp++);
    QVERIFY(clientUnminimizedSpy.wait());
    QVERIFY(!client->isMinimized());

    // Reopen the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The window shortcut should be set to the last known value.
    QCOMPARE(client->shortcut(), (QKeySequence{Qt::CTRL + Qt::ALT + Qt::Key_2}));

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testShortcutForce)

void TestShellClientRules::testShortcutForce()
{
    QSKIP("KWin core can't release forced window shortcuts");

    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("shortcut", "Ctrl+Alt+1");
    group.writeEntry("shortcutrule", int(Rules::Force));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // If we press the window shortcut, the window should be brought back to user.
    QSignalSpy clientUnminimizedSpy(client, &AbstractClient::clientUnminimized);
    QVERIFY(clientUnminimizedSpy.isValid());
    quint32 timestamp = 1;
    QCOMPARE(client->shortcut(), (QKeySequence{Qt::CTRL + Qt::ALT + Qt::Key_1}));
    client->minimize();
    QVERIFY(client->isMinimized());
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTCTRL, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_1, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_1, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTCTRL, timestamp++);
    QVERIFY(clientUnminimizedSpy.wait());
    QVERIFY(!client->isMinimized());

    // Any attempt to change the window shortcut should not succeed.
    client->setShortcut(QStringLiteral("Ctrl+Alt+2"));
    QCOMPARE(client->shortcut(), (QKeySequence{Qt::CTRL + Qt::ALT + Qt::Key_1}));
    client->minimize();
    QVERIFY(client->isMinimized());
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTCTRL, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_2, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_2, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTCTRL, timestamp++);
    QVERIFY(!clientUnminimizedSpy.wait(100));
    QVERIFY(client->isMinimized());

    // Reopen the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The window shortcut should still be forced.
    QCOMPARE(client->shortcut(), (QKeySequence{Qt::CTRL + Qt::ALT + Qt::Key_1}));

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testShortcutApplyNow)

void TestShellClientRules::testShortcutApplyNow()
{
    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->shortcut().isEmpty());

    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("shortcut", "Ctrl+Alt+1");
    group.writeEntry("shortcutrule", int(Rules::ApplyNow));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // The client should now have a window shortcut assigned.
    QCOMPARE(client->shortcut(), (QKeySequence{Qt::CTRL + Qt::ALT + Qt::Key_1}));
    QSignalSpy clientUnminimizedSpy(client, &AbstractClient::clientUnminimized);
    QVERIFY(clientUnminimizedSpy.isValid());
    quint32 timestamp = 1;
    client->minimize();
    QVERIFY(client->isMinimized());
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTCTRL, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_1, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_1, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTCTRL, timestamp++);
    QVERIFY(clientUnminimizedSpy.wait());
    QVERIFY(!client->isMinimized());

    // Assign a different shortcut.
    client->setShortcut(QStringLiteral("Ctrl+Alt+2"));
    QCOMPARE(client->shortcut(), (QKeySequence{Qt::CTRL + Qt::ALT + Qt::Key_2}));
    client->minimize();
    QVERIFY(client->isMinimized());
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTCTRL, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_2, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_2, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTCTRL, timestamp++);
    QVERIFY(clientUnminimizedSpy.wait());
    QVERIFY(!client->isMinimized());

    // The rule should not be applied again.
    client->evaluateWindowRules();
    QCOMPARE(client->shortcut(), (QKeySequence{Qt::CTRL + Qt::ALT + Qt::Key_2}));

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testShortcutForceTemporarily)

void TestShellClientRules::testShortcutForceTemporarily()
{
    QSKIP("KWin core can't release forced window shortcuts");

    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("shortcut", "Ctrl+Alt+1");
    group.writeEntry("shortcutrule", int(Rules::ForceTemporarily));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // If we press the window shortcut, the window should be brought back to user.
    QSignalSpy clientUnminimizedSpy(client, &AbstractClient::clientUnminimized);
    QVERIFY(clientUnminimizedSpy.isValid());
    quint32 timestamp = 1;
    QCOMPARE(client->shortcut(), (QKeySequence{Qt::CTRL + Qt::ALT + Qt::Key_1}));
    client->minimize();
    QVERIFY(client->isMinimized());
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTCTRL, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_1, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_1, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTCTRL, timestamp++);
    QVERIFY(clientUnminimizedSpy.wait());
    QVERIFY(!client->isMinimized());

    // Any attempt to change the window shortcut should not succeed.
    client->setShortcut(QStringLiteral("Ctrl+Alt+2"));
    QCOMPARE(client->shortcut(), (QKeySequence{Qt::CTRL + Qt::ALT + Qt::Key_1}));
    client->minimize();
    QVERIFY(client->isMinimized());
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTCTRL, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyPressed(KEY_2, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_2, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTALT, timestamp++);
    kwinApp()->platform()->keyboardKeyReleased(KEY_LEFTCTRL, timestamp++);
    QVERIFY(!clientUnminimizedSpy.wait(100));
    QVERIFY(client->isMinimized());

    // The rule should be discarded when the client is closed.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->shortcut().isEmpty());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testDesktopFileDontAffect)

void TestShellClientRules::testDesktopFileDontAffect()
{
    // Currently, the desktop file name is derived from the app id. If the app id is
    // changed, then the old rules will be lost. Either setDesktopFileName should
    // be exposed or the desktop file name rule should be removed for wayland clients.
    QSKIP("Needs changes in KWin core to pass");
}

TEST_DATA(testDesktopFileApply)

void TestShellClientRules::testDesktopFileApply()
{
    // Currently, the desktop file name is derived from the app id. If the app id is
    // changed, then the old rules will be lost. Either setDesktopFileName should
    // be exposed or the desktop file name rule should be removed for wayland clients.
    QSKIP("Needs changes in KWin core to pass");
}

TEST_DATA(testDesktopFileRemember)

void TestShellClientRules::testDesktopFileRemember()
{
    // Currently, the desktop file name is derived from the app id. If the app id is
    // changed, then the old rules will be lost. Either setDesktopFileName should
    // be exposed or the desktop file name rule should be removed for wayland clients.
    QSKIP("Needs changes in KWin core to pass");
}

TEST_DATA(testDesktopFileForce)

void TestShellClientRules::testDesktopFileForce()
{
    // Currently, the desktop file name is derived from the app id. If the app id is
    // changed, then the old rules will be lost. Either setDesktopFileName should
    // be exposed or the desktop file name rule should be removed for wayland clients.
    QSKIP("Needs changes in KWin core to pass");
}

TEST_DATA(testDesktopFileApplyNow)

void TestShellClientRules::testDesktopFileApplyNow()
{
    // Currently, the desktop file name is derived from the app id. If the app id is
    // changed, then the old rules will be lost. Either setDesktopFileName should
    // be exposed or the desktop file name rule should be removed for wayland clients.
    QSKIP("Needs changes in KWin core to pass");
}

TEST_DATA(testDesktopFileForceTemporarily)

void TestShellClientRules::testDesktopFileForceTemporarily()
{
    // Currently, the desktop file name is derived from the app id. If the app id is
    // changed, then the old rules will be lost. Either setDesktopFileName should
    // be exposed or the desktop file name rule should be removed for wayland clients.
    QSKIP("Needs changes in KWin core to pass");
}

TEST_DATA(testActiveOpacityDontAffect)

void TestShellClientRules::testActiveOpacityDontAffect()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("opacityactive", 90);
    group.writeEntry("opacityactiverule", int(Rules::DontAffect));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());

    // The opacity should not be affected by the rule.
    QCOMPARE(client->opacity(), 1.0);

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testActiveOpacityForce)

void TestShellClientRules::testActiveOpacityForce()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("opacityactive", 90);
    group.writeEntry("opacityactiverule", int(Rules::Force));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());
    QCOMPARE(client->opacity(), 0.9);

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testActiveOpacityForceTemporarily)

void TestShellClientRules::testActiveOpacityForceTemporarily()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("opacityactive", 90);
    group.writeEntry("opacityactiverule", int(Rules::ForceTemporarily));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());
    QCOMPARE(client->opacity(), 0.9);

    // The rule should be discarded when the client is closed.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());
    QCOMPARE(client->opacity(), 1.0);

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testInactiveOpacityDontAffect)

void TestShellClientRules::testInactiveOpacityDontAffect()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("opacityinactive", 80);
    group.writeEntry("opacityinactiverule", int(Rules::DontAffect));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());

    // Make the client inactive.
    workspace()->setActiveClient(nullptr);
    QVERIFY(!client->isActive());

    // The opacity of the client should not be affected by the rule.
    QCOMPARE(client->opacity(), 1.0);

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testInactiveOpacityForce)

void TestShellClientRules::testInactiveOpacityForce()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("opacityinactive", 80);
    group.writeEntry("opacityinactiverule", int(Rules::Force));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());
    QCOMPARE(client->opacity(), 1.0);

    // Make the client inactive.
    workspace()->setActiveClient(nullptr);
    QVERIFY(!client->isActive());

    // The opacity should be forced by the rule.
    QCOMPARE(client->opacity(), 0.8);

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testInactiveOpacityForceTemporarily)

void TestShellClientRules::testInactiveOpacityForceTemporarily()
{
    // Initialize RuleBook with the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("opacityinactive", 80);
    group.writeEntry("opacityinactiverule", int(Rules::ForceTemporarily));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    XdgShellSurface *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());
    QCOMPARE(client->opacity(), 1.0);

    // Make the client inactive.
    workspace()->setActiveClient(nullptr);
    QVERIFY(!client->isActive());

    // The opacity should be forced by the rule.
    QCOMPARE(client->opacity(), 0.8);

    // The rule should be discarded when the client is closed.
    delete shellSurface;
    delete surface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());
    QCOMPARE(client->opacity(), 1.0);
    workspace()->setActiveClient(nullptr);
    QVERIFY(!client->isActive());
    QCOMPARE(client->opacity(), 1.0);

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

void TestShellClientRules::testMatchAfterNameChange()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);

    auto group = config->group("1");
    group.writeEntry("above", true);
    group.writeEntry("aboverule", int(Rules::Force));
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", int(Rules::ExactMatch));
    group.sync();

    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    QScopedPointer<Surface> surface(Test::createSurface());
    QScopedPointer<XdgShellSurface> shellSurface(Test::createXdgShellV6Surface(surface.data()));

    auto c = Test::renderAndWaitForShown(surface.data(), QSize(100, 50), Qt::blue);
    QVERIFY(c);
    QVERIFY(c->isActive());
    QCOMPARE(c->keepAbove(), false);

    QSignalSpy desktopFileNameSpy(c, &AbstractClient::desktopFileNameChanged);
    QVERIFY(desktopFileNameSpy.isValid());

    shellSurface->setAppId(QByteArrayLiteral("org.kde.foo"));
    QVERIFY(desktopFileNameSpy.wait());
    QCOMPARE(c->keepAbove(), true);
}

WAYLANDTEST_MAIN(TestShellClientRules)
#include "shell_client_rules_test.moc"
