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
    QTest::newRow("WlShell") << Test::ShellSurfaceType::WlShell; \
    QTest::newRow("XdgShellV5") << Test::ShellSurfaceType::XdgShellV5; \
    QTest::newRow("XdgShellV6") << Test::ShellSurfaceType::XdgShellV6; \
    QTest::newRow("XdgWmBase") << Test::ShellSurfaceType::XdgShellStable; \
}

std::tuple<ShellClient *, Surface *, QObject *> createWindow(Test::ShellSurfaceType type, const QByteArray &appId)
{
    Surface *surface = Test::createSurface();
    QObject *shellSurface = Test::createShellSurface(type, surface);

    switch (type) {
    case Test::ShellSurfaceType::WlShell: {
        auto wlShellSurface = static_cast<ShellSurface *>(shellSurface);
        wlShellSurface->setWindowClass(appId);
        break;
    }
    case Test::ShellSurfaceType::XdgShellV5:
    case Test::ShellSurfaceType::XdgShellV6:
    case Test::ShellSurfaceType::XdgShellStable: {
        auto xdgShellSurface = static_cast<XdgShellSurface *>(shellSurface);
        xdgShellSurface->setAppId(appId);
        break;
    }
    }

    ShellClient *client = Test::renderAndWaitForShown(surface, QSize(100, 50), Qt::blue);

    return {client, surface, shellSurface};
}

TEST_DATA(testDesktopDontAffect)

void TestShellClientRules::testDesktopDontAffect()
{
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("desktop", 2);
    group.writeEntry("desktoprule", 1);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
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
    QObject *shellSurface;
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("desktop", 2);
    group.writeEntry("desktoprule", 3);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
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
    QObject *shellSurface;
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("desktop", 2);
    group.writeEntry("desktoprule", 4);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
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
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QCOMPARE(client->desktop(), 2);
    QCOMPARE(VirtualDesktopManager::self()->current(), 2);

    // Move the client to the first virtual desktop.
    workspace()->sendClientToDesktop(client, 1, true);
    QCOMPARE(client->desktop(), 1);
    QCOMPARE(VirtualDesktopManager::self()->current(), 2);
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));

    // If open the client again, it should appear on the first virtual desktop.
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("desktop", 2);
    group.writeEntry("desktoprule", 2);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
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
    QObject *shellSurface;
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
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QCOMPARE(client->desktop(), 1);
    QCOMPARE(VirtualDesktopManager::self()->current(), 1);

    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("desktop", 2);
    group.writeEntry("desktoprule", 5);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("desktop", 2);
    group.writeEntry("desktoprule", 6);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
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
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should appear on the second virtual desktop.
    QCOMPARE(client->desktop(), 2);
    QCOMPARE(VirtualDesktopManager::self()->current(), 2);

    // Any attempt to move the client to another virtual desktop should fail.
    workspace()->sendClientToDesktop(client, 1, true);
    QCOMPARE(client->desktop(), 2);
    QCOMPARE(VirtualDesktopManager::self()->current(), 2);

    // If we re-open the client, the desktop should no longer be forced.
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("minimize", true);
    group.writeEntry("minimizerule", 1);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("minimize", true);
    group.writeEntry("minimizerule", 3);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("minimize", false);
    group.writeEntry("minimizerule", 4);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("minimize", false);
    group.writeEntry("minimizerule", 2);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isMinimizable());
    QVERIFY(!client->isMinimized());

    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("minimize", true);
    group.writeEntry("minimizerule", 5);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("minimize", false);
    group.writeEntry("minimizerule", 6);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(!client->isMinimizable());
    QVERIFY(!client->isMinimized());

    // Any attempt to minimize the client should fail, for now.
    client->minimize();
    QVERIFY(!client->isMinimized());

    // If we re-open the client, one should be able to alter the minimized state.
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skiptaskbar", true);
    group.writeEntry("skiptaskbarrule", 1);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skiptaskbar", true);
    group.writeEntry("skiptaskbarrule", 3);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should not be included on a taskbar.
    QVERIFY(client->skipTaskbar());

    // Though one can change that.
    client->setSkipTaskbar(false);
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skiptaskbar", true);
    group.writeEntry("skiptaskbarrule", 4);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should not be included on a taskbar.
    QVERIFY(client->skipTaskbar());

    // Change the skip-taskbar state.
    client->setSkipTaskbar(false);
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skiptaskbar", true);
    group.writeEntry("skiptaskbarrule", 2);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should not be included on a taskbar.
    QVERIFY(client->skipTaskbar());

    // Any attempt to change the skip-taskbar state should not succeed.
    client->setSkipTaskbar(false);
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
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(!client->skipTaskbar());

    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skiptaskbar", true);
    group.writeEntry("skiptaskbarrule", 5);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // The client should not be on a taskbar now.
    QVERIFY(client->skipTaskbar());

    // Also, one change the skip-taskbar state.
    client->setSkipTaskbar(false);
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skiptaskbar", true);
    group.writeEntry("skiptaskbarrule", 6);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should not be included on a taskbar.
    QVERIFY(client->skipTaskbar());

    // Any attempt to change the skip-taskbar state should not succeed.
    client->setSkipTaskbar(false);
    QVERIFY(client->skipTaskbar());

    // Reopen the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should be included on a taskbar now.
    QVERIFY(!client->skipTaskbar());

    // Also, one can change that.
    client->setSkipTaskbar(true);
    QVERIFY(client->skipTaskbar());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testSkipPagerDontAffect)

void TestShellClientRules::testSkipPagerDontAffect()
{
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skippager", true);
    group.writeEntry("skippagerrule", 1);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skippager", true);
    group.writeEntry("skippagerrule", 3);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skippager", true);
    group.writeEntry("skippagerrule", 4);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skippager", true);
    group.writeEntry("skippagerrule", 2);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(!client->skipPager());

    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skippager", true);
    group.writeEntry("skippagerrule", 5);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skippager", true);
    group.writeEntry("skippagerrule", 6);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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

    // The client should be included on a pager now.
    QVERIFY(!client->skipPager());

    // Also, one can change that.
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skipswitcher", true);
    group.writeEntry("skipswitcherrule", 1);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skipswitcher", true);
    group.writeEntry("skipswitcherrule", 3);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skipswitcher", true);
    group.writeEntry("skipswitcherrule", 4);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skipswitcher", true);
    group.writeEntry("skipswitcherrule", 2);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(!client->skipSwitcher());

    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skipswitcher", true);
    group.writeEntry("skipswitcherrule", 5);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("skipswitcher", true);
    group.writeEntry("skipswitcherrule", 6);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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

    // The client should be included in window switching effects now.
    QVERIFY(!client->skipSwitcher());

    // Also, one can change that.
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("above", true);
    group.writeEntry("aboverule", 1);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should not be kept above.
    QVERIFY(!client->keepAbove());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testKeepAboveApply)

void TestShellClientRules::testKeepAboveApply()
{
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("above", true);
    group.writeEntry("aboverule", 3);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("above", true);
    group.writeEntry("aboverule", 4);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("above", true);
    group.writeEntry("aboverule", 2);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(!client->keepAbove());

    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("above", true);
    group.writeEntry("aboverule", 5);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("above", true);
    group.writeEntry("aboverule", 6);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // Initially, the client should be kept above.
    QVERIFY(client->keepAbove());

    // Any attempt to alter the keep-above state should not succeed.
    client->setKeepAbove(false);
    QVERIFY(client->keepAbove());

    // Re-open the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The keep-above state is no longer forced.
    QVERIFY(!client->keepAbove());

    // We should now be able to alter the keep-above state.
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("below", true);
    group.writeEntry("belowrule", 1);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The client should not be kept below.
    QVERIFY(!client->keepBelow());

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testKeepBelowApply)

void TestShellClientRules::testKeepBelowApply()
{
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("below", true);
    group.writeEntry("belowrule", 3);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("below", true);
    group.writeEntry("belowrule", 4);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("below", true);
    group.writeEntry("belowrule", 2);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(!client->keepBelow());

    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("below", true);
    group.writeEntry("belowrule", 5);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("below", true);
    group.writeEntry("belowrule", 6);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // Initially, the client should be kept below.
    QVERIFY(client->keepBelow());

    // Any attempt to alter the keep-below state should not succeed.
    client->setKeepBelow(false);
    QVERIFY(client->keepBelow());

    // Re-open the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);

    // The keep-below state is no longer forced.
    QVERIFY(!client->keepBelow());

    // We should now be able to alter the keep-below state.
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("shortcut", "Ctrl+Alt+1");
    group.writeEntry("shortcutrule", 1);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("shortcut", "Ctrl+Alt+1");
    group.writeEntry("shortcutrule", 3);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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

    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("shortcut", "Ctrl+Alt+1");
    group.writeEntry("shortcutrule", 4);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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

    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("shortcut", "Ctrl+Alt+1");
    group.writeEntry("shortcutrule", 2);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->shortcut().isEmpty());

    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("shortcut", "Ctrl+Alt+1");
    group.writeEntry("shortcutrule", 5);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
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

    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("shortcut", "Ctrl+Alt+1");
    group.writeEntry("shortcutrule", 6);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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

    // The window shortcut should no longer be forced.
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("opacityactive", 90);
    group.writeEntry("opacityactiverule", 1);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("opacityactive", 90);
    group.writeEntry("opacityactiverule", 2);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("opacityactive", 90);
    group.writeEntry("opacityactiverule", 6);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());
    QCOMPARE(client->opacity(), 0.9);

    // Re-open the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());

    // The opacity is no longer forced.
    QCOMPARE(client->opacity(), 1.0);

    // Destroy the client.
    delete shellSurface;
    delete surface;
    QVERIFY(Test::waitForWindowDestroyed(client));
}

TEST_DATA(testInactiveOpacityDontAffect)

void TestShellClientRules::testInactiveOpacityDontAffect()
{
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("opacityinactive", 80);
    group.writeEntry("opacityinactiverule", 1);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());

    // Make the surface inactive.
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("opacityinactive", 80);
    group.writeEntry("opacityinactiverule", 2);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
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
    // Set the test rule.
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    config->group("General").writeEntry("count", 1);
    auto group = config->group("1");
    group.writeEntry("opacityinactive", 80);
    group.writeEntry("opacityinactiverule", 6);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
    group.sync();
    RuleBook::self()->setConfig(config);
    workspace()->slotReconfigure();

    // Create the test client.
    QFETCH(Test::ShellSurfaceType, type);
    ShellClient *client;
    Surface *surface;
    QObject *shellSurface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());
    QCOMPARE(client->opacity(), 1.0);

    // Make the client inactive.
    workspace()->setActiveClient(nullptr);
    QVERIFY(!client->isActive());

    // The opacity should be forced by the rule.
    QCOMPARE(client->opacity(), 0.8);

    // Re-open the client.
    delete shellSurface;
    delete surface;
    std::tie(client, surface, shellSurface) = createWindow(type, "org.kde.foo");
    QVERIFY(client);
    QVERIFY(client->isActive());
    QCOMPARE(client->opacity(), 1.0);

    // Make the client inactive.
    workspace()->setActiveClient(nullptr);
    QVERIFY(!client->isActive());

    // The opacity is no longer forced.
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
    group.writeEntry("aboverule", 2);
    group.writeEntry("wmclass", "org.kde.foo");
    group.writeEntry("wmclasscomplete", false);
    group.writeEntry("wmclassmatch", 1);
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
