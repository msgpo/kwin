/*
 * Copyright (C) 2015 Martin Flöser <mgraesslin@kde.org>
 * Copyright (C) 2018 David Edmundson <davidedmundson@kde.org>
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

#include "shellsurfaceclient.h"
#include "screens.h"
#include "wayland_server.h"
#include "workspace.h"

#include <KWayland/Server/display.h>
#include <KWayland/Server/clientconnection.h>
#include <KWayland/Server/surface_interface.h>

#include <QFileInfo>

#include <csignal>

#include <sys/types.h>
#include <unistd.h>

using namespace KWayland::Server;

namespace KWin
{

ShellSurfaceClient::ShellSurfaceClient(SurfaceInterface *surface)
{
    // Note that we cannot setup compositing here because we may need to call visibleRect(),
    // which in its turn will call bufferGeometry(), which is a pure virtual method.

    setSurface(surface);

    m_windowId = waylandServer()->createWindowId(surface);

    connect(this, &ShellSurfaceClient::frameGeometryChanged,
            this, &ShellSurfaceClient::updateClientOutputs);
    connect(this, &ShellSurfaceClient::frameGeometryChanged,
            this, &ShellSurfaceClient::updateClientArea);
    connect(this, &ShellSurfaceClient::desktopFileNameChanged,
            this, &ShellSurfaceClient::updateIcon);
    connect(screens(), &Screens::changed, this,
            &ShellSurfaceClient::updateClientOutputs);

    updateResourceName();
    updateIcon();
}

QString ShellSurfaceClient::captionNormal() const
{
    return m_captionNormal;
}

QString ShellSurfaceClient::captionSuffix() const
{
    return m_captionSuffix;
}

QStringList ShellSurfaceClient::activities() const
{
    return QStringList();
}

void ShellSurfaceClient::setOnAllActivities(bool set)
{
    Q_UNUSED(set)
}

void ShellSurfaceClient::blockActivityUpdates(bool b)
{
    Q_UNUSED(b)
}

QPoint ShellSurfaceClient::clientContentPos() const
{
    return -clientPos();
}

QRect ShellSurfaceClient::transparentRect() const
{
    return QRect();
}

quint32 ShellSurfaceClient::windowId() const
{
    return m_windowId;
}

pid_t ShellSurfaceClient::pid() const
{
    return surface()->client()->processId();
}

bool ShellSurfaceClient::isLockScreen() const
{
    return surface()->client() == waylandServer()->screenLockerClientConnection();
}

bool ShellSurfaceClient::isInputMethod() const
{
    return surface()->client() == waylandServer()->inputMethodConnection();
}

bool ShellSurfaceClient::isLocalhost() const
{
    return true;
}

double ShellSurfaceClient::opacity() const
{
    return m_opacity;
}

void ShellSurfaceClient::setOpacity(double opacity)
{
    const qreal newOpacity = qBound(0.0, opacity, 1.0);
    if (newOpacity == m_opacity) {
        return;
    }
    const qreal oldOpacity = m_opacity;
    m_opacity = newOpacity;
    addRepaintFull();
    emit opacityChanged(this, oldOpacity);
}

AbstractClient *ShellSurfaceClient::findModal(bool allow_itself)
{
    Q_UNUSED(allow_itself)
    return nullptr;
}

void ShellSurfaceClient::resizeWithChecks(int w, int h, ForceGeometry_t force)
{
    const QRect area = workspace()->clientArea(WorkArea, this);
    // don't allow growing larger than workarea
    if (w > area.width()) {
        w = area.width();
    }
    if (h > area.height()) {
        h = area.height();
    }
    setFrameGeometry(x(), y(), w, h, force);
}

void ShellSurfaceClient::killWindow()
{
    if (!surface()) {
        return;
    }
    auto c = surface()->client();
    if (c->processId() == getpid() || c->processId() == 0) {
        c->destroy();
        return;
    }
    ::kill(c->processId(), SIGTERM);
    // give it time to terminate and only if terminate fails, try destroy Wayland connection
    QTimer::singleShot(5000, c, &ClientConnection::destroy);
}

QByteArray ShellSurfaceClient::windowRole() const
{
    return QByteArray();
}

bool ShellSurfaceClient::belongsToSameApplication(const AbstractClient *other, SameApplicationChecks checks) const
{
    if (checks.testFlag(SameApplicationCheck::AllowCrossProcesses)) {
        if (other->desktopFileName() == desktopFileName()) {
            return true;
        }
    }
    if (auto s = other->surface()) {
        return s->client() == surface()->client();
    }
    return false;
}

bool ShellSurfaceClient::belongsToDesktop() const
{
    const auto clients = waylandServer()->clients();

    return std::any_of(clients.constBegin(), clients.constEnd(),
        [this](const AbstractClient *client) {
            if (belongsToSameApplication(client, SameApplicationChecks())) {
                return client->isDesktop();
            }
            return false;
        }
    );
}

void ShellSurfaceClient::updateClientArea()
{
    if (hasStrut()) {
        workspace()->updateClientArea();
    }
}

void ShellSurfaceClient::updateClientOutputs()
{
    QVector<OutputInterface *> clientOutputs;
    const auto outputs = waylandServer()->display()->outputs();
    for (OutputInterface *output : outputs) {
        const QRect outputGeometry(output->globalPosition(), output->pixelSize() / output->scale());
        if (frameGeometry().intersects(outputGeometry)) {
            clientOutputs << output;
        }
    }
    surface()->setOutputs(clientOutputs);
}

void ShellSurfaceClient::updateIcon()
{
    const QString waylandIconName = QStringLiteral("wayland");
    const QString dfIconName = iconFromDesktopFile();
    const QString iconName = dfIconName.isEmpty() ? waylandIconName : dfIconName;
    if (iconName == icon().name()) {
        return;
    }
    setIcon(QIcon::fromTheme(iconName));
}

void ShellSurfaceClient::updateResourceName()
{
    const QFileInfo fileInfo(surface()->client()->executablePath());
    if (fileInfo.exists()) {
        setResourceClass(fileInfo.fileName().toUtf8());
    }
}

void ShellSurfaceClient::updateCaption()
{
    const QString oldSuffix = m_captionSuffix;
    const auto shortcut = shortcutCaptionSuffix();
    m_captionSuffix = shortcut;
    if ((!isSpecialWindow() || isToolbar()) && findClientWithSameCaption()) {
        int i = 2;
        do {
            m_captionSuffix = shortcut + QLatin1String(" <") + QString::number(i) + QLatin1Char('>');
            i++;
        } while (findClientWithSameCaption());
    }
    if (m_captionSuffix != oldSuffix) {
        emit captionChanged();
    }
}

void ShellSurfaceClient::setCaption(const QString &caption)
{
    const QString oldSuffix = m_captionSuffix;
    m_captionNormal = caption.simplified();
    updateCaption();
    if (m_captionSuffix == oldSuffix) {
        // Don't emit caption change twice it already got emitted by the changing suffix.
        emit captionChanged();
    }
}

} // namespace KWin
