/*
 * Copyright (C) 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
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

#include "xdgshellv6client.h"
#include "deleted.h"
#include "screenedge.h"
#include "screens.h"
#include "subsurfacetreemonitor.h"
#include "wayland_server.h"
#include "workspace.h"

#ifdef KWIN_BUILD_TABBOX
#include "tabbox.h"
#endif

#include <KDecoration2/DecoratedClient>
#include <KDecoration2/Decoration>
#include <KWayland/Server/appmenu_interface.h>
#include <KWayland/Server/buffer_interface.h>
#include <KWayland/Server/output_interface.h>
#include <KWayland/Server/plasmashell_interface.h>
#include <KWayland/Server/seat_interface.h>
#include <KWayland/Server/server_decoration_interface.h>
#include <KWayland/Server/server_decoration_palette_interface.h>
#include <KWayland/Server/surface_interface.h>

using namespace KWayland::Server;

namespace KWin
{

// WARNING: Any changes made to this file may also need to be added to xdgshellclient.cpp

XdgSurfaceV6Configure::~XdgSurfaceV6Configure()
{
}

void XdgSurfaceV6Configure::setGeometry(const QRect &rect)
{
    m_geometry = rect;
}

QRect XdgSurfaceV6Configure::geometry() const
{
    return m_geometry;
}

void XdgSurfaceV6Configure::setSerial(quint32 serial)
{
    m_serial = serial;
}

quint32 XdgSurfaceV6Configure::serial() const
{
    return m_serial;
}

XdgSurfaceV6Client::XdgSurfaceV6Client(XdgSurfaceV6Interface *shellSurface)
    : ShellSurfaceClient(shellSurface->surface())
    , m_shellSurface(shellSurface)
    , m_configureTimer(new QTimer(this))
{
    setupCompositing();

    connect(shellSurface, &XdgSurfaceV6Interface::configureAcknowledged,
            this, &XdgSurfaceV6Client::handleConfigureAcknowledged);
    connect(shellSurface->surface(), &SurfaceInterface::committed,
            this, &XdgSurfaceV6Client::handleCommit);
    connect(shellSurface->surface(), &SurfaceInterface::shadowChanged,
            this, &XdgSurfaceV6Client::updateShadow);
    connect(shellSurface->surface(), &SurfaceInterface::unmapped,
            this, &XdgSurfaceV6Client::internalUnmap);
    connect(shellSurface->surface(), &SurfaceInterface::unbound,
            this, &XdgSurfaceV6Client::destroyClient);
    connect(shellSurface->surface(), &SurfaceInterface::destroyed,
            this, &XdgSurfaceV6Client::destroyClient);

    // The effective window geometry is determined by two things: (a) the rectangle that bounds
    // the main surface and all of its sub-surfaces, (b) the client-specified window geometry, if
    // any. If the client hasn't provided the window geometry, we fallback to the bounding sub-
    // surface rectangle. If the client has provided the window geometry, we intersect it with
    // the bounding rectangle and that will be the effective window geometry. It's worth to point
    // out that geometry updates do not occur that frequently, so we don't need to recompute the
    // bounding geometry every time the client commits the surface.

    SubSurfaceTreeMonitor *treeMonitor = new SubSurfaceTreeMonitor(surface(), this);

    connect(treeMonitor, &SubSurfaceTreeMonitor::subSurfaceAdded,
            this, &XdgSurfaceV6Client::setHaveNextWindowGeometry);
    connect(treeMonitor, &SubSurfaceTreeMonitor::subSurfaceRemoved,
            this, &XdgSurfaceV6Client::setHaveNextWindowGeometry);
    connect(treeMonitor, &SubSurfaceTreeMonitor::subSurfaceMoved,
            this, &XdgSurfaceV6Client::setHaveNextWindowGeometry);
    connect(treeMonitor, &SubSurfaceTreeMonitor::subSurfaceResized,
            this, &XdgSurfaceV6Client::setHaveNextWindowGeometry);
    connect(shellSurface, &XdgSurfaceV6Interface::windowGeometryChanged,
            this, &XdgSurfaceV6Client::setHaveNextWindowGeometry);
    connect(surface(), &SurfaceInterface::sizeChanged,
            this, &XdgSurfaceV6Client::setHaveNextWindowGeometry);

    // Configure events are not sent immediately, but rather scheduled to be sent when the event
    // loop is about to be idle. By doing this, we can avoid sending configure events that do
    // nothing, and implementation-wise, it's simpler.

    m_configureTimer->setSingleShot(true);
    connect(m_configureTimer, &QTimer::timeout, this, &XdgSurfaceV6Client::sendConfigure);

    // Unfortunately, AbstractClient::checkWorkspacePosition() operates on the geometry restore
    // so we need to initialize it with some reasonable value; otherwise bad things will happen
    // when we want to decorate the client or move the client to another screen. This is a hack.

    connect(this, &XdgSurfaceV6Client::frameGeometryChanged,
            this, &XdgSurfaceV6Client::updateGeometryRestoreHack);
}

XdgSurfaceV6Client::~XdgSurfaceV6Client()
{
    qDeleteAll(m_configureEvents);
}

QRect XdgSurfaceV6Client::requestedFrameGeometry() const
{
    return m_requestedFrameGeometry;
}

QPoint XdgSurfaceV6Client::requestedPos() const
{
    return m_requestedFrameGeometry.topLeft();
}

QRect XdgSurfaceV6Client::requestedClientGeometry() const
{
    return m_requestedClientGeometry;
}

/**
 * @todo When the client is not server-side decorated we probably need to resort to the
 * bounding geometry, i.e. the rectangle that bounds the main surface and sub-surfaces.
 */
QRect XdgSurfaceV6Client::inputGeometry() const
{
    return isDecorated() ? AbstractClient::inputGeometry() : bufferGeometry();
}

QRect XdgSurfaceV6Client::bufferGeometry() const
{
    return m_bufferGeometry;
}

QSize XdgSurfaceV6Client::requestedClientSize() const
{
    return requestedClientGeometry().size();
}

QRect XdgSurfaceV6Client::clientGeometry() const
{
    return m_clientGeometry;
}

QSize XdgSurfaceV6Client::clientSize() const
{
    return m_clientGeometry.size();
}

QMatrix4x4 XdgSurfaceV6Client::inputTransformation() const
{
    QMatrix4x4 transformation;
    transformation.translate(-m_bufferGeometry.x(), -m_bufferGeometry.y());
    return transformation;
}

XdgSurfaceV6Configure *XdgSurfaceV6Client::lastAcknowledgedConfigure() const
{
    return m_lastAcknowledgedConfigure;
}

void XdgSurfaceV6Client::scheduleConfigure()
{
    if (!isClosing()) {
        m_configureTimer->start();
    }
}

void XdgSurfaceV6Client::sendConfigure()
{
    XdgSurfaceV6Configure *configureEvent = sendRoleConfigure();

    if (configureEvent->geometry().isValid()) {
        m_configureEvents.append(configureEvent);
    } else {
        delete configureEvent;
    }
}

void XdgSurfaceV6Client::handleConfigureAcknowledged(quint32 serial)
{
    while (!m_configureEvents.isEmpty()) {
        if (serial < m_configureEvents.first()->serial()) {
            break;
        }
        delete m_lastAcknowledgedConfigure;
        m_lastAcknowledgedConfigure = m_configureEvents.takeFirst();
    }
}

void XdgSurfaceV6Client::handleCommit()
{
    if (!surface()->buffer()) {
        return;
    }

    if (haveNextWindowGeometry()) {
        handleNextWindowGeometry();
        resetHaveNextWindowGeometry();
    }

    handleRoleCommit();

    delete m_lastAcknowledgedConfigure;
    m_lastAcknowledgedConfigure = nullptr;

    internalMap();
    updateDepth();
}

void XdgSurfaceV6Client::handleRoleCommit()
{
}

void XdgSurfaceV6Client::handleNextWindowGeometry()
{
    const QRect boundingGeometry = surface()->boundingRect();

    // The effective window geometry is defined as the intersection of the window geometry
    // and the rectangle that bounds the main surface and all of its sub-surfaces. If the
    // client hasn't specified the window geometry, we must fallback to the bounding geometry.
    // Note that the xdg-shell spec is not clear about when exactly we have to clamp the
    // window geometry.

    m_windowGeometry = m_shellSurface->windowGeometry();
    if (m_windowGeometry.isValid()) {
        m_windowGeometry &= boundingGeometry;
    } else {
        m_windowGeometry = boundingGeometry;
    }

    if (m_windowGeometry.isEmpty()) {
        qCWarning(KWIN_CORE) << "Committed empty window geometry, dealing with a buggy client!";
    }

    QRect geometry(pos(), clientSizeToFrameSize(m_windowGeometry.size()));

    // We're not done yet. The xdg-shell spec allows clients to attach buffers smaller than
    // we asked. Normally, this is not a big deal, but when the client is being interactively
    // resized, it may cause the window contents to bounce. In order to counter this, we have
    // to "gravitate" the new geometry according to the current move-resize pointer mode so
    // the opposite window corner stays still.

    if (isMoveResize()) {
        geometry = adjustMoveResizeGeometry(geometry);
    } else if (lastAcknowledgedConfigure()) {
        geometry.moveTopLeft(lastAcknowledgedConfigure()->geometry().topLeft());
    }

    updateGeometry(geometry);

    if (isResize()) {
        performMoveResize();
    }
}

bool XdgSurfaceV6Client::haveNextWindowGeometry() const
{
    return m_haveNextWindowGeometry || m_lastAcknowledgedConfigure;
}

void XdgSurfaceV6Client::setHaveNextWindowGeometry()
{
    m_haveNextWindowGeometry = true;
}

void XdgSurfaceV6Client::resetHaveNextWindowGeometry()
{
    m_haveNextWindowGeometry = false;
}

QRect XdgSurfaceV6Client::adjustMoveResizeGeometry(const QRect &rect) const
{
    QRect geometry = rect;

    switch (moveResizePointerMode()) {
    case PositionTopLeft:
        geometry.moveRight(moveResizeGeometry().right());
        geometry.moveBottom(moveResizeGeometry().bottom());
        break;
    case PositionTop:
    case PositionTopRight:
        geometry.moveLeft(moveResizeGeometry().left());
        geometry.moveBottom(moveResizeGeometry().bottom());
        break;
    case PositionRight:
    case PositionBottomRight:
    case PositionBottom:
    case PositionCenter:
        geometry.moveLeft(moveResizeGeometry().left());
        geometry.moveTop(moveResizeGeometry().top());
        break;
    case PositionBottomLeft:
    case PositionLeft:
        geometry.moveRight(moveResizeGeometry().right());
        geometry.moveTop(moveResizeGeometry().top());
        break;
    }

    return geometry;
}

/**
 * Sets the frame geometry of the XdgSurfaceV6Client to @p x, @p y, @p w, and @p h.
 *
 * Because geometry updates are asynchronous on Wayland, there are no any guarantees that
 * the frame geometry will be changed immediately. We may need to send a configure event to
 * the client if the current window geometry size and the requested window geometry size
 * don't match. frameGeometryChanged() will be emitted when the requested frame geometry
 * has been applied.
 *
 * Notice that the client may attach a buffer smaller than the one in the configure event.
 */
void XdgSurfaceV6Client::setFrameGeometry(int x, int y, int w, int h, ForceGeometry_t force)
{
    m_requestedFrameGeometry = QRect(x, y, w, h);

    // XdgToplevelV6Client currently doesn't support shaded clients, but let's stick with
    // what X11Client does. Hopefully, one day we will be able to unify setFrameGeometry()
    // for all AbstractClient subclasses. It's going to be great!

    if (isShade()) {
        if (m_requestedFrameGeometry.height() == borderTop() + borderBottom()) {
            qCDebug(KWIN_CORE) << "Passed shaded frame geometry to setFrameGeometry()";
        } else {
            m_requestedClientGeometry = frameRectToClientRect(m_requestedFrameGeometry);
            m_requestedFrameGeometry.setHeight(borderTop() + borderBottom());
        }
    } else {
        m_requestedClientGeometry = frameRectToClientRect(m_requestedFrameGeometry);
    }

    if (areGeometryUpdatesBlocked()) {
        m_frameGeometry = m_requestedFrameGeometry;
        if (pendingGeometryUpdate() == PendingGeometryForced) {
            return;
        }
        if (force == ForceGeometrySet) {
            setPendingGeometryUpdate(PendingGeometryForced);
        } else {
            setPendingGeometryUpdate(PendingGeometryNormal);
        }
        return;
    }

    m_frameGeometry = frameGeometryBeforeUpdateBlocking();

    // Notice that the window geometry size of (0, 0) has special meaning to xdg shell clients.
    // It basically says "pick whatever size you think is the best, dawg."

    if (requestedClientSize() != clientSize()) {
        requestGeometry(requestedFrameGeometry());
    } else {
        updateGeometry(requestedFrameGeometry());
    }
}

void XdgSurfaceV6Client::move(int x, int y, ForceGeometry_t force)
{
    Q_ASSERT(pendingGeometryUpdate() == PendingGeometryNone || areGeometryUpdatesBlocked());
    QPoint p(x, y);
    if (!areGeometryUpdatesBlocked() && p != rules()->checkPosition(p)) {
        qCDebug(KWIN_CORE) << "forced position fail:" << p << ":" << rules()->checkPosition(p);
    }
    m_requestedFrameGeometry.moveTopLeft(p);
    m_requestedClientGeometry.moveTopLeft(framePosToClientPos(p));
    if (force == NormalGeometrySet && m_frameGeometry.topLeft() == p) {
        return;
    }
    m_frameGeometry.moveTopLeft(m_requestedFrameGeometry.topLeft());
    if (areGeometryUpdatesBlocked()) {
        if (pendingGeometryUpdate() == PendingGeometryForced) {
            return;
        }
        if (force == ForceGeometrySet) {
            setPendingGeometryUpdate(PendingGeometryForced);
        } else {
            setPendingGeometryUpdate(PendingGeometryNormal);
        }
        return;
    }
    m_clientGeometry.moveTopLeft(m_requestedClientGeometry.topLeft());
    m_bufferGeometry = frameRectToBufferRect(m_frameGeometry);
    updateWindowRules(Rules::Position);
    screens()->setCurrent(this);
    workspace()->updateStackingOrder();
    emit frameGeometryChanged(this, frameGeometryBeforeUpdateBlocking());
    addRepaintDuringGeometryUpdates();
    updateGeometryBeforeUpdateBlocking();
}

/**
 * @internal
 *
 * Schedules the frame geometry of the XdgSurfaceV6Client to be updated to @p rect.
 *
 * This method is usually called when the current window geometry size and the requested
 * window geometry size don't match. Under hood, this method will schedule a configure
 * event. When the configure event is acknowledged by the client, the frame geometry will
 * be changed to @p rect.
 */
void XdgSurfaceV6Client::requestGeometry(const QRect &rect)
{
    m_requestedFrameGeometry = rect;
    m_requestedClientGeometry = frameRectToClientRect(rect);

    // Note that we don't have to send the configure event immediately!
    scheduleConfigure();
}

/**
 * @internal
 *
 * Updates the frame geometry of the XdgSurfaceV6Client to @p rect.
 *
 * This method is called when the frame geometry needs to be updated and the window geometry
 * hasn't changed or when a configure event has been acknowledged by the client.
 */
void XdgSurfaceV6Client::updateGeometry(const QRect &rect)
{
    const QRect oldFrameGeometry = m_frameGeometry;

    m_frameGeometry = rect;
    m_bufferGeometry = frameRectToBufferRect(rect);
    m_clientGeometry = frameRectToClientRect(rect);

    if (oldFrameGeometry == m_frameGeometry) {
        return;
    }

    updateWindowRules(Rules::Position | Rules::Size);
    updateGeometryBeforeUpdateBlocking();

    emit frameGeometryChanged(this, oldFrameGeometry);
    emit geometryShapeChanged(this, oldFrameGeometry);

    addRepaintDuringGeometryUpdates();
}

/**
 * @internal
 * @todo We have to check the current frame geometry in checkWorskpacePosition().
 *
 * Sets the geometry restore to the first valid frame geometry. This is a HACK!
 *
 * Unfortunately, AbstractClient::checkWorkspacePosition() operates on the geometry restore
 * rather than the current frame geometry, so we have to ensure that it's initialized with
 * some reasonable value even if the client is not maximized or quick tiled.
 */
void XdgSurfaceV6Client::updateGeometryRestoreHack()
{
    if (isUnmapped() && geometryRestore().isEmpty() && !frameGeometry().isEmpty()) {
        setGeometryRestore(frameGeometry());
    }
}

/**
 * @todo The depth value must be set per surface basis. Drop this method when the scene is
 * redesigned for sub-surfaces.
 */
void XdgSurfaceV6Client::updateDepth()
{
    if (surface()->buffer()->hasAlphaChannel() && !isDesktop()) {
        setDepth(32);
    } else {
        setDepth(24);
    }
}

QRect XdgSurfaceV6Client::frameRectToBufferRect(const QRect &rect) const
{
    const int left = rect.left() + borderLeft() - m_windowGeometry.left();
    const int top = rect.top() + borderTop() - m_windowGeometry.top();
    return QRect(QPoint(left, top), surface()->size());
}

/**
 * Reimplemented to schedule a repaint when the main surface is damaged.
 *
 * @todo This is actually incorrect. We need to schedule repaints per surface basis, not just
 * for the main surface. Drop this method once the scene is redesigned.
 */
void XdgSurfaceV6Client::addDamage(const QRegion &damage)
{
    const int offsetX = m_bufferGeometry.x() - m_frameGeometry.x();
    const int offsetY = m_bufferGeometry.y() - m_frameGeometry.y();
    repaints_region += damage.translated(offsetX, offsetY);
    Toplevel::addDamage(damage);
}

bool XdgSurfaceV6Client::isShown(bool shaded_is_shown) const
{
    Q_UNUSED(shaded_is_shown)
    return !isClosing() && !isHidden() && !isMinimized() && !isUnmapped();
}

bool XdgSurfaceV6Client::isHiddenInternal() const
{
    return isUnmapped() || isHidden();
}

void XdgSurfaceV6Client::hideClient(bool hide)
{
    if (hide) {
        internalHide();
    } else {
        internalShow();
    }
}

bool XdgSurfaceV6Client::isHidden() const
{
    return m_isHidden;
}

void XdgSurfaceV6Client::internalShow()
{
    if (!isHidden()) {
        return;
    }
    m_isHidden = false;
    addRepaintFull();
    emit windowShown(this);
}

void XdgSurfaceV6Client::internalHide()
{
    if (isHidden()) {
        return;
    }
    if (isMoveResize()) {
        leaveMoveResize();
    }
    m_isHidden = true;
    addWorkspaceRepaint(visibleRect());
    workspace()->clientHidden(this);
    emit windowHidden(this);
}

bool XdgSurfaceV6Client::isUnmapped() const
{
    return m_isUnmapped;
}

void XdgSurfaceV6Client::internalMap()
{
    if (!isUnmapped()) {
        return;
    }
    m_isUnmapped = false;
    if (readyForPainting()) {
        addRepaintFull();
        emit windowShown(this);
    } else {
        setReadyForPainting();
    }
    emit windowMapped();
}

/**
 * @todo We probably just need to destroy XdgSurfaceV6Client when the xdg-surface is unmapped.
 */
void XdgSurfaceV6Client::internalUnmap()
{
    if (isUnmapped()) {
        return;
    }
    if (isMoveResize()) {
        leaveMoveResize();
    }
    m_isUnmapped = true;
    m_requestedClientGeometry = QRect();
    m_lastAcknowledgedConfigure = nullptr;
    m_configureTimer->stop();
    qDeleteAll(m_configureEvents);
    m_configureEvents.clear();
    addWorkspaceRepaint(visibleRect());
    workspace()->clientHidden(this);
    emit windowHidden(this);
    emit windowUnmapped();
}

bool XdgSurfaceV6Client::isClosing() const
{
    return m_isClosing;
}

void XdgSurfaceV6Client::destroyClient()
{
    m_isClosing = true;
    m_configureTimer->stop();
    if (isMoveResize()) {
        leaveMoveResize();
    }
    cleanTabBox();
    Deleted *deleted = Deleted::create(this);
    emit windowClosed(this, deleted);
    StackingUpdatesBlocker blocker(workspace());
    RuleBook::self()->discardUsed(this, true);
    destroyWindowManagementInterface();
    destroyDecoration();
    cleanGrouping();
    waylandServer()->removeClient(this);
    deleted->unrefWindow();
    delete this;
}

void XdgSurfaceV6Client::cleanGrouping()
{
    if (transientFor()) {
        transientFor()->removeTransient(this);
    }
    for (auto it = transients().constBegin(); it != transients().constEnd();) {
        if ((*it)->transientFor() == this) {
            removeTransient(*it);
            it = transients().constBegin(); // restart, just in case something more has changed with the list
        } else {
            ++it;
        }
    }
}

void XdgSurfaceV6Client::cleanTabBox()
{
#ifdef KWIN_BUILD_TABBOX
    TabBox::TabBox *tabBox = TabBox::TabBox::self();
    if (tabBox->isDisplayed() && tabBox->currentClient() == this) {
        tabBox->nextPrev(true);
    }
#endif
}

void XdgToplevelV6Configure::setStates(const XdgToplevelV6Interface::States &states)
{
    m_states = states;
}

XdgToplevelV6Interface::States XdgToplevelV6Configure::states() const
{
    return m_states;
}

XdgToplevelV6Client::XdgToplevelV6Client(XdgToplevelV6Interface *shellSurface)
    : XdgSurfaceV6Client(shellSurface->xdgSurface())
    , m_shellSurface(shellSurface)
{
    setupWindowManagementIntegration();
    setupPlasmaShellIntegration();
    setDesktop(VirtualDesktopManager::self()->current());

    if (waylandServer()->inputMethodConnection() == surface()->client()) {
        m_windowType = NET::OnScreenDisplay;
    }

    connect(shellSurface, &XdgToplevelV6Interface::windowTitleChanged,
            this, &XdgToplevelV6Client::handleWindowTitleChanged);
    connect(shellSurface, &XdgToplevelV6Interface::windowClassChanged,
            this, &XdgToplevelV6Client::handleWindowClassChanged);
    connect(shellSurface, &XdgToplevelV6Interface::windowMenuRequested,
            this, &XdgToplevelV6Client::handleWindowMenuRequested);
    connect(shellSurface, &XdgToplevelV6Interface::moveRequested,
            this, &XdgToplevelV6Client::handleMoveRequested);
    connect(shellSurface, &XdgToplevelV6Interface::resizeRequested,
            this, &XdgToplevelV6Client::handleResizeRequested);
    connect(shellSurface, &XdgToplevelV6Interface::maximizeRequested,
            this, &XdgToplevelV6Client::handleMaximizeRequested);
    connect(shellSurface, &XdgToplevelV6Interface::unmaximizeRequested,
            this, &XdgToplevelV6Client::handleUnmaximizeRequested);
    connect(shellSurface, &XdgToplevelV6Interface::fullscreenRequested,
            this, &XdgToplevelV6Client::handleFullscreenRequested);
    connect(shellSurface, &XdgToplevelV6Interface::unfullscreenRequested,
            this, &XdgToplevelV6Client::handleUnfullscreenRequested);
    connect(shellSurface, &XdgToplevelV6Interface::minimizeRequested,
            this, &XdgToplevelV6Client::handleMinimizeRequested);
    connect(shellSurface, &XdgToplevelV6Interface::parentXdgToplevelChanged,
            this, &XdgToplevelV6Client::handleTransientForChanged);
    connect(shellSurface, &XdgToplevelV6Interface::initializeRequested,
            this, &XdgToplevelV6Client::initialize);
    connect(shellSurface, &XdgToplevelV6Interface::destroyed,
            this, &XdgToplevelV6Client::destroyClient);
    connect(shellSurface->shell(), &XdgShellV6Interface::pingTimeout,
            this, &XdgToplevelV6Client::handlePingTimeout);
    connect(shellSurface->shell(), &XdgShellV6Interface::pingDelayed,
            this, &XdgToplevelV6Client::handlePingDelayed);
    connect(shellSurface->shell(), &XdgShellV6Interface::pongReceived,
            this, &XdgToplevelV6Client::handlePongReceived);

    connect(waylandServer(), &WaylandServer::foreignTransientChanged,
            this, &XdgToplevelV6Client::handleForeignTransientForChanged);

    connect(this, &XdgToplevelV6Client::clientStartUserMovedResized,
            this, &XdgToplevelV6Client::scheduleConfigure);
    connect(this, &XdgToplevelV6Client::clientFinishUserMovedResized,
            this, &XdgToplevelV6Client::scheduleConfigure);
}

XdgToplevelV6Client::~XdgToplevelV6Client()
{
}

void XdgToplevelV6Client::debug(QDebug &stream) const
{
    stream << this;
}

NET::WindowType XdgToplevelV6Client::windowType(bool direct, int supported_types) const
{
    Q_UNUSED(direct)
    Q_UNUSED(supported_types)
    return m_windowType;
}

MaximizeMode XdgToplevelV6Client::maximizeMode() const
{
    return m_maximizeMode;
}

MaximizeMode XdgToplevelV6Client::requestedMaximizeMode() const
{
    return m_requestedMaximizeMode;
}

QSize XdgToplevelV6Client::minSize() const
{
    return rules()->checkMinSize(m_shellSurface->minimumSize());
}

QSize XdgToplevelV6Client::maxSize() const
{
    return rules()->checkMaxSize(m_shellSurface->maximumSize());
}

bool XdgToplevelV6Client::isFullScreen() const
{
    return m_isFullScreen;
}

bool XdgToplevelV6Client::isMovable() const
{
    if (isFullScreen()) {
        return false;
    }
    if (isSpecialWindow() && !isSplash() && !isToolbar()) {
        return false;
    }
    if (rules()->checkPosition(invalidPoint) != invalidPoint) {
        return false;
    }
    return true;
}

bool XdgToplevelV6Client::isMovableAcrossScreens() const
{
    if (isSpecialWindow() && !isSplash() && !isToolbar()) {
        return false;
    }
    if (rules()->checkPosition(invalidPoint) != invalidPoint) {
        return false;
    }
    return true;
}

bool XdgToplevelV6Client::isResizable() const
{
    if (isFullScreen()) {
        return false;
    }
    if (isSpecialWindow() || isSplash() || isToolbar()) {
        return false;
    }
    if (rules()->checkSize(QSize()).isValid()) {
        return false;
    }
    return true;
}

bool XdgToplevelV6Client::isCloseable() const
{
    return !isDesktop() && !isDock();
}

bool XdgToplevelV6Client::isFullScreenable() const
{
    if (!rules()->checkFullScreen(true)) {
        return false;
    }
    return !isSpecialWindow();
}

bool XdgToplevelV6Client::isMaximizable() const
{
    if (!isResizable()) {
        return false;
    }
    if (rules()->checkMaximize(MaximizeRestore) != MaximizeRestore ||
            rules()->checkMaximize(MaximizeFull) != MaximizeFull) {
        return false;
    }
    return true;
}

bool XdgToplevelV6Client::isMinimizable() const
{
    if (isSpecialWindow() && !isTransient()) {
        return false;
    }
    if (!rules()->checkMinimize(true)) {
        return false;
    }
    return true;
}

bool XdgToplevelV6Client::isTransient() const
{
    return m_isTransient;
}

bool XdgToplevelV6Client::userCanSetFullScreen() const
{
    return true;
}

bool XdgToplevelV6Client::userCanSetNoBorder() const
{
    if (m_serverDecoration) {
        switch (m_serverDecoration->mode()) {
        case ServerSideDecorationManagerInterface::Mode::Server:
            return !isFullScreen() && !isShade();
        case ServerSideDecorationManagerInterface::Mode::Client:
        case ServerSideDecorationManagerInterface::Mode::None:
            return false;
        }
    }
    return false;
}

bool XdgToplevelV6Client::noBorder() const
{
    if (m_serverDecoration) {
        switch (m_serverDecoration->mode()) {
        case ServerSideDecorationManagerInterface::Mode::Server:
            return m_userNoBorder || isFullScreen();
        case ServerSideDecorationManagerInterface::Mode::Client:
        case ServerSideDecorationManagerInterface::Mode::None:
            return true;
        }
    }
    return true;
}

void XdgToplevelV6Client::setNoBorder(bool set)
{
    if (!userCanSetNoBorder()) {
        return;
    }
    set = rules()->checkNoBorder(set);
    if (m_userNoBorder == set) {
        return;
    }
    m_userNoBorder = set;
    updateDecoration(true, false);
    updateWindowRules(Rules::NoBorder);
}

void XdgToplevelV6Client::updateDecoration(bool check_workspace_pos, bool force)
{
    if (!force && ((!isDecorated() && noBorder()) || (isDecorated() && !noBorder()))) {
        return;
    }
    const QRect oldFrameGeometry = frameGeometry();
    const QRect oldClientGeometry = clientGeometry();
    blockGeometryUpdates(true);
    if (force) {
        destroyDecoration();
    }
    if (!noBorder()) {
        createDecoration(oldFrameGeometry);
    } else {
        destroyDecoration();
    }
    if (m_serverDecoration && isDecorated()) {
        m_serverDecoration->setMode(KWayland::Server::ServerSideDecorationManagerInterface::Mode::Server);
    }
    updateShadow();
    if (check_workspace_pos) {
        checkWorkspacePosition(oldFrameGeometry, -2, oldClientGeometry);
    }
    blockGeometryUpdates(false);
}

bool XdgToplevelV6Client::supportsWindowRules() const
{
    return !m_plasmaShellSurface;
}

bool XdgToplevelV6Client::hasStrut() const
{
    if (!isShown(true)) {
        return false;
    }
    if (!m_plasmaShellSurface) {
        return false;
    }
    if (m_plasmaShellSurface->role() != PlasmaShellSurfaceInterface::Role::Panel) {
        return false;
    }
    return m_plasmaShellSurface->panelBehavior() == PlasmaShellSurfaceInterface::PanelBehavior::AlwaysVisible;
}

void XdgToplevelV6Client::showOnScreenEdge()
{
    if (!m_plasmaShellSurface || isUnmapped()) {
        return;
    }
    hideClient(false);
    workspace()->raiseClient(this);
    if (m_plasmaShellSurface->panelBehavior() == PlasmaShellSurfaceInterface::PanelBehavior::AutoHide) {
        m_plasmaShellSurface->showAutoHidingPanel();
    }
}

bool XdgToplevelV6Client::isInitialPositionSet() const
{
    return m_plasmaShellSurface ? m_plasmaShellSurface->isPositionSet() : false;
}

void XdgToplevelV6Client::closeWindow()
{
    if (isCloseable()) {
        sendPing(PingReason::CloseWindow);
        m_shellSurface->sendClose();
    }
}

XdgSurfaceV6Configure *XdgToplevelV6Client::sendRoleConfigure() const
{
    XdgToplevelV6Interface::States xdgStates;

    if (isActive()) {
        xdgStates |= XdgToplevelV6Interface::State::Activated;
    }
    if (isResize()) {
        xdgStates |= XdgToplevelV6Interface::State::Resizing;
    }
    if (requestedMaximizeMode() & MaximizeHorizontal) {
        xdgStates |= XdgToplevelV6Interface::State::MaximizedHorizontal;
    }
    if (requestedMaximizeMode() & MaximizeVertical) {
        xdgStates |= XdgToplevelV6Interface::State::MaximizedVertical;
    }
    if (isFullScreen()) {
        xdgStates |= XdgToplevelV6Interface::State::FullScreen;
    }

    const quint32 serial = m_shellSurface->sendConfigure(requestedClientSize(), xdgStates);

    XdgToplevelV6Configure *configureEvent = new XdgToplevelV6Configure();
    configureEvent->setGeometry(requestedFrameGeometry());
    configureEvent->setStates(xdgStates);
    configureEvent->setSerial(serial);

    return configureEvent;
}

void XdgToplevelV6Client::handleRoleCommit()
{
    auto configure = static_cast<XdgToplevelV6Configure *>(lastAcknowledgedConfigure());
    if (configure) {
        handleStatesAcknowledged(configure->states());
    }
}

void XdgToplevelV6Client::doMinimize()
{
    if (isMinimized()) {
        workspace()->clientHidden(this);
    } else {
        emit windowShown(this);
    }
    workspace()->updateMinimizedOfTransients(this);
}

void XdgToplevelV6Client::doResizeSync()
{
    requestGeometry(moveResizeGeometry());
}

void XdgToplevelV6Client::doSetActive()
{
    scheduleConfigure();
    if (!isActive()) {
        return;
    }
    StackingUpdatesBlocker blocker(workspace());
    workspace()->focusToNull();
}

void XdgToplevelV6Client::takeFocus()
{
    if (wantsInput()) {
        sendPing(PingReason::FocusWindow);
        setActive(true);
    }
    if (!keepAbove() && !isOnScreenDisplay() && !belongsToDesktop()) {
        workspace()->setShowingDesktop(false);
    }
}

bool XdgToplevelV6Client::wantsInput() const
{
    return rules()->checkAcceptFocus(acceptsFocus());
}

bool XdgToplevelV6Client::dockWantsInput() const
{
    if (m_plasmaShellSurface) {
        if (m_plasmaShellSurface->role() == PlasmaShellSurfaceInterface::Role::Panel) {
            return m_plasmaShellSurface->panelTakesFocus();
        }
    }
    return false;
}

bool XdgToplevelV6Client::acceptsFocus() const
{
    if (isInputMethod()) {
        return false;
    }
    if (m_plasmaShellSurface) {
        if (m_plasmaShellSurface->role() == PlasmaShellSurfaceInterface::Role::OnScreenDisplay ||
            m_plasmaShellSurface->role() == PlasmaShellSurfaceInterface::Role::ToolTip) {
            return false;
        }
        if (m_plasmaShellSurface->role() == PlasmaShellSurfaceInterface::Role::Notification ||
            m_plasmaShellSurface->role() == PlasmaShellSurfaceInterface::Role::CriticalNotification) {
            return m_plasmaShellSurface->panelTakesFocus();
        }
    }
    return !isClosing() && !isUnmapped();
}

Layer XdgToplevelV6Client::layerForDock() const
{
    if (m_plasmaShellSurface) {
        switch (m_plasmaShellSurface->panelBehavior()) {
        case PlasmaShellSurfaceInterface::PanelBehavior::WindowsCanCover:
            return NormalLayer;
        case PlasmaShellSurfaceInterface::PanelBehavior::AutoHide:
            return AboveLayer;
        case PlasmaShellSurfaceInterface::PanelBehavior::WindowsGoBelow:
        case PlasmaShellSurfaceInterface::PanelBehavior::AlwaysVisible:
            return DockLayer;
        default:
            Q_UNREACHABLE();
            break;
        }
    }
    return AbstractClient::layerForDock();
}

void XdgToplevelV6Client::handleWindowTitleChanged()
{
    setCaption(m_shellSurface->windowTitle());
}

void XdgToplevelV6Client::handleWindowClassChanged()
{
    const QByteArray applicationId = m_shellSurface->windowClass().toUtf8();
    setResourceClass(resourceName(), applicationId);
    if (m_isInitialized && supportsWindowRules()) {
        evaluateWindowRules();
    }
    setDesktopFileName(applicationId);
}

void XdgToplevelV6Client::handleWindowMenuRequested(SeatInterface *seat, const QPoint &surfacePos,
                                                    quint32 serial)
{
    Q_UNUSED(seat)
    Q_UNUSED(serial)
    performMouseCommand(Options::MouseOperationsMenu, pos() + surfacePos);
}

void XdgToplevelV6Client::handleMoveRequested(SeatInterface *seat, quint32 serial)
{
    Q_UNUSED(seat)
    Q_UNUSED(serial)
    performMouseCommand(Options::MouseMove, Cursor::pos());
}

void XdgToplevelV6Client::handleResizeRequested(SeatInterface *seat, Qt::Edges edges, quint32 serial)
{
    Q_UNUSED(seat)
    Q_UNUSED(serial)
    if (!isResizable() || isShade()) {
        return;
    }
    if (isMoveResize()) {
        finishMoveResize(false);
    }
    setMoveResizePointerButtonDown(true);
    setMoveOffset(Cursor::pos() - pos());
    setInvertedMoveOffset(rect().bottomRight() - moveOffset());
    setUnrestrictedMoveResize(false);
    auto toPosition = [edges] {
        Position position = PositionCenter;
        if (edges.testFlag(Qt::TopEdge)) {
            position = PositionTop;
        } else if (edges.testFlag(Qt::BottomEdge)) {
            position = PositionBottom;
        }
        if (edges.testFlag(Qt::LeftEdge)) {
            position = Position(position | PositionLeft);
        } else if (edges.testFlag(Qt::RightEdge)) {
            position = Position(position | PositionRight);
        }
        return position;
    };
    setMoveResizePointerMode(toPosition());
    if (!startMoveResize()) {
        setMoveResizePointerButtonDown(false);
    }
    updateCursor();
}

void XdgToplevelV6Client::handleStatesAcknowledged(const XdgToplevelV6Interface::States &states)
{
    const XdgToplevelV6Interface::States delta = m_lastAcknowledgedStates ^ states;

    if (delta & XdgToplevelV6Interface::State::Maximized) {
        MaximizeMode maximizeMode = MaximizeRestore;
        if (states & XdgToplevelV6Interface::State::MaximizedHorizontal) {
            maximizeMode = MaximizeMode(maximizeMode | MaximizeVertical);
        }
        if (states & XdgToplevelV6Interface::State::MaximizedVertical) {
            maximizeMode = MaximizeMode(maximizeMode | MaximizeHorizontal);
        }
        updateMaximizeMode(maximizeMode);
    }
    if (delta & XdgToplevelV6Interface::State::FullScreen) {
        updateFullScreenMode(states & XdgToplevelV6Interface::State::FullScreen);
    }

    m_lastAcknowledgedStates = states;
}

void XdgToplevelV6Client::handleMaximizeRequested()
{
    maximize(MaximizeFull);
    scheduleConfigure();
}

void XdgToplevelV6Client::handleUnmaximizeRequested()
{
    maximize(MaximizeRestore);
    scheduleConfigure();
}

void XdgToplevelV6Client::handleFullscreenRequested(OutputInterface *output)
{
    Q_UNUSED(output)
    setFullScreen(/* set */ true, /* user */ false);
    scheduleConfigure();
}

void XdgToplevelV6Client::handleUnfullscreenRequested()
{
    setFullScreen(/* set */ false, /* user */ false);
    scheduleConfigure();
}

void XdgToplevelV6Client::handleMinimizeRequested()
{
    performMouseCommand(Options::MouseMinimize, Cursor::pos());
}

void XdgToplevelV6Client::handleTransientForChanged()
{
    SurfaceInterface *transientForSurface = nullptr;
    if (XdgToplevelV6Interface *parentToplevel = m_shellSurface->parentXdgToplevel()) {
        transientForSurface = parentToplevel->surface();
    }
    if (!transientForSurface) {
        transientForSurface = waylandServer()->findForeignTransientForSurface(surface());
    }
    AbstractClient *transientForClient = waylandServer()->findClient(transientForSurface);
    if (transientForClient != transientFor()) {
        if (transientFor()) {
            transientFor()->removeTransient(this);
        }
        if (transientForClient) {
            transientForClient->addTransient(this);
        }
        setTransientFor(transientForClient);
    }
    m_isTransient = transientForClient;
}

void XdgToplevelV6Client::handleForeignTransientForChanged(SurfaceInterface *child)
{
    if (surface() == child) {
        handleTransientForChanged();
    }
}

void XdgToplevelV6Client::handlePingTimeout(quint32 serial)
{
    auto pingIt = m_pings.find(serial);
    if (pingIt == m_pings.end()) {
        return;
    }
    if (pingIt.value() == PingReason::CloseWindow) {
        qCDebug(KWIN_CORE) << "Final ping timeout on a close attempt, asking to kill:" << caption();

        // For internal windows, killing the window will delete this.
        QPointer<QObject> guard(this);
        killWindow();
        if (!guard) {
            return;
        }
    }
    m_pings.erase(pingIt);
}

void XdgToplevelV6Client::handlePingDelayed(quint32 serial)
{
    auto it = m_pings.find(serial);
    if (it != m_pings.end()) {
        qCDebug(KWIN_CORE) << "First ping timeout:" << caption();
        setUnresponsive(true);
    }
}

void XdgToplevelV6Client::handlePongReceived(quint32 serial)
{
    auto it = m_pings.find(serial);
    if (it != m_pings.end()) {
        setUnresponsive(false);
        m_pings.erase(it);
    }
}

void XdgToplevelV6Client::sendPing(PingReason reason)
{
    XdgShellV6Interface *shell = m_shellSurface->shell();
    XdgSurfaceV6Interface *surface = m_shellSurface->xdgSurface();

    const quint32 serial = shell->ping(surface);
    m_pings.insert(serial, reason);
}

void XdgToplevelV6Client::initialize()
{
    blockGeometryUpdates(true);

    bool needsPlacement = !isInitialPositionSet();

    if (supportsWindowRules()) {
        setupWindowRules(false);

        const QRect originalGeometry = frameGeometry();
        const QRect ruledGeometry = rules()->checkGeometry(originalGeometry, true);
        if (originalGeometry != ruledGeometry) {
            setFrameGeometry(ruledGeometry);
        }
        maximize(rules()->checkMaximize(maximizeMode(), true));
        setDesktop(rules()->checkDesktop(desktop(), true));
        setDesktopFileName(rules()->checkDesktopFile(desktopFileName(), true).toUtf8());
        if (rules()->checkMinimize(isMinimized(), true)) {
            minimize(true); // No animation.
        }
        setSkipTaskbar(rules()->checkSkipTaskbar(skipTaskbar(), true));
        setSkipPager(rules()->checkSkipPager(skipPager(), true));
        setSkipSwitcher(rules()->checkSkipSwitcher(skipSwitcher(), true));
        setKeepAbove(rules()->checkKeepAbove(keepAbove(), true));
        setKeepBelow(rules()->checkKeepBelow(keepBelow(), true));
        setShortcut(rules()->checkShortcut(shortcut().toString(), true));

        // Don't place the client if its position is set by a rule.
        if (rules()->checkPosition(invalidPoint, true) != invalidPoint) {
            needsPlacement = false;
        }

        // Don't place the client if the maximize state is set by a rule.
        if (requestedMaximizeMode() != MaximizeRestore) {
            needsPlacement = false;
        }

        discardTemporaryRules();
        RuleBook::self()->discardUsed(this, false); // Remove Apply Now rules.
        updateWindowRules(Rules::All);
    }
    if (isFullScreen()) {
        needsPlacement = false;
    }
    if (needsPlacement) {
        const QRect area = workspace()->clientArea(PlacementArea, Screens::self()->current(), desktop());
        placeIn(area);
    }

    blockGeometryUpdates(false);
    scheduleConfigure();
    updateColorScheme();

    m_isInitialized = true;
}

void XdgToplevelV6Client::updateMaximizeMode(MaximizeMode maximizeMode)
{
    if (m_maximizeMode == maximizeMode) {
        return;
    }
    m_maximizeMode = maximizeMode;
    updateWindowRules(Rules::MaximizeVert | Rules::MaximizeHoriz);
    emit clientMaximizedStateChanged(this, maximizeMode);
    emit clientMaximizedStateChanged(this, maximizeMode & MaximizeHorizontal, maximizeMode & MaximizeVertical);
}

void XdgToplevelV6Client::updateFullScreenMode(bool set)
{
    if (m_isFullScreen == set) {
        return;
    }
    m_isFullScreen = set;
    updateWindowRules(Rules::Fullscreen);
    emit fullScreenChanged();
}

void XdgToplevelV6Client::updateColorScheme()
{
    if (m_paletteInterface) {
        AbstractClient::updateColorScheme(rules()->checkDecoColor(m_paletteInterface->palette()));
    } else {
        AbstractClient::updateColorScheme(rules()->checkDecoColor(QString()));
    }
}

void XdgToplevelV6Client::installAppMenu(AppMenuInterface *appMenu)
{
    m_appMenuInterface = appMenu;

    auto updateMenu = [this](const AppMenuInterface::InterfaceAddress &address) {
        updateApplicationMenuServiceName(address.serviceName);
        updateApplicationMenuObjectPath(address.objectPath);
    };
    connect(m_appMenuInterface, &AppMenuInterface::addressChanged, this, updateMenu);
    updateMenu(appMenu->address());
}

void XdgToplevelV6Client::installServerDecoration(ServerSideDecorationInterface *decoration)
{
    m_serverDecoration = decoration;

    connect(m_serverDecoration, &ServerSideDecorationInterface::destroyed, this, [this] {
        if (!isClosing() && !isUnmapped()) {
            updateDecoration(/* check_workspace_pos */ true);
        }
    });
    connect(m_serverDecoration, &ServerSideDecorationInterface::modeRequested, this,
        [this] (ServerSideDecorationManagerInterface::Mode mode) {
            const bool changed = mode != m_serverDecoration->mode();
            if (changed && !isUnmapped()) {
                updateDecoration(/* check_workspace_pos */ false);
            }
        }
    );
    if (!isUnmapped()) {
        updateDecoration(/* check_workspace_pos */ true);
    }
}

void XdgToplevelV6Client::installPalette(ServerSideDecorationPaletteInterface *palette)
{
    m_paletteInterface = palette;

    auto updatePalette = [this](const QString &palette) {
        AbstractClient::updateColorScheme(rules()->checkDecoColor(palette));
    };
    connect(m_paletteInterface, &ServerSideDecorationPaletteInterface::paletteChanged, this, [=](const QString &palette) {
        updatePalette(palette);
    });
    connect(m_paletteInterface, &QObject::destroyed, this, [=]() {
        updatePalette(QString());
    });
    updatePalette(palette->palette());
}

/**
 * @todo This whole plasma shell surface thing doesn't seem right. It turns xdg-toplevel into
 * something completely different! Perhaps plasmashell surfaces need to be implemented via a
 * proprietary protocol that doesn't piggyback on existing shell surface protocols. It'll lead
 * to cleaner code and will be technically correct, but I'm not sure whether this is do-able.
 */
void XdgToplevelV6Client::installPlasmaShellSurface(PlasmaShellSurfaceInterface *shellSurface)
{
    m_plasmaShellSurface = shellSurface;

    auto updatePosition = [this, shellSurface] { move(shellSurface->position()); };
    auto updateRole = [this, shellSurface] {
        NET::WindowType type = NET::Unknown;
        switch (shellSurface->role()) {
        case PlasmaShellSurfaceInterface::Role::Desktop:
            type = NET::Desktop;
            break;
        case PlasmaShellSurfaceInterface::Role::Panel:
            type = NET::Dock;
            break;
        case PlasmaShellSurfaceInterface::Role::OnScreenDisplay:
            type = NET::OnScreenDisplay;
            break;
        case PlasmaShellSurfaceInterface::Role::Notification:
            type = NET::Notification;
            break;
        case PlasmaShellSurfaceInterface::Role::ToolTip:
            type = NET::Tooltip;
            break;
        case PlasmaShellSurfaceInterface::Role::CriticalNotification:
            type = NET::CriticalNotification;
            break;
        case PlasmaShellSurfaceInterface::Role::Normal:
        default:
            type = NET::Normal;
            break;
        }
        if (m_windowType == type) {
            return;
        }
        m_windowType = type;
        switch (m_windowType) {
        case NET::Desktop:
        case NET::Dock:
        case NET::OnScreenDisplay:
        case NET::Notification:
        case NET::CriticalNotification:
        case NET::Tooltip:
            setOnAllDesktops(true);
            break;
        default:
            break;
        }
        workspace()->updateClientArea();
    };
    connect(shellSurface, &PlasmaShellSurfaceInterface::positionChanged, this, updatePosition);
    connect(shellSurface, &PlasmaShellSurfaceInterface::roleChanged, this, updateRole);
    connect(shellSurface, &PlasmaShellSurfaceInterface::panelBehaviorChanged, this, [this] {
        updateShowOnScreenEdge();
        workspace()->updateClientArea();
    });
    connect(shellSurface, &PlasmaShellSurfaceInterface::panelAutoHideHideRequested, this, [this] {
        hideClient(true);
        m_plasmaShellSurface->hideAutoHidingPanel();
        updateShowOnScreenEdge();
    });
    connect(shellSurface, &PlasmaShellSurfaceInterface::panelAutoHideShowRequested, this, [this] {
        hideClient(false);
        ScreenEdges::self()->reserve(this, ElectricNone);
        m_plasmaShellSurface->showAutoHidingPanel();
    });
    updatePosition();
    updateRole();
    updateShowOnScreenEdge();
    connect(this, &XdgToplevelV6Client::frameGeometryChanged,
            this, &XdgToplevelV6Client::updateShowOnScreenEdge);
    connect(this, &XdgToplevelV6Client::windowShown,
            this, &XdgToplevelV6Client::updateShowOnScreenEdge);

    setSkipTaskbar(shellSurface->skipTaskbar());
    connect(shellSurface, &PlasmaShellSurfaceInterface::skipTaskbarChanged, this, [this] {
        setSkipTaskbar(m_plasmaShellSurface->skipTaskbar());
    });

    setSkipSwitcher(shellSurface->skipSwitcher());
    connect(shellSurface, &PlasmaShellSurfaceInterface::skipSwitcherChanged, this, [this] {
        setSkipSwitcher(m_plasmaShellSurface->skipSwitcher());
    });
}

void XdgToplevelV6Client::updateShowOnScreenEdge()
{
    if (!ScreenEdges::self()) {
        return;
    }
    if (isUnmapped() || !m_plasmaShellSurface ||
            m_plasmaShellSurface->role() != PlasmaShellSurfaceInterface::Role::Panel) {
        ScreenEdges::self()->reserve(this, ElectricNone);
        return;
    }
    const PlasmaShellSurfaceInterface::PanelBehavior panelBehavior = m_plasmaShellSurface->panelBehavior();
    if ((panelBehavior == PlasmaShellSurfaceInterface::PanelBehavior::AutoHide && isHidden()) ||
            panelBehavior == PlasmaShellSurfaceInterface::PanelBehavior::WindowsCanCover) {
        // Screen edge API requires an edge, thus we need to figure out which edge the window borders.
        const QRect clientGeometry = frameGeometry();
        Qt::Edges edges;
        for (int i = 0; i < screens()->count(); i++) {
            const QRect screenGeometry = screens()->geometry(i);
            if (screenGeometry.left() == clientGeometry.left()) {
                edges |= Qt::LeftEdge;
            }
            if (screenGeometry.right() == clientGeometry.right()) {
                edges |= Qt::RightEdge;
            }
            if (screenGeometry.top() == clientGeometry.top()) {
                edges |= Qt::TopEdge;
            }
            if (screenGeometry.bottom() == clientGeometry.bottom()) {
                edges |= Qt::BottomEdge;
            }
        }

        // A panel might border multiple screen edges. E.g. a horizontal panel at the bottom will
        // also border the left and right edge. Let's remove such cases.
        if (edges & Qt::LeftEdge && edges & Qt::RightEdge) {
            edges = edges & (~(Qt::LeftEdge | Qt::RightEdge));
        }
        if (edges & Qt::TopEdge && edges & Qt::BottomEdge) {
            edges = edges & (~(Qt::TopEdge | Qt::BottomEdge));
        }

        // It's still possible that a panel borders two edges, e.g. bottom and left in that case
        // the one which is sharing more with the edge wins.
        auto check = [clientGeometry](Qt::Edges edges, Qt::Edge horizontal, Qt::Edge vertical) {
            if (edges & horizontal && edges & vertical) {
                if (clientGeometry.width() >= clientGeometry.height()) {
                    return edges & ~horizontal;
                } else {
                    return edges & ~vertical;
                }
            }
            return edges;
        };
        edges = check(edges, Qt::LeftEdge, Qt::TopEdge);
        edges = check(edges, Qt::LeftEdge, Qt::BottomEdge);
        edges = check(edges, Qt::RightEdge, Qt::TopEdge);
        edges = check(edges, Qt::RightEdge, Qt::BottomEdge);

        ElectricBorder border = ElectricNone;
        if (edges & Qt::LeftEdge) {
            border = ElectricLeft;
        }
        if (edges & Qt::RightEdge) {
            border = ElectricRight;
        }
        if (edges & Qt::TopEdge) {
            border = ElectricTop;
        }
        if (edges & Qt::BottomEdge) {
            border = ElectricBottom;
        }
        ScreenEdges::self()->reserve(this, border);
    } else {
        ScreenEdges::self()->reserve(this, ElectricNone);
    }
}

void XdgToplevelV6Client::setupWindowManagementIntegration()
{
    if (isLockScreen()) {
        return;
    }
    connect(this, &XdgToplevelV6Client::windowMapped,
            this, &XdgToplevelV6Client::setupWindowManagementInterface);
    connect(this, &XdgToplevelV6Client::windowUnmapped,
            this, &XdgToplevelV6Client::destroyWindowManagementInterface);
}

void XdgToplevelV6Client::setupPlasmaShellIntegration()
{
    connect(this, &XdgToplevelV6Client::windowMapped,
            this, &XdgToplevelV6Client::updateShowOnScreenEdge);
}

void XdgToplevelV6Client::setFullScreen(bool set, bool user)
{
    set = rules()->checkFullScreen(set);

    const bool wasFullscreen = isFullScreen();
    if (wasFullscreen == set) {
        return;
    }
    if (isSpecialWindow()) {
        return;
    }
    if (user && !userCanSetFullScreen()) {
        return;
    }

    if (wasFullscreen) {
        workspace()->updateFocusMousePosition(Cursor::pos()); // may cause leave event
    } else {
        m_fullScreenGeometryRestore = frameGeometry();
    }
    m_isFullScreen = set;

    if (set) {
        workspace()->raiseClient(this);
    }
    StackingUpdatesBlocker blocker1(workspace());
    GeometryUpdatesBlocker blocker2(this);

    workspace()->updateClientLayer(this);   // active fullscreens get different layer
    updateDecoration(false, false);

    if (set) {
        setFrameGeometry(workspace()->clientArea(FullScreenArea, this));
    } else {
        if (m_fullScreenGeometryRestore.isValid()) {
            int currentScreen = screen();
            setFrameGeometry(QRect(m_fullScreenGeometryRestore.topLeft(),
                                   constrainFrameSize(m_fullScreenGeometryRestore.size())));
            if( currentScreen != screen())
                workspace()->sendClientToScreen( this, currentScreen );
        } else {
            // this can happen when the window was first shown already fullscreen,
            // so let the client set the size by itself
            setFrameGeometry(QRect(workspace()->clientArea(PlacementArea, this).topLeft(), QSize(0, 0)));
        }
    }

    scheduleConfigure();

    updateWindowRules(Rules::Fullscreen|Rules::Position|Rules::Size);
    emit fullScreenChanged();
}

static bool changeMaximizeRecursion = false;
void XdgToplevelV6Client::changeMaximize(bool horizontal, bool vertical, bool adjust)
{
    if (changeMaximizeRecursion) {
        return;
    }

    if (!isResizable()) {
        return;
    }

    const QRect clientArea = isElectricBorderMaximizing() ?
        workspace()->clientArea(MaximizeArea, Cursor::pos(), desktop()) :
        workspace()->clientArea(MaximizeArea, this);

    const MaximizeMode oldMode = m_requestedMaximizeMode;
    const QRect oldGeometry = frameGeometry();

    // 'adjust == true' means to update the size only, e.g. after changing workspace size
    if (!adjust) {
        if (vertical)
            m_requestedMaximizeMode = MaximizeMode(m_requestedMaximizeMode ^ MaximizeVertical);
        if (horizontal)
            m_requestedMaximizeMode = MaximizeMode(m_requestedMaximizeMode ^ MaximizeHorizontal);
    }

    m_requestedMaximizeMode = rules()->checkMaximize(m_requestedMaximizeMode);
    if (!adjust && m_requestedMaximizeMode == oldMode) {
        return;
    }

    StackingUpdatesBlocker blocker(workspace());

    // call into decoration update borders
    if (isDecorated() && decoration()->client() && !(options->borderlessMaximizedWindows() && m_requestedMaximizeMode == KWin::MaximizeFull)) {
        changeMaximizeRecursion = true;
        const auto c = decoration()->client().toStrongRef();
        if ((m_requestedMaximizeMode & MaximizeVertical) != (oldMode & MaximizeVertical)) {
            emit c->maximizedVerticallyChanged(m_requestedMaximizeMode & MaximizeVertical);
        }
        if ((m_requestedMaximizeMode & MaximizeHorizontal) != (oldMode & MaximizeHorizontal)) {
            emit c->maximizedHorizontallyChanged(m_requestedMaximizeMode & MaximizeHorizontal);
        }
        if ((m_requestedMaximizeMode == MaximizeFull) != (oldMode == MaximizeFull)) {
            emit c->maximizedChanged(m_requestedMaximizeMode & MaximizeFull);
        }
        changeMaximizeRecursion = false;
    }

    if (options->borderlessMaximizedWindows()) {
        // triggers a maximize change.
        // The next setNoBorder interation will exit since there's no change but the first recursion pullutes the restore geometry
        changeMaximizeRecursion = true;
        setNoBorder(rules()->checkNoBorder(m_requestedMaximizeMode == MaximizeFull));
        changeMaximizeRecursion = false;
    }

    // Conditional quick tiling exit points
    const auto oldQuickTileMode = quickTileMode();
    if (quickTileMode() != QuickTileMode(QuickTileFlag::None)) {
        if (oldMode == MaximizeFull &&
                !clientArea.contains(geometryRestore().center())) {
            // Not restoring on the same screen
            // TODO: The following doesn't work for some reason
            //quick_tile_mode = QuickTileNone; // And exit quick tile mode manually
        } else if ((oldMode == MaximizeVertical && m_requestedMaximizeMode == MaximizeRestore) ||
                  (oldMode == MaximizeFull && m_requestedMaximizeMode == MaximizeHorizontal)) {
            // Modifying geometry of a tiled window
            updateQuickTileMode(QuickTileFlag::None); // Exit quick tile mode without restoring geometry
        }
    }

    if (m_requestedMaximizeMode == MaximizeFull) {
        setGeometryRestore(oldGeometry);
        // TODO: Client has more checks
        if (options->electricBorderMaximize()) {
            updateQuickTileMode(QuickTileFlag::Maximize);
        } else {
            updateQuickTileMode(QuickTileFlag::None);
        }
        if (quickTileMode() != oldQuickTileMode) {
            emit quickTileModeChanged();
        }
        setFrameGeometry(workspace()->clientArea(MaximizeArea, this));
        workspace()->raiseClient(this);
    } else {
        if (m_requestedMaximizeMode == MaximizeRestore) {
            updateQuickTileMode(QuickTileFlag::None);
        }
        if (quickTileMode() != oldQuickTileMode) {
            emit quickTileModeChanged();
        }

        if (geometryRestore().isValid()) {
            setFrameGeometry(geometryRestore());
        } else {
            setFrameGeometry(workspace()->clientArea(PlacementArea, this));
        }
    }

    scheduleConfigure();
}

XdgPopupV6Client::XdgPopupV6Client(XdgPopupV6Interface *shellSurface)
    : XdgSurfaceV6Client(shellSurface->xdgSurface())
    , m_shellSurface(shellSurface)
{
    setDesktop(VirtualDesktopManager::self()->current());

    connect(shellSurface, &XdgPopupV6Interface::grabRequested,
            this, &XdgPopupV6Client::handleGrabRequested);
    connect(shellSurface, &XdgPopupV6Interface::initializeRequested,
            this, &XdgPopupV6Client::initialize);
    connect(shellSurface, &XdgPopupV6Interface::destroyed,
            this, &XdgPopupV6Client::destroyClient);

    XdgSurfaceV6Interface *parentShellSurface = shellSurface->parentXdgSurface();
    AbstractClient *parentClient = waylandServer()->findClient(parentShellSurface->surface());
    parentClient->addTransient(this);
    setTransientFor(parentClient);
}

XdgPopupV6Client::~XdgPopupV6Client()
{
}

void XdgPopupV6Client::debug(QDebug &stream) const
{
    stream << this;
}

NET::WindowType XdgPopupV6Client::windowType(bool direct, int supported_types) const
{
    Q_UNUSED(direct)
    Q_UNUSED(supported_types)
    return NET::Normal;
}

bool XdgPopupV6Client::hasPopupGrab() const
{
    return m_haveExplicitGrab;
}

void XdgPopupV6Client::popupDone()
{
    m_shellSurface->sendPopupDone();
}

bool XdgPopupV6Client::isPopupWindow() const
{
    return true;
}

bool XdgPopupV6Client::isTransient() const
{
    return true;
}

bool XdgPopupV6Client::isResizable() const
{
    return false;
}

bool XdgPopupV6Client::isMovable() const
{
    return false;
}

bool XdgPopupV6Client::isMovableAcrossScreens() const
{
    return false;
}

bool XdgPopupV6Client::hasTransientPlacementHint() const
{
    return true;
}

static QPoint popupOffset(const QRect &anchorRect, const Qt::Edges anchorEdge,
                          const Qt::Edges gravity, const QSize popupSize)
{
    QPoint anchorPoint;
    switch (anchorEdge & (Qt::LeftEdge | Qt::RightEdge)) {
    case Qt::LeftEdge:
        anchorPoint.setX(anchorRect.x());
        break;
    case Qt::RightEdge:
        anchorPoint.setX(anchorRect.x() + anchorRect.width());
        break;
    default:
        anchorPoint.setX(qRound(anchorRect.x() + anchorRect.width() / 2.0));
    }
    switch (anchorEdge & (Qt::TopEdge | Qt::BottomEdge)) {
    case Qt::TopEdge:
        anchorPoint.setY(anchorRect.y());
        break;
    case Qt::BottomEdge:
        anchorPoint.setY(anchorRect.y() + anchorRect.height());
        break;
    default:
        anchorPoint.setY(qRound(anchorRect.y() + anchorRect.height() / 2.0));
    }

    // calculate where the top left point of the popup will end up with the applied gravity
    // gravity indicates direction. i.e if gravitating towards the top the popup's bottom edge
    // will next to the anchor point
    QPoint popupPosAdjust;
    switch (gravity & (Qt::LeftEdge | Qt::RightEdge)) {
    case Qt::LeftEdge:
        popupPosAdjust.setX(-popupSize.width());
        break;
    case Qt::RightEdge:
        popupPosAdjust.setX(0);
        break;
    default:
        popupPosAdjust.setX(qRound(-popupSize.width() / 2.0));
    }
    switch (gravity & (Qt::TopEdge | Qt::BottomEdge)) {
    case Qt::TopEdge:
        popupPosAdjust.setY(-popupSize.height());
        break;
    case Qt::BottomEdge:
        popupPosAdjust.setY(0);
        break;
    default:
        popupPosAdjust.setY(qRound(-popupSize.height() / 2.0));
    }

    return anchorPoint + popupPosAdjust;
}

QRect XdgPopupV6Client::transientPlacement(const QRect &bounds) const
{
    const XdgPositionerV6 positioner = m_shellSurface->positioner();
    const QSize desiredSize = isUnmapped() ? positioner.size() : size();

    const QPoint parentPosition = transientFor()->framePosToClientPos(transientFor()->pos());

    // returns if a target is within the supplied bounds, optional edges argument states which side to check
    auto inBounds = [bounds](const QRect &target, Qt::Edges edges = Qt::LeftEdge | Qt::RightEdge | Qt::TopEdge | Qt::BottomEdge) -> bool {
        if (edges & Qt::LeftEdge && target.left() < bounds.left()) {
            return false;
        }
        if (edges & Qt::TopEdge && target.top() < bounds.top()) {
            return false;
        }
        if (edges & Qt::RightEdge && target.right() > bounds.right()) {
            //normal QRect::right issue cancels out
            return false;
        }
        if (edges & Qt::BottomEdge && target.bottom() > bounds.bottom()) {
            return false;
        }
        return true;
    };

    QRect popupRect(popupOffset(positioner.anchorRect(), positioner.anchorEdges(), positioner.gravityEdges(), desiredSize) + positioner.offset() + parentPosition, desiredSize);

    //if that fits, we don't need to do anything
    if (inBounds(popupRect)) {
        return popupRect;
    }
    //otherwise apply constraint adjustment per axis in order XDG Shell Popup states

    if (positioner.flipConstraintAdjustments() & Qt::Horizontal) {
        if (!inBounds(popupRect, Qt::LeftEdge | Qt::RightEdge)) {
            //flip both edges (if either bit is set, XOR both)
            auto flippedAnchorEdge = positioner.anchorEdges();
            if (flippedAnchorEdge & (Qt::LeftEdge | Qt::RightEdge)) {
                flippedAnchorEdge ^= (Qt::LeftEdge | Qt::RightEdge);
            }
            auto flippedGravity = positioner.gravityEdges();
            if (flippedGravity & (Qt::LeftEdge | Qt::RightEdge)) {
                flippedGravity ^= (Qt::LeftEdge | Qt::RightEdge);
            }
            auto flippedPopupRect = QRect(popupOffset(positioner.anchorRect(), flippedAnchorEdge, flippedGravity, desiredSize) + positioner.offset() + parentPosition, desiredSize);

            //if it still doesn't fit we should continue with the unflipped version
            if (inBounds(flippedPopupRect, Qt::LeftEdge | Qt::RightEdge)) {
                popupRect.moveLeft(flippedPopupRect.left());
            }
        }
    }
    if (positioner.slideConstraintAdjustments() & Qt::Horizontal) {
        if (!inBounds(popupRect, Qt::LeftEdge)) {
            popupRect.moveLeft(bounds.left());
        }
        if (!inBounds(popupRect, Qt::RightEdge)) {
            popupRect.moveRight(bounds.right());
        }
    }
    if (positioner.resizeConstraintAdjustments() & Qt::Horizontal) {
        QRect unconstrainedRect = popupRect;

        if (!inBounds(unconstrainedRect, Qt::LeftEdge)) {
            unconstrainedRect.setLeft(bounds.left());
        }
        if (!inBounds(unconstrainedRect, Qt::RightEdge)) {
            unconstrainedRect.setRight(bounds.right());
        }

        if (unconstrainedRect.isValid()) {
            popupRect = unconstrainedRect;
        }
    }

    if (positioner.flipConstraintAdjustments() & Qt::Vertical) {
        if (!inBounds(popupRect, Qt::TopEdge | Qt::BottomEdge)) {
            //flip both edges (if either bit is set, XOR both)
            auto flippedAnchorEdge = positioner.anchorEdges();
            if (flippedAnchorEdge & (Qt::TopEdge | Qt::BottomEdge)) {
                flippedAnchorEdge ^= (Qt::TopEdge | Qt::BottomEdge);
            }
            auto flippedGravity = positioner.gravityEdges();
            if (flippedGravity & (Qt::TopEdge | Qt::BottomEdge)) {
                flippedGravity ^= (Qt::TopEdge | Qt::BottomEdge);
            }
            auto flippedPopupRect = QRect(popupOffset(positioner.anchorRect(), flippedAnchorEdge, flippedGravity, desiredSize) + positioner.offset() + parentPosition, desiredSize);

            //if it still doesn't fit we should continue with the unflipped version
            if (inBounds(flippedPopupRect, Qt::TopEdge | Qt::BottomEdge)) {
                popupRect.moveTop(flippedPopupRect.top());
            }
        }
    }
    if (positioner.slideConstraintAdjustments() & Qt::Vertical) {
        if (!inBounds(popupRect, Qt::TopEdge)) {
            popupRect.moveTop(bounds.top());
        }
        if (!inBounds(popupRect, Qt::BottomEdge)) {
            popupRect.moveBottom(bounds.bottom());
        }
    }
    if (positioner.resizeConstraintAdjustments() & Qt::Vertical) {
        QRect unconstrainedRect = popupRect;

        if (!inBounds(unconstrainedRect, Qt::TopEdge)) {
            unconstrainedRect.setTop(bounds.top());
        }
        if (!inBounds(unconstrainedRect, Qt::BottomEdge)) {
            unconstrainedRect.setBottom(bounds.bottom());
        }

        if (unconstrainedRect.isValid()) {
            popupRect = unconstrainedRect;
        }
    }

    return popupRect;
}

bool XdgPopupV6Client::isCloseable() const
{
    return false;
}

void XdgPopupV6Client::closeWindow()
{
}

void XdgPopupV6Client::updateColorScheme()
{
    AbstractClient::updateColorScheme(QString());
}

bool XdgPopupV6Client::noBorder() const
{
    return true;
}

bool XdgPopupV6Client::userCanSetNoBorder() const
{
    return false;
}

void XdgPopupV6Client::setNoBorder(bool set)
{
    Q_UNUSED(set)
}

void XdgPopupV6Client::updateDecoration(bool check_workspace_pos, bool force)
{
    Q_UNUSED(check_workspace_pos)
    Q_UNUSED(force)
}

void XdgPopupV6Client::showOnScreenEdge()
{
}

bool XdgPopupV6Client::supportsWindowRules() const
{
    return false;
}

bool XdgPopupV6Client::wantsInput() const
{
    return false;
}

void XdgPopupV6Client::takeFocus()
{
}

bool XdgPopupV6Client::acceptsFocus() const
{
    return false;
}

XdgSurfaceV6Configure *XdgPopupV6Client::sendRoleConfigure() const
{
    const QPoint parentPosition = transientFor()->framePosToClientPos(transientFor()->pos());
    const QPoint popupPosition = requestedPos() - parentPosition;

    const quint32 serial = m_shellSurface->sendConfigure(QRect(popupPosition, requestedClientSize()));

    XdgSurfaceV6Configure *configureEvent = new XdgSurfaceV6Configure();
    configureEvent->setGeometry(requestedFrameGeometry());
    configureEvent->setSerial(serial);

    return configureEvent;
}

void XdgPopupV6Client::handleGrabRequested(SeatInterface *seat, quint32 serial)
{
    Q_UNUSED(seat)
    Q_UNUSED(serial)
    m_haveExplicitGrab = true;
}

void XdgPopupV6Client::initialize()
{
    const QRect area = workspace()->clientArea(PlacementArea, Screens::self()->current(), desktop());
    placeIn(area);

    scheduleConfigure();
}

} // namespace KWin
