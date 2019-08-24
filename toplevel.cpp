/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006 Lubos Lunak <l.lunak@kde.org>
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
#include "toplevel.h"

#ifdef KWIN_BUILD_ACTIVITIES
#include "activities.h"
#endif
#include "atoms.h"
#include "client.h"
#include "client_machine.h"
#include "composite.h"
#include "effects.h"
#include "screens.h"
#include "shadow.h"
#include "workspace.h"
#include "xcbutils.h"

#include <KWayland/Server/surface_interface.h>

#include <QDebug>

namespace KWin
{

Toplevel::Toplevel()
    : m_visual(XCB_NONE)
    , bit_depth(24)
    , info(NULL)
    , ready_for_painting(true)
    , m_isDamaged(false)
    , m_internalId(QUuid::createUuid())
    , m_client()
    , damage_handle(None)
    , is_shape(false)
    , effect_window(NULL)
    , m_clientMachine(new ClientMachine(this))
    , wmClientLeaderWin(0)
    , m_damageReplyPending(false)
    , m_screen(0)
    , m_skipCloseAnimation(false)
{
    connect(this, SIGNAL(damaged(KWin::Toplevel*,QRect)), SIGNAL(needsRepaint()));
    connect(screens(), SIGNAL(changed()), SLOT(checkScreen()));
    connect(screens(), SIGNAL(countChanged(int,int)), SLOT(checkScreen()));
    setupCheckScreenConnection();
}

Toplevel::~Toplevel()
{
    assert(damage_handle == None);
    delete info;
}

QDebug& operator<<(QDebug& stream, const Toplevel* cl)
{
    if (cl == NULL)
        return stream << "\'NULL\'";
    cl->debug(stream);
    return stream;
}

QDebug& operator<<(QDebug& stream, const ToplevelList& list)
{
    stream << "LIST:(";
    bool first = true;
    for (ToplevelList::ConstIterator it = list.begin();
            it != list.end();
            ++it) {
        if (!first)
            stream << ":";
        first = false;
        stream << *it;
    }
    stream << ")";
    return stream;
}

QRect Toplevel::decorationRect() const
{
    return rect();
}

void Toplevel::detectShape(Window id)
{
    const bool wasShape = is_shape;
    is_shape = Xcb::Extensions::self()->hasShape(id);
    if (wasShape != is_shape) {
        emit shapedChanged();
    }
}

// used only by Deleted::copy()
void Toplevel::copyToDeleted(Toplevel* c)
{
    m_internalId = c->internalId();
    m_visual = c->m_visual;
    bit_depth = c->bit_depth;
    info = c->info;
    m_client.reset(c->m_client, false);
    ready_for_painting = c->ready_for_painting;
    damage_handle = None;
    damage_region = c->damage_region;
    repaints_region = c->repaints_region;
    layer_repaints_region = c->layer_repaints_region;
    is_shape = c->is_shape;
    effect_window = c->effect_window;
    if (effect_window != NULL)
        effect_window->setWindow(this);
    resource_name = c->resourceName();
    resource_class = c->resourceClass();
    m_clientMachine = c->m_clientMachine;
    m_clientMachine->setParent(this);
    wmClientLeaderWin = c->wmClientLeader();
    opaque_region = c->opaqueRegion();
    m_screen = c->m_screen;
    m_skipCloseAnimation = c->m_skipCloseAnimation;
    m_internalFBO = c->m_internalFBO;
}

// before being deleted, remove references to everything that's now
// owner by Deleted
void Toplevel::disownDataPassedToDeleted()
{
    info = NULL;
}

QRect Toplevel::visibleRect() const
{
    QRect r = bufferGeometry() | frameGeometry();
    if (hasShadow() && !shadow()->shadowRegion().isEmpty()) {
        r |= shadow()->shadowRegion().boundingRect().translated(frameGeometry().topLeft());
    }
    return r;
}

Xcb::Property Toplevel::fetchWmClientLeader() const
{
    return Xcb::Property(false, window(), atoms->wm_client_leader, XCB_ATOM_WINDOW, 0, 10000);
}

void Toplevel::readWmClientLeader(Xcb::Property &prop)
{
    wmClientLeaderWin = prop.value<xcb_window_t>(window());
}

void Toplevel::getWmClientLeader()
{
    auto prop = fetchWmClientLeader();
    readWmClientLeader(prop);
}

/**
 * Returns sessionId for this client,
 * taken either from its window or from the leader window.
 */
QByteArray Toplevel::sessionId() const
{
    QByteArray result = Xcb::StringProperty(window(), atoms->sm_client_id);
    if (result.isEmpty() && wmClientLeaderWin && wmClientLeaderWin != window())
        result = Xcb::StringProperty(wmClientLeaderWin, atoms->sm_client_id);
    return result;
}

/**
 * Returns command property for this client,
 * taken either from its window or from the leader window.
 */
QByteArray Toplevel::wmCommand()
{
    QByteArray result = Xcb::StringProperty(window(), XCB_ATOM_WM_COMMAND);
    if (result.isEmpty() && wmClientLeaderWin && wmClientLeaderWin != window())
        result = Xcb::StringProperty(wmClientLeaderWin, XCB_ATOM_WM_COMMAND);
    result.replace(0, ' ');
    return result;
}

void Toplevel::getWmClientMachine()
{
    m_clientMachine->resolve(window(), wmClientLeader());
}

/**
 * Returns client machine for this client,
 * taken either from its window or from the leader window.
 */
QByteArray Toplevel::wmClientMachine(bool use_localhost) const
{
    if (!m_clientMachine) {
        // this should never happen
        return QByteArray();
    }
    if (use_localhost && m_clientMachine->isLocal()) {
        // special name for the local machine (localhost)
        return ClientMachine::localhost();
    }
    return m_clientMachine->hostName();
}

/**
 * Returns client leader window for this client.
 * Returns the client window itself if no leader window is defined.
 */
Window Toplevel::wmClientLeader() const
{
    if (wmClientLeaderWin)
        return wmClientLeaderWin;
    return window();
}

void Toplevel::getResourceClass()
{
    setResourceClass(QByteArray(info->windowClassName()).toLower(), QByteArray(info->windowClassClass()).toLower());
}

void Toplevel::setResourceClass(const QByteArray &name, const QByteArray &className)
{
    resource_name  = name;
    resource_class = className;
    emit windowClassChanged();
}

double Toplevel::opacity() const
{
    if (info->opacity() == 0xffffffff)
        return 1.0;
    return info->opacity() * 1.0 / 0xffffffff;
}

void Toplevel::setOpacity(double new_opacity)
{
    double old_opacity = opacity();
    new_opacity = qBound(0.0, new_opacity, 1.0);
    if (old_opacity == new_opacity)
        return;
    info->setOpacity(static_cast< unsigned long >(new_opacity * 0xffffffff));
    if (compositing()) {
        addRepaintFull();
        emit opacityChanged(this, old_opacity);
    }
}

bool Toplevel::setupCompositing()
{
    if (!compositing())
        return false;

    if (damage_handle != XCB_NONE)
        return false;

    if (kwinApp()->operationMode() == Application::OperationModeX11 && !surface()) {
        damage_handle = xcb_generate_id(connection());
        xcb_damage_create(connection(), damage_handle, frameId(), XCB_DAMAGE_REPORT_LEVEL_NON_EMPTY);
    }

    const QRect boundingRect = bufferGeometry() | frameGeometry();

    damage_region = QRegion(0, 0, boundingRect.width(), boundingRect.height());
    effect_window = new EffectWindowImpl(this);

    Compositor::self()->scene()->addToplevel(this);

    return true;
}

void Toplevel::finishCompositing(ReleaseReason releaseReason)
{
    if (kwinApp()->operationMode() == Application::OperationModeX11 && damage_handle == XCB_NONE)
        return;
    if (effect_window->window() == this) { // otherwise it's already passed to Deleted, don't free data
        discardWindowPixmap();
        delete effect_window;
    }

    if (damage_handle != XCB_NONE &&
            releaseReason != ReleaseReason::Destroyed) {
        xcb_damage_destroy(connection(), damage_handle);
    }

    damage_handle = XCB_NONE;
    damage_region = QRegion();
    repaints_region = QRegion();
    effect_window = NULL;
}

void Toplevel::discardWindowPixmap()
{
    addDamageFull();
    if (effectWindow() != NULL && effectWindow()->sceneWindow() != NULL)
        effectWindow()->sceneWindow()->pixmapDiscarded();
}

void Toplevel::damageNotifyEvent()
{
    m_isDamaged = true;

    // Note: The rect is supposed to specify the damage extents,
    //       but we don't know it at this point. No one who connects
    //       to this signal uses the rect however.
    emit damaged(this, QRect());
}

bool Toplevel::compositing() const
{
    if (!Workspace::self()) {
        return false;
    }
    return Workspace::self()->compositing();
}

void Client::damageNotifyEvent()
{
    if (syncRequest.isPending && isResize()) {
        emit damaged(this, QRect());
        m_isDamaged = true;
        return;
    }

    if (!ready_for_painting) { // avoid "setReadyForPainting()" function calling overhead
        if (syncRequest.counter == XCB_NONE) {  // cannot detect complete redraw, consider done now
            setReadyForPainting();
            setupWindowManagementInterface();
        }
    }

    Toplevel::damageNotifyEvent();
}

bool Toplevel::resetAndFetchDamage()
{
    if (!m_isDamaged)
        return false;

    if (damage_handle == XCB_NONE) {
        m_isDamaged = false;
        return true;
    }

    xcb_connection_t *conn = connection();

    // Create a new region and copy the damage region to it,
    // resetting the damaged state.
    xcb_xfixes_region_t region = xcb_generate_id(conn);
    xcb_xfixes_create_region(conn, region, 0, 0);
    xcb_damage_subtract(conn, damage_handle, 0, region);

    // Send a fetch-region request and destroy the region
    m_regionCookie = xcb_xfixes_fetch_region_unchecked(conn, region);
    xcb_xfixes_destroy_region(conn, region);

    m_isDamaged = false;
    m_damageReplyPending = true;

    return m_damageReplyPending;
}

void Toplevel::getDamageRegionReply()
{
    if (!m_damageReplyPending)
        return;

    m_damageReplyPending = false;

    // Get the fetch-region reply
    xcb_xfixes_fetch_region_reply_t *reply =
            xcb_xfixes_fetch_region_reply(connection(), m_regionCookie, 0);

    if (!reply)
        return;

    // Convert the reply to a QRegion
    int count = xcb_xfixes_fetch_region_rectangles_length(reply);
    QRegion region;

    if (count > 1 && count < 16) {
        xcb_rectangle_t *rects = xcb_xfixes_fetch_region_rectangles(reply);

        QVector<QRect> qrects;
        qrects.reserve(count);

        for (int i = 0; i < count; i++)
            qrects << QRect(rects[i].x, rects[i].y, rects[i].width, rects[i].height);

        region.setRects(qrects.constData(), count);
    } else
        region += QRect(reply->extents.x, reply->extents.y,
                        reply->extents.width, reply->extents.height);

    damage_region += region;
    repaints_region += region;

    free(reply);
}

void Toplevel::addDamageFull()
{
    if (!compositing())
        return;

    const QRect fullRect = frameGeometry() | bufferGeometry();
    const QRect damagedRect = QRect(0, 0, fullRect.width(), fullRect.height());

    damage_region = damagedRect;
    repaints_region |= damagedRect;

    emit damaged(this, damagedRect);
}

void Toplevel::resetDamage()
{
    damage_region = QRegion();
}

void Toplevel::addRepaint(const QRect& r)
{
    if (!compositing()) {
        return;
    }
    repaints_region += r;
    emit needsRepaint();
}

void Toplevel::addRepaint(int x, int y, int w, int h)
{
    QRect r(x, y, w, h);
    addRepaint(r);
}

void Toplevel::addRepaint(const QRegion& r)
{
    if (!compositing()) {
        return;
    }
    repaints_region += r;
    emit needsRepaint();
}

void Toplevel::addLayerRepaint(const QRect& r)
{
    if (!compositing()) {
        return;
    }
    layer_repaints_region += r;
    emit needsRepaint();
}

void Toplevel::addLayerRepaint(int x, int y, int w, int h)
{
    QRect r(x, y, w, h);
    addLayerRepaint(r);
}

void Toplevel::addLayerRepaint(const QRegion& r)
{
    if (!compositing())
        return;
    layer_repaints_region += r;
    emit needsRepaint();
}

void Toplevel::addRepaintFull()
{
    const QRect rect = frameGeometry() | bufferGeometry();
    repaints_region = visibleRect().translated(-rect.topLeft());
    emit needsRepaint();
}

void Toplevel::resetRepaints()
{
    repaints_region = QRegion();
    layer_repaints_region = QRegion();
}

void Toplevel::addWorkspaceRepaint(int x, int y, int w, int h)
{
    addWorkspaceRepaint(QRect(x, y, w, h));
}

void Toplevel::addWorkspaceRepaint(const QRect& r2)
{
    if (!compositing())
        return;
    Compositor::self()->addRepaint(r2);
}

void Toplevel::setReadyForPainting()
{
    if (!ready_for_painting) {
        ready_for_painting = true;
        if (compositing()) {
            addRepaintFull();
            emit windowShown(this);
            if (auto *cl = dynamic_cast<AbstractClient*>(this)) {
                if (cl->tabGroup() && cl->tabGroup()->current() == cl)
                    cl->tabGroup()->setCurrent(cl, true);
            }
        }
    }
}

void Toplevel::deleteEffectWindow()
{
    delete effect_window;
    effect_window = NULL;
}

void Toplevel::checkScreen()
{
    if (screens()->count() == 1) {
        if (m_screen != 0) {
            m_screen = 0;
            emit screenChanged();
        }
    } else {
        const int s = screens()->number(frameGeometry().center());
        if (s != m_screen) {
            m_screen = s;
            emit screenChanged();
        }
    }
    qreal newScale = screens()->scale(m_screen);
    if (newScale != m_screenScale) {
        m_screenScale = newScale;
        emit screenScaleChanged();
    }
}

void Toplevel::setupCheckScreenConnection()
{
    connect(this, SIGNAL(geometryShapeChanged(KWin::Toplevel*,QRect)), SLOT(checkScreen()));
    connect(this, SIGNAL(geometryChanged()), SLOT(checkScreen()));
}

void Toplevel::removeCheckScreenConnection()
{
    disconnect(this, SIGNAL(geometryShapeChanged(KWin::Toplevel*,QRect)), this, SLOT(checkScreen()));
    disconnect(this, SIGNAL(geometryChanged()), this, SLOT(checkScreen()));
}

int Toplevel::screen() const
{
    return m_screen;
}

qreal Toplevel::screenScale() const
{
    return m_screenScale;
}

bool Toplevel::isOnScreen(int screen) const
{
    return screens()->geometry(screen).intersects(frameGeometry());
}

bool Toplevel::isOnActiveScreen() const
{
    return isOnScreen(screens()->current());
}

void Toplevel::getShadow()
{
    QRect dirtyRect;  // old & new shadow region
    const QRect oldVisibleRect = visibleRect();
    if (hasShadow()) {
        dirtyRect = shadow()->shadowRegion().boundingRect();
        if (!effectWindow()->sceneWindow()->shadow()->updateShadow()) {
            effectWindow()->sceneWindow()->updateShadow(nullptr);
        }
        emit shadowChanged();
    } else {
        Shadow::createShadow(this);
    }
    if (hasShadow())
        dirtyRect |= shadow()->shadowRegion().boundingRect();
    if (oldVisibleRect != visibleRect())
        emit paddingChanged(this, oldVisibleRect);
    if (dirtyRect.isValid()) {
        dirtyRect.translate(pos());
        addLayerRepaint(dirtyRect);
    }
}

bool Toplevel::hasShadow() const
{
    if (effectWindow() && effectWindow()->sceneWindow()) {
        return effectWindow()->sceneWindow()->shadow() != NULL;
    }
    return false;
}

Shadow *Toplevel::shadow()
{
    if (effectWindow() && effectWindow()->sceneWindow()) {
        return effectWindow()->sceneWindow()->shadow();
    } else {
        return NULL;
    }
}

const Shadow *Toplevel::shadow() const
{
    if (effectWindow() && effectWindow()->sceneWindow()) {
        return effectWindow()->sceneWindow()->shadow();
    } else {
        return NULL;
    }
}

bool Toplevel::wantsShadowToBeRendered() const
{
    return true;
}

void Toplevel::getWmOpaqueRegion()
{
    const auto rects = info->opaqueRegion();
    QRegion new_opaque_region;
    for (const auto &r : rects) {
        new_opaque_region += QRect(r.pos.x, r.pos.y, r.size.width, r.size.height);
    }

    opaque_region = new_opaque_region;
}

bool Toplevel::isClient() const
{
    return false;
}

bool Toplevel::isDeleted() const
{
    return false;
}

bool Toplevel::isOnCurrentActivity() const
{
#ifdef KWIN_BUILD_ACTIVITIES
    if (!Activities::self()) {
        return true;
    }
    return isOnActivity(Activities::self()->current());
#else
    return true;
#endif
}

void Toplevel::elevate(bool elevate)
{
    if (!effectWindow()) {
        return;
    }
    effectWindow()->elevate(elevate);
    addWorkspaceRepaint(visibleRect());
}

pid_t Toplevel::pid() const
{
    return info->pid();
}

xcb_window_t Toplevel::frameId() const
{
    return m_client;
}

Xcb::Property Toplevel::fetchSkipCloseAnimation() const
{
    return Xcb::Property(false, window(), atoms->kde_skip_close_animation, XCB_ATOM_CARDINAL, 0, 1);
}

void Toplevel::readSkipCloseAnimation(Xcb::Property &property)
{
    setSkipCloseAnimation(property.toBool());
}

void Toplevel::getSkipCloseAnimation()
{
    Xcb::Property property = fetchSkipCloseAnimation();
    readSkipCloseAnimation(property);
}

bool Toplevel::skipsCloseAnimation() const
{
    return m_skipCloseAnimation;
}

void Toplevel::setSkipCloseAnimation(bool set)
{
    if (set == m_skipCloseAnimation) {
        return;
    }
    m_skipCloseAnimation = set;
    emit skipCloseAnimationChanged();
}

void Toplevel::setSurface(KWayland::Server::SurfaceInterface *surface)
{
    if (m_surface == surface) {
        return;
    }
    using namespace KWayland::Server;
    if (m_surface) {
        disconnect(m_surface, &SurfaceInterface::damaged, this, &Toplevel::addDamage);
        disconnect(m_surface, &SurfaceInterface::sizeChanged, this, &Toplevel::discardWindowPixmap);
    }
    m_surface = surface;
    connect(m_surface, &SurfaceInterface::damaged, this, &Toplevel::addDamage);
    connect(m_surface, &SurfaceInterface::sizeChanged, this, &Toplevel::discardWindowPixmap);
    connect(m_surface, &SurfaceInterface::subSurfaceTreeChanged, this,
        [this] {
            // TODO improve to only update actual visual area
            if (ready_for_painting) {
                addDamageFull();
                m_isDamaged = true;
            }
        }
    );
    connect(m_surface, &SurfaceInterface::destroyed, this,
        [this] {
            m_surface = nullptr;
        }
    );
    emit surfaceChanged();
}

void Toplevel::addDamage(const QRegion &damage)
{
    m_isDamaged = true;
    damage_region += damage;
    for (const QRect &r : damage) {
        emit damaged(this, r);
    }
}

QByteArray Toplevel::windowRole() const
{
    return QByteArray(info->windowRole());
}

void Toplevel::setDepth(int depth)
{
    if (bit_depth == depth) {
        return;
    }
    const bool oldAlpha = hasAlpha();
    bit_depth = depth;
    if (oldAlpha != hasAlpha()) {
        emit hasAlphaChanged();
    }
}

QRegion Toplevel::inputShape() const
{
    if (m_surface) {
        return m_surface->input();
    } else {
        // TODO: maybe also for X11?
        return QRegion();
    }
}

void Toplevel::setInternalFramebufferObject(const QSharedPointer<QOpenGLFramebufferObject> &fbo)
{
    if (m_internalFBO != fbo) {
        discardWindowPixmap();
        m_internalFBO = fbo;
    }
    setDepth(32);
}

QMatrix4x4 Toplevel::inputTransformation() const
{
    QMatrix4x4 m;
    m.translate(-x(), -y());
    return m;
}

quint32 Toplevel::windowId() const
{
    return window();
}

QRect Toplevel::inputGeometry() const
{
    return bufferGeometry();
}

bool Toplevel::isLocalhost() const
{
    if (!m_clientMachine) {
        return true;
    }
    return m_clientMachine->isLocal();
}

QSize Toplevel::size() const
{
    return frameGeometry().size();
}

QPoint Toplevel::pos() const
{
    return frameGeometry().topLeft();
}

int Toplevel::x() const
{
    return frameGeometry().x();
}

int Toplevel::y() const
{
    return frameGeometry().y();
}

int Toplevel::width() const
{
    return frameGeometry().width();
}

int Toplevel::height() const
{
    return frameGeometry().height();
}

QRect Toplevel::rect() const
{
    return QRect(0, 0, width(), height());
}

} // namespace

