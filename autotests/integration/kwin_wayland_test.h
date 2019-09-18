/********************************************************************
KWin - the KDE window manager
This file is part of the KDE project.

Copyright (C) 2015 Martin Gräßlin <mgraesslin@kde.org>

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
#ifndef KWIN_WAYLAND_TEST_H
#define KWIN_WAYLAND_TEST_H

#include "../../main.h"

// Qt
#include <QtTest>

// KWayland
#include <KWayland/Client/xdgshell.h>

namespace KWayland
{
namespace Client
{
class AppMenuManager;
class ConnectionThread;
class Compositor;
class IdleInhibitManager;
class PlasmaShell;
class PlasmaWindowManagement;
class PointerConstraints;
class Seat;
class ServerSideDecorationManager;
class ShadowManager;
class ShmPool;
class SubCompositor;
class SubSurface;
class Surface;
class XdgDecorationManager;
}
}

namespace KWin
{
namespace Xwl
{
class Xwayland;
}

class AbstractClient;
class ShellClient;

class WaylandTestApplication : public ApplicationWaylandAbstract
{
    Q_OBJECT
public:
    WaylandTestApplication(OperationMode mode, int &argc, char **argv);
    ~WaylandTestApplication() override;

protected:
    void performStartup() override;

private:
    void createBackend();
    void continueStartupWithScreens();
    void continueStartupWithScene();
    void finalizeStartup();

    Xwl::Xwayland *m_xwayland = nullptr;
};

namespace Test
{

enum class AdditionalWaylandInterface {
    Seat = 1 << 0,
    Decoration = 1 << 1,
    PlasmaShell = 1 << 2,
    WindowManagement = 1 << 3,
    PointerConstraints = 1 << 4,
    IdleInhibition = 1 << 5,
    AppMenu = 1 << 6,
    ShadowManager = 1 << 7,
    XdgDecoration = 1 << 8,
};
Q_DECLARE_FLAGS(AdditionalWaylandInterfaces, AdditionalWaylandInterface)
/**
 * Creates a Wayland Connection in a dedicated thread and creates various
 * client side objects which can be used to create windows.
 * @returns @c true if created successfully, @c false if there was an error
 * @see destroyWaylandConnection
 */
bool setupWaylandConnection(AdditionalWaylandInterfaces flags = AdditionalWaylandInterfaces());

/**
 * Destroys the Wayland Connection created with @link{setupWaylandConnection}.
 * This can be called from cleanup in order to ensure that no Wayland Connection
 * leaks into the next test method.
 * @see setupWaylandConnection
 */
void destroyWaylandConnection();

KWayland::Client::ConnectionThread *waylandConnection();
KWayland::Client::Compositor *waylandCompositor();
KWayland::Client::SubCompositor *waylandSubCompositor();
KWayland::Client::ShadowManager *waylandShadowManager();
KWayland::Client::ShmPool *waylandShmPool();
KWayland::Client::Seat *waylandSeat();
KWayland::Client::ServerSideDecorationManager *waylandServerSideDecoration();
KWayland::Client::PlasmaShell *waylandPlasmaShell();
KWayland::Client::PlasmaWindowManagement *waylandWindowManagement();
KWayland::Client::PointerConstraints *waylandPointerConstraints();
KWayland::Client::IdleInhibitManager *waylandIdleInhibitManager();
KWayland::Client::AppMenuManager *waylandAppMenuManager();
KWayland::Client::XdgDecorationManager *xdgDecorationManager();

bool waitForWaylandPointer();
bool waitForWaylandTouch();
bool waitForWaylandKeyboard();

void flushWaylandConnection();

KWayland::Client::Surface *createSurface(QObject *parent = nullptr);
KWayland::Client::SubSurface *createSubSurface(KWayland::Client::Surface *surface,
                                               KWayland::Client::Surface *parentSurface, QObject *parent = nullptr);
enum class XdgShellSurfaceType {
    XdgShellV5,
    XdgShellV6,
    XdgShellStable
};

enum class CreationSetup {
    CreateOnly,
    CreateAndConfigure, /// commit and wait for the configure event, making this surface ready to commit buffers
};

KWayland::Client::XdgShellSurface *createXdgShellSurface(XdgShellSurfaceType type,
                                                         KWayland::Client::Surface *surface,
                                                         QObject *parent = nullptr,
                                                         CreationSetup creationSetup = CreationSetup::CreateAndConfigure);

KWayland::Client::XdgShellSurface *createXdgShellV5Surface(KWayland::Client::Surface *surface,
                                                           QObject *parent = nullptr,
                                                           CreationSetup = CreationSetup::CreateAndConfigure);
KWayland::Client::XdgShellSurface *createXdgShellV6Surface(KWayland::Client::Surface *surface,
                                                           QObject *parent = nullptr,
                                                           CreationSetup = CreationSetup::CreateAndConfigure);
KWayland::Client::XdgShellSurface *createXdgShellStableSurface(KWayland::Client::Surface *surface,
                                                               QObject *parent = nullptr,
                                                               CreationSetup = CreationSetup::CreateAndConfigure);
KWayland::Client::XdgShellPopup *createXdgShellStablePopup(KWayland::Client::Surface *surface,
                                                           KWayland::Client::XdgShellSurface *parentSurface,
                                                           const KWayland::Client::XdgPositioner &positioner,
                                                           QObject *parent = nullptr,
                                                           CreationSetup = CreationSetup::CreateAndConfigure);


/**
 * Commits the XdgShellSurface to the given surface, and waits for the configure event from the compositor
 */
void initXdgShellSurface(KWayland::Client::Surface *surface, KWayland::Client::XdgShellSurface *shellSurface);
void initXdgShellPopup(KWayland::Client::Surface *surface, KWayland::Client::XdgShellPopup *popup);



/**
 * Creates a shared memory buffer of @p size in @p color and attaches it to the @p surface.
 * The @p surface gets damaged and committed, thus it's rendered.
 */
void render(KWayland::Client::Surface *surface, const QSize &size, const QColor &color, const QImage::Format &format = QImage::Format_ARGB32_Premultiplied);

/**
 * Creates a shared memory buffer using the supplied image @p img and attaches it to the @p surface
 */
void render(KWayland::Client::Surface *surface, const QImage &img);

/**
 * Waits till a new ShellClient is shown and returns the created ShellClient.
 * If no ShellClient gets shown during @p timeout @c null is returned.
 */
ShellClient *waitForWaylandWindowShown(int timeout = 5000);

/**
 * Combination of @link{render} and @link{waitForWaylandWindowShown}.
 */
ShellClient *renderAndWaitForShown(KWayland::Client::Surface *surface, const QSize &size, const QColor &color, const QImage::Format &format = QImage::Format_ARGB32, int timeout = 5000);

/**
 * Waits for the @p client to be destroyed.
 */
bool waitForWindowDestroyed(AbstractClient *client);

/**
 * Locks the screen and waits till the screen is locked.
 * @returns @c true if the screen could be locked, @c false otherwise
 */
bool lockScreen();

/**
 * Unlocks the screen and waits till the screen is unlocked.
 * @returns @c true if the screen could be unlocked, @c false otherwise
 */
bool unlockScreen();
}

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KWin::Test::AdditionalWaylandInterfaces)
Q_DECLARE_METATYPE(KWin::Test::XdgShellSurfaceType)

#define WAYLANDTEST_MAIN_HELPER(TestObject, DPI, OperationMode) \
int main(int argc, char *argv[]) \
{ \
    setenv("QT_QPA_PLATFORM", "wayland-org.kde.kwin.qpa", true); \
    setenv("QT_QPA_PLATFORM_PLUGIN_PATH", QFileInfo(QString::fromLocal8Bit(argv[0])).absolutePath().toLocal8Bit().constData(), true); \
    setenv("KWIN_FORCE_OWN_QPA", "1", true); \
    qunsetenv("KDE_FULL_SESSION"); \
    qunsetenv("KDE_SESSION_VERSION"); \
    qunsetenv("XDG_SESSION_DESKTOP"); \
    qunsetenv("XDG_CURRENT_DESKTOP"); \
    DPI; \
    KWin::WaylandTestApplication app(OperationMode, argc, argv); \
    app.setAttribute(Qt::AA_Use96Dpi, true); \
    TestObject tc; \
    return QTest::qExec(&tc, argc, argv); \
}

#ifdef NO_XWAYLAND
#define WAYLANDTEST_MAIN(TestObject) WAYLANDTEST_MAIN_HELPER(TestObject, QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps), KWin::Application::OperationModeWaylandOnly)
#else
#define WAYLANDTEST_MAIN(TestObject) WAYLANDTEST_MAIN_HELPER(TestObject, QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps), KWin::Application::OperationModeXwayland)
#endif

#endif
