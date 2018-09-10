/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006 Lubos Lunak <l.lunak@kde.org>
Copyright (C) 2009 Lucas Murray <lmurray@undefinedfire.com>
Copyright (C) 2010, 2011 Martin Gräßlin <mgraesslin@kde.org>
Copyright (C) 2018 Vlad Zagorodniy <vladzzag@gmail.com>

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

#pragma once

#include <kwineffects.h>

#include <QJSEngine>
#include <QJSValue>

namespace KWin
{

class EffectWindowWrapper;

class EffectsHandlerWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int currentDesktop READ currentDesktop WRITE setCurrentDesktop NOTIFY desktopChanged)
    Q_PROPERTY(QString currentActivity READ currentActivity NOTIFY currentActivityChanged)
    Q_PROPERTY(QJSValue activeWindow READ activeWindow WRITE activateWindow NOTIFY windowActivated)
    Q_PROPERTY(QSize desktopGridSize READ desktopGridSize)
    Q_PROPERTY(int desktopGridWidth READ desktopGridWidth)
    Q_PROPERTY(int desktopGridHeight READ desktopGridHeight)
    Q_PROPERTY(int workspaceWidth READ workspaceWidth)
    Q_PROPERTY(int workspaceHeight READ workspaceHeight)
    Q_PROPERTY(int desktops READ numberOfDesktops WRITE setNumberOfDesktops NOTIFY numberDesktopsChanged)
    Q_PROPERTY(bool optionRollOverDesktops READ optionRollOverDesktops)
    Q_PROPERTY(int activeScreen READ activeScreen)
    Q_PROPERTY(int numScreens READ numScreens NOTIFY numberScreensChanged)
    Q_PROPERTY(qreal animationTimeFactor READ animationTimeFactor)
    Q_PROPERTY(QJSValue stackingOrder READ stackingOrder)
    Q_PROPERTY(bool decorationsHaveAlpha READ decorationsHaveAlpha)
    Q_PROPERTY(bool decorationSupportsBlurBehind READ decorationSupportsBlurBehind)
    Q_PROPERTY(CompositingType compositingType READ compositingType CONSTANT)
    Q_PROPERTY(QPoint cursorPos READ cursorPos)
    Q_PROPERTY(QSize virtualScreenSize READ virtualScreenSize NOTIFY virtualScreenSizeChanged)
    Q_PROPERTY(QRect virtualScreenGeometry READ virtualScreenGeometry NOTIFY virtualScreenGeometryChanged)

public:
    EffectsHandlerWrapper(QJSEngine *engine, EffectsHandler *wrapped);
    ~EffectsHandlerWrapper() override;

    EffectWindowWrapper *findWrappedWindow(EffectWindow *w) const;

    Q_INVOKABLE void moveWindow(QJSValue w, const QPoint &pos, bool snap = false, double snapAdjust = 1.0);
    Q_INVOKABLE void windowToDesktop(QJSValue w, int desktop);
    Q_INVOKABLE void windowToScreen(QJSValue w, int screen);

    Q_INVOKABLE int desktopAbove(int desktop = 0, bool wrap = true) const;
    Q_INVOKABLE int desktopToRight(int desktop = 0, bool wrap = true) const;
    Q_INVOKABLE int desktopBelow(int desktop = 0, bool wrap = true) const;
    Q_INVOKABLE int desktopToLeft(int desktop = 0, bool wrap = true) const;
    Q_INVOKABLE QString desktopName(int desktop) const;

    Q_INVOKABLE int screenNumber(const QPoint &pos) const;

    Q_INVOKABLE QJSValue findWindow(WId id) const;
    Q_INVOKABLE QJSValue findWindow(KWayland::Server::SurfaceInterface *surf) const;

    Q_INVOKABLE void setElevatedWindow(QJSValue w, bool set);

    Q_INVOKABLE void addRepaintFull();
    Q_INVOKABLE void addRepaint(const QRect &r);
    Q_INVOKABLE void addRepaint(const QRegion &r);
    Q_INVOKABLE void addRepaint(int x, int y, int w, int h);

    int currentDesktop() const;
    void setCurrentDesktop(int desktop);

    QString currentActivity() const;

    QJSValue activeWindow() const;
    void activateWindow(QJSValue w);

    QSize desktopGridSize() const;
    int desktopGridWidth() const;
    int desktopGridHeight() const;

    int workspaceWidth() const;
    int workspaceHeight() const;

    int numberOfDesktops() const;
    void setNumberOfDesktops(int desktops);
    bool optionRollOverDesktops() const;

    int activeScreen() const;
    int numScreens() const;

    qreal animationTimeFactor() const;

    QJSValue stackingOrder() const;

    bool decorationsHaveAlpha() const;
    bool decorationSupportsBlurBehind() const;

    CompositingType compositingType() const;

    QPoint cursorPos() const;

    QSize virtualScreenSize() const;
    QRect virtualScreenGeometry() const;

Q_SIGNALS:
    /**
     * Signal emitted when the current desktop changed.
     * @param oldDesktop The previously current desktop
     * @param newDesktop The new current desktop
     * @param with The window which is taken over to the new desktop, can be NULL
     * @since 4.9
     */
    void desktopChanged(int oldDesktop, int newDesktop, QJSValue with);

    /**
     * Signal emitted when a window moved to another desktop
     * NOTICE that this does NOT imply that the desktop has changed
     * The @param window which is moved to the new desktop
     * @param oldDesktop The previous desktop of the window
     * @param newDesktop The new desktop of the window
     * @since 4.11.4
     */
    void desktopPresenceChanged(QJSValue window, int oldDesktop, int newDesktop);

    /**
    * Signal emitted when the number of currently existing desktops is changed.
    * @param old The previous number of desktops in used.
    * @see EffectsHandler::numberOfDesktops.
    * @since 4.7
    */
    void numberDesktopsChanged(uint old);

    /**
     * Signal emitted when the number of screens changed.
     * @since 5.0
     **/
    void numberScreensChanged();

    /**
     * Signal emitted when the desktop showing ("dashboard") state changed
     * The desktop is risen to the keepAbove layer, you may want to elevate
     * windows or such.
     * @since 5.3
     **/
    void showingDesktopChanged(bool);

    /**
     * Signal emitted when a new window has been added to the Workspace.
     * @param w The added window
     * @since 4.7
     **/
    void windowAdded(QJSValue w);

    /**
     * Signal emitted when a window is being removed from the Workspace.
     * An effect which wants to animate the window closing should connect
     * to this signal and reference the window by using
     * @link EffectWindow::refWindow
     * @param w The window which is being closed
     * @since 4.7
     **/
    void windowClosed(QJSValue w);

    /**
     * Signal emitted when a window get's activated.
     * @param w The new active window, or @c NULL if there is no active window.
     * @since 4.7
     **/
    void windowActivated(QJSValue w);

    /**
     * Signal emitted when a window is deleted.
     * This means that a closed window is not referenced any more.
     * An effect bookkeeping the closed windows should connect to this
     * signal to clean up the internal references.
     * @param w The window which is going to be deleted.
     * @see EffectWindow::refWindow
     * @see EffectWindow::unrefWindow
     * @see windowClosed
     * @since 4.7
     **/
    void windowDeleted(QJSValue w);

    /**
     * Signal emitted when a user begins a window move or resize operation.
     * To figure out whether the user resizes or moves the window use
     * @link EffectWindow::isUserMove or @link EffectWindow::isUserResize.
     * Whenever the geometry is updated the signal @link windowStepUserMovedResized
     * is emitted with the current geometry.
     * The move/resize operation ends with the signal @link windowFinishUserMovedResized.
     * Only one window can be moved/resized by the user at the same time!
     * @param w The window which is being moved/resized
     * @see windowStepUserMovedResized
     * @see windowFinishUserMovedResized
     * @see EffectWindow::isUserMove
     * @see EffectWindow::isUserResize
     * @since 4.7
     **/
    void windowStartUserMovedResized(QJSValue w);

    /**
     * Signal emitted during a move/resize operation when the user changed the geometry.
     * Please note: KWin supports two operation modes. In one mode all changes are applied
     * instantly. This means the window's geometry matches the passed in @p geometry. In the
     * other mode the geometry is changed after the user ended the move/resize mode.
     * The @p geometry differs from the window's geometry. Also the window's pixmap still has
     * the same size as before. Depending what the effect wants to do it would be recommended
     * to scale/translate the window.
     * @param w The window which is being moved/resized
     * @param geometry The geometry of the window in the current move/resize step.
     * @see windowStartUserMovedResized
     * @see windowFinishUserMovedResized
     * @see EffectWindow::isUserMove
     * @see EffectWindow::isUserResize
     * @since 4.7
     **/
    void windowStepUserMovedResized(QJSValue w, const QRect &geometry);

    /**
     * Signal emitted when the user finishes move/resize of window @p w.
     * @param w The window which has been moved/resized
     * @see windowStartUserMovedResized
     * @see windowFinishUserMovedResized
     * @since 4.7
     **/
    void windowFinishUserMovedResized(QJSValue w);

    /**
     * Signal emitted when the maximized state of the window @p w changed.
     * A window can be in one of four states:
     * @li restored: both @p horizontal and @p vertical are @c false
     * @li horizontally maximized: @p horizontal is @c true and @p vertical is @c false
     * @li vertically maximized: @p horizontal is @c false and @p vertical is @c true
     * @li completely maximized: both @p horizontal and @p vertical are @C true
     * @param w The window whose maximized state changed
     * @param horizontal If @c true maximized horizontally
     * @param vertical If @c true maximized vertically
     * @since 4.7
     **/
    void windowMaximizedStateChanged(QJSValue w, bool horizontal, bool vertical);

    /**
     * Signal emitted when the geometry or shape of a window changed.
     * This is caused if the window changes geometry without user interaction.
     * E.g. the decoration is changed. This is in opposite to windowUserMovedResized
     * which is caused by direct user interaction.
     * @param w The window whose geometry changed
     * @param old The previous geometry
     * @see windowUserMovedResized
     * @since 4.7
     **/
    void windowGeometryShapeChanged(QJSValue w, const QRect &old);

    /**
     * Signal emitted when the padding of a window changed. (eg. shadow size)
     * @param w The window whose geometry changed
     * @param old The previous expandedGeometry()
     * @since 4.9
     **/
    void windowPaddingChanged(QJSValue w, const QRect &old);

    /**
     * Signal emitted when the windows opacity is changed.
     * @param w The window whose opacity level is changed.
     * @param oldOpacity The previous opacity level
     * @param newOpacity The new opacity level
     * @since 4.7
     **/
    void windowOpacityChanged(QJSValue w, qreal oldOpacity, qreal newOpacity);

    /**
     * Signal emitted when a window got minimized.
     * @param w The window which was minimized
     * @since 4.7
     **/
    void windowMinimized(QJSValue w);

    /**
     * Signal emitted when a window got unminimized.
     * @param w The window which was unminimized
     * @since 4.7
     **/
    void windowUnminimized(QJSValue w);

    /**
     * Signal emitted when a window either becomes modal (ie. blocking for its main client) or looses that state.
     * @param w The window which was unminimized
     * @since 4.11
     **/
    void windowModalityChanged(QJSValue w);

    /**
     * Signal emitted when a window either became unresponsive (eg. app froze or crashed)
     * or respoonsive
     * @param w The window that became (un)responsive
     * @param unresponsive Whether the window is responsive or unresponsive
     * @since 5.10
     */
    void windowUnresponsiveChanged(QJSValue w, bool unresponsive);

    /**
     * Signal emitted when an area of a window is scheduled for repainting.
     * Use this signal in an effect if another area needs to be synced as well.
     * @param w The window which is scheduled for repainting
     * @param r Always empty.
     * @since 4.7
     **/
    void windowDamaged(QJSValue w, const QRect &r);

    /**
     * Signal emitted when a tabbox is added.
     * An effect who wants to replace the tabbox with itself should use @link refTabBox.
     * @param mode The TabBoxMode.
     * @see refTabBox
     * @see tabBoxClosed
     * @see tabBoxUpdated
     * @see tabBoxKeyEvent
     * @since 4.7
     **/
    void tabBoxAdded(int mode);

    /**
     * Signal emitted when the TabBox was closed by KWin core.
     * An effect which referenced the TabBox should use @link unrefTabBox to unref again.
     * @see unrefTabBox
     * @see tabBoxAdded
     * @since 4.7
     **/
    void tabBoxClosed();

    /**
     * Signal emitted when the selected TabBox window changed or the TabBox List changed.
     * An effect should only response to this signal if it referenced the TabBox with @link refTabBox.
     * @see refTabBox
     * @see currentTabBoxWindowList
     * @see currentTabBoxDesktopList
     * @see currentTabBoxWindow
     * @see currentTabBoxDesktop
     * @since 4.7
     **/
    void tabBoxUpdated();

    /**
     * Signal emitted when a key event, which is not handled by TabBox directly is, happens while
     * TabBox is active. An effect might use the key event to e.g. change the selected window.
     * An effect should only response to this signal if it referenced the TabBox with @link refTabBox.
     * @param event The key event not handled by TabBox directly
     * @see refTabBox
     * @since 4.7
     **/
    void tabBoxKeyEvent(QKeyEvent* event);
    void currentTabAboutToChange(QJSValue from, QJSValue to);
    void tabAdded(QJSValue from, QJSValue to);   // from merged with to
    void tabRemoved(QJSValue c, QJSValue group);   // c removed from group

    /**
     * Signal emitted when mouse changed.
     * If an effect needs to get updated mouse positions, it needs to first call @link startMousePolling.
     * For a fullscreen effect it is better to use an input window and react on @link windowInputMouseEvent.
     * @param pos The new mouse position
     * @param oldpos The previously mouse position
     * @param buttons The pressed mouse buttons
     * @param oldbuttons The previously pressed mouse buttons
     * @param modifiers Pressed keyboard modifiers
     * @param oldmodifiers Previously pressed keyboard modifiers.
     * @see startMousePolling
     * @since 4.7
     **/
    void mouseChanged(const QPoint &pos, const QPoint &oldPos,
                      Qt::MouseButtons buttons, Qt::MouseButtons oldButtons,
                      Qt::KeyboardModifiers modifiers, Qt::KeyboardModifiers oldModifiers);

    /**
     * Signal emitted when the cursor shape changed.
     * You'll likely want to query the current cursor as reaction: xcb_xfixes_get_cursor_image_unchecked
     * Connection to this signal is tracked, so if you don't need it anymore, disconnect from it to stop cursor event filtering
     */
    void cursorShapeChanged();

    /**
     * Receives events registered for using @link registerPropertyType.
     * Use readProperty() to get the property data.
     * Note that the property may be already set on the window, so doing the same
     * processing from windowAdded() (e.g. simply calling propertyNotify() from it)
     * is usually needed.
     * @param w The window whose property changed, is @c null if it is a root window property
     * @param atom The property
     * @since 4.7
     */
    void propertyNotify(QJSValue w, long atom);

    /**
     * Signal emitted after the screen geometry changed (e.g. add of a monitor).
     * Effects using displayWidth()/displayHeight() to cache information should
     * react on this signal and update the caches.
     * @param size The new screen size
     * @since 4.8
     **/
    void screenGeometryChanged(const QSize &size);

    /**
     * This signal is emitted when the global
     * activity is changed
     * @param id id of the new current activity
     * @since 4.9
     **/
    void currentActivityChanged(const QString &id);

    /**
     * This signal is emitted when a new activity is added
     * @param id id of the new activity
     * @since 4.9
     */
    void activityAdded(const QString &id);

    /**
     * This signal is emitted when the activity
     * is removed
     * @param id id of the removed activity
     * @since 4.9
     */
    void activityRemoved(const QString &id);

    /**
     * This signal is emitted when the screen got locked or unlocked.
     * @param locked @c true if the screen is now locked, @c false if it is now unlocked
     * @since 4.11
     **/
    void screenLockingChanged(bool locked);

    /**
     * This signels is emitted when ever the stacking order is change, ie. a window is risen
     * or lowered
     * @since 4.10
     */
    void stackingOrderChanged();

    /**
     * This signal is emitted when the user starts to approach the @p border with the mouse.
     * The @p factor describes how far away the mouse is in a relative mean. The values are in
     * [0.0, 1.0] with 0.0 being emitted when first entered and on leaving. The value 1.0 means that
     * the @p border is reached with the mouse. So the values are well suited for animations.
     * The signal is always emitted when the mouse cursor position changes.
     * @param border The screen edge which is being approached
     * @param factor Value in range [0.0,1.0] to describe how close the mouse is to the border
     * @param geometry The geometry of the edge which is being approached
     * @since 4.11
     **/
    void screenEdgeApproaching(ElectricBorder border, qreal factor, const QRect &geometry);

    /**
     * Emitted whenever the virtualScreenSize changes.
     * @see virtualScreenSize()
     * @since 5.0
     **/
    void virtualScreenSizeChanged();

    /**
     * Emitted whenever the virtualScreenGeometry changes.
     * @see virtualScreenGeometry()
     * @since 5.0
     **/
    void virtualScreenGeometryChanged();

    /**
     * The window @p w gets shown again. The window was previously
     * initially shown with @link{windowAdded} and hidden with @link{windowHidden}.
     *
     * @see windowHidden
     * @see windowAdded
     * @since 5.8
     **/
    void windowShown(QJSValue w);

    /**
     * The window @p w got hidden but not yet closed.
     * This can happen when a window is still being used and is supposed to be shown again
     * with @link{windowShown}. On X11 an example is autohiding panels. On Wayland every
     * window first goes through the window hidden state and might get shown again, or might
     * get closed the normal way.
     *
     * @see windowShown
     * @see windowClosed
     * @since 5.8
     **/
    void windowHidden(QJSValue w);

    /**
     * This signal gets emitted when the data on EffectWindow @p w for @p role changed.
     *
     * An Effect can connect to this signal to read the new value and react on it.
     * E.g. an Effect which does not operate on windows grabbed by another Effect wants
     * to cancel the already scheduled animation if another Effect adds a grab.
     *
     * @param w The EffectWindow for which the data changed
     * @param role The data role which changed
     * @see EffectWindow::setData
     * @see EffectWindow::data
     * @since 5.8.4
     **/
    void windowDataChanged(QJSValue w, int role);

    /**
     * The xcb connection changed, either a new xcbConnection got created or the existing one
     * got destroyed.
     * Effects can use this to refetch the properties they want to set.
     *
     * When the xcbConnection changes also the @link{x11RootWindow} becomes invalid.
     * @see xcbConnection
     * @see x11RootWindow
     * @since 5.11
     **/
    void xcbConnectionChanged();

    /**
     * This signal is emitted when active fullscreen effect changed.
     *
     * @see activeFullScreenEffect
     * @see setActiveFullScreenEffect
     * @since 5.14
     **/
    void activeFullScreenEffectChanged();

private:
    QJSEngine *m_engine;
    EffectsHandler *m_wrapped;
    mutable QHash<const EffectWindow *, EffectWindowWrapper *> m_wrappedWindows;

    Q_DISABLE_COPY(EffectsHandlerWrapper)
    friend class EffectWindowWrapper;
};

} // namespace KWin
