/********************************************************************
KWin - the KDE window manager
This file is part of the KDE project.

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

#include <algorithm>

#include <QDir>
#include <QObject>
#include <QVector>

#include <KDecoration2/Decoration>
#include <KDecoration2/DecorationShadow>

#include <KWayland/Client/server_decoration.h>
#include <KWayland/Client/shell.h>
#include <KWayland/Client/surface.h>

#include "kwin_wayland_test.h"

#include "composite.h"
#include "effect_builtins.h"
#include "effectloader.h"
#include "effects.h"
#include "platform.h"
#include "shadow.h"
#include "shell_client.h"
#include "wayland_server.h"
#include "workspace.h"

using namespace KWin;
using namespace KWayland::Client;

static const QString s_socketName = QStringLiteral("wayland_test_kwin_scene_opengl_shadow-0");

class SceneOpenGLShadowTest : public QObject
{
    Q_OBJECT
public:
    SceneOpenGLShadowTest() {}

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();

    void testShadowTileOverlaps_data();
    void testShadowTileOverlaps();

};


///////////////////////////////////////////////////////////
// Helpers.
///////////////////////////////////////////////////////////

inline bool compareDoubles(double a, double b, double eps = 1e-5)
{
    if (a == b) {
        return true;
    }
    double diff = std::fabs(a - b);
    if (a == 0 || b == 0) {
        return diff < eps;
    }
    return diff / std::max(a, b) < eps;
}

inline bool compareQuads(const WindowQuad &a, const WindowQuad &b)
{
    for (int i = 0; i < 4; i++) {
        if (compareDoubles(a[i].x(), b[i].x())
                && compareDoubles(a[i].y(), b[i].y())
                && compareDoubles(a[i].u(), b[i].u())
                && compareDoubles(a[i].v(), b[i].v())) {
            return true;
        }
    }
    return false;
}

inline WindowQuad makeShadowQuad(const QRectF &geo, qreal tx1, qreal ty1, qreal tx2, qreal ty2)
{
    WindowQuad quad(WindowQuadShadow);
    quad[0] = WindowVertex(geo.left(),  geo.top(),    tx1, ty1);
    quad[1] = WindowVertex(geo.right(), geo.top(),    tx2, ty1);
    quad[2] = WindowVertex(geo.right(), geo.bottom(), tx2, ty2);
    quad[3] = WindowVertex(geo.left(),  geo.bottom(), tx1, ty2);
    return quad;
}

// Need this one because WindowQuadList isn't registered with Q_DECLARE_METATYPE
class WindowQuadListWrapper
{
public:
    WindowQuadListWrapper() {}
    WindowQuadListWrapper(const WindowQuadList &quadList)
        : m_quadList(quadList) {}
    WindowQuadListWrapper(const WindowQuadListWrapper &other)
        : m_quadList(other.m_quadList) {}
    ~WindowQuadListWrapper() {}

    WindowQuadList &quadList() { return m_quadList; }

private:
    WindowQuadList m_quadList;
};
Q_DECLARE_METATYPE(WindowQuadListWrapper);

///////////////////////////////////////////////////////////
// End of helpers.
///////////////////////////////////////////////////////////

void SceneOpenGLShadowTest::initTestCase()
{
    // Copied from generic_scene_opengl_test.cpp

    if (!QFile::exists(QStringLiteral("/dev/dri/card0"))) {
        QSKIP("Needs a dri device");
    }
    qRegisterMetaType<KWin::ShellClient*>();
    qRegisterMetaType<KWin::AbstractClient*>();
    QSignalSpy workspaceCreatedSpy(kwinApp(), &Application::workspaceCreated);
    QVERIFY(workspaceCreatedSpy.isValid());
    kwinApp()->platform()->setInitialWindowSize(QSize(1280, 1024));
    QVERIFY(waylandServer()->init(s_socketName.toLocal8Bit()));

    // disable all effects - we don't want to have it interact with the rendering
    auto config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    KConfigGroup plugins(config, QStringLiteral("Plugins"));
    ScriptedEffectLoader loader;
    const auto builtinNames = BuiltInEffects::availableEffectNames() << loader.listOfKnownEffects();
    for (QString name : builtinNames) {
        plugins.writeEntry(name + QStringLiteral("Enabled"), false);
    }

    config->sync();
    kwinApp()->setConfig(config);

    qputenv("XCURSOR_THEME", QByteArrayLiteral("DMZ-White"));
    qputenv("XCURSOR_SIZE", QByteArrayLiteral("24"));
    qputenv("KWIN_COMPOSE", QByteArrayLiteral("O2"));

    kwinApp()->start();
    QVERIFY(workspaceCreatedSpy.wait());
    QVERIFY(KWin::Compositor::self());

    // Add directory with fake decorations to the plugin search path.
    QCoreApplication::addLibraryPath(
        QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("fakes")
    );

    // Change decoration theme.
    KConfigGroup group = kwinApp()->config()->group("org.kde.kdecoration2");
    group.writeEntry("library", "org.kde.test.fakedecowithshadows");
    group.sync();
    Workspace::self()->slotReconfigure();
}

void SceneOpenGLShadowTest::init()
{
    QVERIFY(Test::setupWaylandConnection(Test::AdditionalWaylandInterface::Decoration));
}

void SceneOpenGLShadowTest::cleanup()
{
    Test::destroyWaylandConnection();
}

namespace {
    const int SHADOW_SIZE = 128;

    const int SHADOW_OFFSET_TOP  = 64;
    const int SHADOW_OFFSET_LEFT = 48;

    // NOTE: We assume deco shadows are generated with blur so that's
    //       why there is 4, 1 is the size of the inner shadow rect.
    const int SHADOW_TEXTURE_WIDTH  = 4 * SHADOW_SIZE + 1;
    const int SHADOW_TEXTURE_HEIGHT = 4 * SHADOW_SIZE + 1;

    const int SHADOW_PADDING_TOP    = SHADOW_SIZE - SHADOW_OFFSET_TOP;
    const int SHADOW_PADDING_RIGHT  = SHADOW_SIZE + SHADOW_OFFSET_LEFT;
    const int SHADOW_PADDING_BOTTOM = SHADOW_SIZE + SHADOW_OFFSET_TOP;
    const int SHADOW_PADDING_LEFT   = SHADOW_SIZE - SHADOW_OFFSET_LEFT;

    const QRectF SHADOW_INNER_RECT(2 * SHADOW_SIZE, 2 * SHADOW_SIZE, 1, 1);
}

void SceneOpenGLShadowTest::testShadowTileOverlaps_data()
{
    QTest::addColumn<QSize>("windowSize");
    QTest::addColumn<WindowQuadListWrapper>("expectedQuadsWrapper");

    // Precompute shadow tile geometries(in texture's space).
    QRectF topLeftTile(
        0,
        0,
        SHADOW_INNER_RECT.x(),
        SHADOW_INNER_RECT.y());
    QRectF topRightTile(
        SHADOW_INNER_RECT.right(),
        0,
        SHADOW_TEXTURE_WIDTH - SHADOW_INNER_RECT.right(),
        SHADOW_INNER_RECT.y());
    QRectF topTile(topLeftTile.topRight(), topRightTile.bottomLeft());

    QRectF bottomLeftTile(
        0,
        SHADOW_INNER_RECT.bottom(),
        SHADOW_INNER_RECT.x(),
        SHADOW_TEXTURE_HEIGHT - SHADOW_INNER_RECT.bottom());
    QRectF bottomRightTile(
        SHADOW_INNER_RECT.right(),
        SHADOW_INNER_RECT.bottom(),
        SHADOW_TEXTURE_WIDTH - SHADOW_INNER_RECT.right(),
        SHADOW_TEXTURE_HEIGHT - SHADOW_INNER_RECT.bottom());
    QRectF bottomTile(bottomLeftTile.topRight(), bottomRightTile.bottomLeft());

    QRectF leftTile(topLeftTile.bottomLeft(), bottomLeftTile.topRight());
    QRectF rightTile(topRightTile.bottomLeft(), bottomRightTile.topRight());

    qreal tx1 = 0;
    qreal ty1 = 0;
    qreal tx2 = 0;
    qreal ty2 = 0;

    // No overlaps: In this case corner tiles are rendered as they are,
    // and top/right/bottom/left tiles are stretched.
    {
        QSize windowSize(1024, 1024);
        WindowQuadList shadowQuads;

        QRectF outerRect(
            -SHADOW_PADDING_LEFT,
            -SHADOW_PADDING_TOP,
            windowSize.width() + SHADOW_PADDING_LEFT + SHADOW_PADDING_RIGHT,
            windowSize.height() + SHADOW_PADDING_TOP + SHADOW_PADDING_BOTTOM);

        QRectF topLeft(
            outerRect.left(),
            outerRect.top(),
            topLeftTile.width(),
            topLeftTile.height());
        tx1 = topLeftTile.left()   / SHADOW_TEXTURE_WIDTH;
        ty1 = topLeftTile.top()    / SHADOW_TEXTURE_HEIGHT;
        tx2 = topLeftTile.right()  / SHADOW_TEXTURE_WIDTH;
        ty2 = topLeftTile.bottom() / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(topLeft, tx1, ty1, tx2, ty2);

        QRectF topRight(
            outerRect.right() - topRightTile.width(),
            outerRect.top(),
            topRightTile.width(),
            topRightTile.height());
        tx1 = topRightTile.left()   / SHADOW_TEXTURE_WIDTH;
        ty1 = topRightTile.top()    / SHADOW_TEXTURE_HEIGHT;
        tx2 = topRightTile.right()  / SHADOW_TEXTURE_WIDTH;
        ty2 = topRightTile.bottom() / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(topRight, tx1, ty1, tx2, ty2);

        QRectF top(topLeft.topRight(), topRight.bottomLeft());
        tx1 = topTile.left()   / SHADOW_TEXTURE_WIDTH;
        ty1 = topTile.top()    / SHADOW_TEXTURE_HEIGHT;
        tx2 = topTile.right()  / SHADOW_TEXTURE_WIDTH;
        ty2 = topTile.bottom() / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(top, tx1, ty1, tx2, ty2);

        QRectF bottomLeft(
            outerRect.left(),
            outerRect.bottom() - bottomLeftTile.height(),
            bottomLeftTile.width(),
            bottomLeftTile.height());
        tx1 = bottomLeftTile.left()   / SHADOW_TEXTURE_WIDTH;
        ty1 = bottomLeftTile.top()    / SHADOW_TEXTURE_HEIGHT;
        tx2 = bottomLeftTile.right()  / SHADOW_TEXTURE_WIDTH;
        ty2 = bottomLeftTile.bottom() / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(bottomLeft, tx1, ty1, tx2, ty2);

        QRectF bottomRight(
            outerRect.right() - bottomRightTile.width(),
            outerRect.bottom() - bottomRightTile.height(),
            bottomRightTile.width(),
            bottomRightTile.height());
        tx1 = bottomRightTile.left()   / SHADOW_TEXTURE_WIDTH;
        ty1 = bottomRightTile.top()    / SHADOW_TEXTURE_HEIGHT;
        tx2 = bottomRightTile.right()  / SHADOW_TEXTURE_WIDTH;
        ty2 = bottomRightTile.bottom() / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(bottomRight, tx1, ty1, tx2, ty2);

        QRectF bottom(bottomLeft.topRight(), bottomRight.bottomLeft());
        tx1 = bottomTile.left()   / SHADOW_TEXTURE_WIDTH;
        ty1 = bottomTile.top()    / SHADOW_TEXTURE_HEIGHT;
        tx2 = bottomTile.right()  / SHADOW_TEXTURE_WIDTH;
        ty2 = bottomTile.bottom() / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(bottom, tx1, ty1, tx2, ty2);

        QRectF left(topLeft.bottomLeft(), bottomLeft.topRight());
        tx1 = leftTile.left()   / SHADOW_TEXTURE_WIDTH;
        ty1 = leftTile.top()    / SHADOW_TEXTURE_HEIGHT;
        tx2 = leftTile.right()  / SHADOW_TEXTURE_WIDTH;
        ty2 = leftTile.bottom() / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(left, tx1, ty1, tx2, ty2);

        QRectF right(topRight.bottomLeft(), bottomRight.topRight());
        tx1 = rightTile.left()   / SHADOW_TEXTURE_WIDTH;
        ty1 = rightTile.top()    / SHADOW_TEXTURE_HEIGHT;
        tx2 = rightTile.right()  / SHADOW_TEXTURE_WIDTH;
        ty2 = rightTile.bottom() / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(right, tx1, ty1, tx2, ty2);

        QTest::newRow("no overlaps")
            << windowSize
            << WindowQuadListWrapper(shadowQuads);
    }

    // Top-Left & Bottom-Left/Top-Right & Bottom-Right overlap:
    // In this case overlapping parts are clipped and left/right
    // tiles aren't rendered.
    {
        QSize windowSize(1024, 128);
        WindowQuadList shadowQuads;
        qreal halfOverlap = 0.0;

        QRectF outerRect(
            -SHADOW_PADDING_LEFT,
            -SHADOW_PADDING_TOP,
            windowSize.width() + SHADOW_PADDING_LEFT + SHADOW_PADDING_RIGHT,
            windowSize.height() + SHADOW_PADDING_TOP + SHADOW_PADDING_BOTTOM);

        QRectF topLeft(
            outerRect.left(),
            outerRect.top(),
            topLeftTile.width(),
            topLeftTile.height());

        QRectF bottomLeft(
            outerRect.left(),
            outerRect.bottom() - bottomLeftTile.height(),
            bottomLeftTile.width(),
            bottomLeftTile.height());

        halfOverlap = qAbs(topLeft.bottom() - bottomLeft.top()) / 2;
        topLeft.setBottom(topLeft.bottom() - halfOverlap);
        bottomLeft.setTop(bottomLeft.top() + halfOverlap);

        tx1 = topLeftTile.left()   / SHADOW_TEXTURE_WIDTH;
        ty1 = topLeftTile.top()    / SHADOW_TEXTURE_HEIGHT;
        tx2 = topLeftTile.right()  / SHADOW_TEXTURE_WIDTH;
        ty2 = topLeft.bottom()     / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(topLeft, tx1, ty1, tx2, ty2);

        tx1 = bottomLeftTile.left()   / SHADOW_TEXTURE_WIDTH;
        ty1 = bottomLeft.top()        / SHADOW_TEXTURE_HEIGHT;
        tx2 = bottomLeftTile.right()  / SHADOW_TEXTURE_WIDTH;
        ty2 = bottomLeftTile.bottom() / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(bottomLeft, tx1, ty1, tx2, ty2);

        QRectF topRight(
            outerRect.right() - topRightTile.width(),
            outerRect.top(),
            topRightTile.width(),
            topRightTile.height());

        QRectF bottomRight(
            outerRect.right() - bottomRightTile.width(),
            outerRect.bottom() - bottomRightTile.height(),
            bottomRightTile.width(),
            bottomRightTile.height());

        halfOverlap = qAbs(topRight.bottom() - bottomRight.top()) / 2;
        topRight.setBottom(topRight.bottom() - halfOverlap);
        bottomRight.setTop(bottomRight.top() + halfOverlap);

        tx1 = topRightTile.left()   / SHADOW_TEXTURE_WIDTH;
        ty1 = topRightTile.top()    / SHADOW_TEXTURE_HEIGHT;
        tx2 = topRightTile.right()  / SHADOW_TEXTURE_WIDTH;
        ty2 = topRight.bottom()     / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(topRight, tx1, ty1, tx2, ty2);

        tx1 = bottomRightTile.left()   / SHADOW_TEXTURE_WIDTH;
        ty1 = bottomRight.top()        / SHADOW_TEXTURE_HEIGHT;
        tx2 = bottomRightTile.right()  / SHADOW_TEXTURE_WIDTH;
        ty2 = bottomRightTile.bottom() / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(bottomRight, tx1, ty1, tx2, ty2);

        QRectF top(topLeft.topRight(), topRight.bottomLeft());
        tx1 = topTile.left()  / SHADOW_TEXTURE_WIDTH;
        ty1 = topTile.top()   / SHADOW_TEXTURE_HEIGHT;
        tx2 = topTile.right() / SHADOW_TEXTURE_WIDTH;
        ty2 = top.height()    / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(top, tx1, ty1, tx2, ty2);

        QRectF bottom(bottomLeft.topRight(), bottomRight.bottomLeft());
        tx1 = bottomTile.left()          / SHADOW_TEXTURE_WIDTH;
        ty1 = 1.0 - (bottomTile.height() / SHADOW_TEXTURE_HEIGHT);
        tx2 = bottomTile.right()         / SHADOW_TEXTURE_WIDTH;
        ty2 = bottomTile.bottom()        / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(bottom, tx1, ty1, tx2, ty2);

        QTest::newRow("top-left & bottom-left/top-right & bottom-right overlap")
            << windowSize
            << WindowQuadListWrapper(shadowQuads);
    }

    // Top-Left & Top-Right/Bottom-Left & Bottom-Right overlap:
    // In this case overlapping parts are clipped and top/bottom
    // tiles aren't rendered.
    {
        QSize windowSize(128, 1024);
        WindowQuadList shadowQuads;
        qreal halfOverlap = 0.0;

        QRectF outerRect(
            -SHADOW_PADDING_LEFT,
            -SHADOW_PADDING_TOP,
            windowSize.width() + SHADOW_PADDING_LEFT + SHADOW_PADDING_RIGHT,
            windowSize.height() + SHADOW_PADDING_TOP + SHADOW_PADDING_BOTTOM);

        QRectF topLeft(
            outerRect.left(),
            outerRect.top(),
            topLeftTile.width(),
            topLeftTile.height());

        QRectF topRight(
            outerRect.right() - topRightTile.width(),
            outerRect.top(),
            topRightTile.width(),
            topRightTile.height());

        halfOverlap = qAbs(topLeft.right() - topRight.left()) / 2;
        topLeft.setRight(topLeft.right() - halfOverlap);
        topRight.setLeft(topRight.left() + halfOverlap);

        tx1 = topLeftTile.left()   / SHADOW_TEXTURE_WIDTH;
        ty1 = topLeftTile.top()    / SHADOW_TEXTURE_HEIGHT;
        tx2 = topLeft.right()      / SHADOW_TEXTURE_WIDTH;
        ty2 = topLeftTile.bottom() / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(topLeft, tx1, ty1, tx2, ty2);

        tx1 = topRight.left()       / SHADOW_TEXTURE_WIDTH;
        ty1 = topRightTile.top()    / SHADOW_TEXTURE_HEIGHT;
        tx2 = topRightTile.right()  / SHADOW_TEXTURE_WIDTH;
        ty2 = topRightTile.bottom() / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(topRight, tx1, ty1, tx2, ty2);

        QRectF bottomLeft(
            outerRect.left(),
            outerRect.bottom() - bottomLeftTile.height(),
            bottomLeftTile.width(),
            bottomLeftTile.height());

        QRectF bottomRight(
            outerRect.right() - bottomRightTile.width(),
            outerRect.bottom() - bottomRightTile.height(),
            bottomRightTile.width(),
            bottomRightTile.height());

        halfOverlap = qAbs(bottomLeft.right() - bottomRight.left()) / 2;
        bottomLeft.setRight(bottomLeft.right() - halfOverlap);
        bottomRight.setLeft(bottomRight.left() + halfOverlap);

        tx1 = bottomLeftTile.left()   / SHADOW_TEXTURE_WIDTH;
        ty1 = bottomLeftTile.top()    / SHADOW_TEXTURE_HEIGHT;
        tx2 = bottomLeft.right()      / SHADOW_TEXTURE_WIDTH;
        ty2 = bottomLeftTile.bottom() / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(bottomLeft, tx1, ty1, tx2, ty2);

        tx1 = bottomRight.left()       / SHADOW_TEXTURE_WIDTH;
        ty1 = bottomRightTile.top()    / SHADOW_TEXTURE_HEIGHT;
        tx2 = bottomRightTile.right()  / SHADOW_TEXTURE_WIDTH;
        ty2 = bottomRightTile.bottom() / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(bottomRight, tx1, ty1, tx2, ty2);

        QRectF left(topLeft.bottomLeft(), bottomLeft.topRight());
        tx1 = leftTile.left()   / SHADOW_TEXTURE_WIDTH;
        ty1 = leftTile.top()    / SHADOW_TEXTURE_HEIGHT;
        tx2 = left.width()      / SHADOW_TEXTURE_WIDTH;
        ty2 = leftTile.bottom() / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(left, tx1, ty1, tx2, ty2);

        QRectF right(topRight.bottomLeft(), bottomRight.topRight());
        tx1 = 1.0 - (right.width() / SHADOW_TEXTURE_WIDTH);
        ty1 = rightTile.top()      / SHADOW_TEXTURE_HEIGHT;
        tx2 = rightTile.right()    / SHADOW_TEXTURE_WIDTH;
        ty2 = rightTile.bottom()   / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(right, tx1, ty1, tx2, ty2);

        QTest::newRow("top-left & top-right/bottom-left & bottom-right overlap")
            << windowSize
            << WindowQuadListWrapper(shadowQuads);
    }

    // All shadow tiles overlap: In this case all overlapping parts
    // are clippend and top/right/bottom/left tiles aren't rendered.
    {
        QSize windowSize(128, 128);
        WindowQuadList shadowQuads;
        qreal halfOverlap = 0.0;

        QRectF outerRect(
            -SHADOW_PADDING_LEFT,
            -SHADOW_PADDING_TOP,
            windowSize.width() + SHADOW_PADDING_LEFT + SHADOW_PADDING_RIGHT,
            windowSize.height() + SHADOW_PADDING_TOP + SHADOW_PADDING_BOTTOM);

        QRectF topLeft(
            outerRect.left(),
            outerRect.top(),
            topLeftTile.width(),
            topLeftTile.height());

        QRectF topRight(
            outerRect.right() - topRightTile.width(),
            outerRect.top(),
            topRightTile.width(),
            topRightTile.height());

        QRectF bottomLeft(
            outerRect.left(),
            outerRect.bottom() - bottomLeftTile.height(),
            bottomLeftTile.width(),
            bottomLeftTile.height());

        QRectF bottomRight(
            outerRect.right() - bottomRightTile.width(),
            outerRect.bottom() - bottomRightTile.height(),
            bottomRightTile.width(),
            bottomRightTile.height());

        halfOverlap = qAbs(topLeft.right() - topRight.left()) / 2;
        topLeft.setRight(topLeft.right() - halfOverlap);
        topRight.setLeft(topRight.left() + halfOverlap);

        halfOverlap = qAbs(bottomLeft.right() - bottomRight.left()) / 2;
        bottomLeft.setRight(bottomLeft.right() - halfOverlap);
        bottomRight.setLeft(bottomRight.left() + halfOverlap);

        halfOverlap = qAbs(topLeft.bottom() - bottomLeft.top()) / 2;
        topLeft.setBottom(topLeft.bottom() - halfOverlap);
        bottomLeft.setTop(bottomLeft.top() + halfOverlap);

        halfOverlap = qAbs(topRight.bottom() - bottomRight.top()) / 2;
        topRight.setBottom(topRight.bottom() - halfOverlap);
        bottomRight.setTop(bottomRight.top() + halfOverlap);

        tx1 = topLeftTile.left() / SHADOW_TEXTURE_WIDTH;
        ty1 = topLeftTile.top()  / SHADOW_TEXTURE_HEIGHT;
        tx2 = topLeft.width()    / SHADOW_TEXTURE_WIDTH;
        ty2 = topLeft.height()   / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(topLeft, tx1, ty1, tx2, ty2);

        tx1 = 1.0 - (topRight.width() / SHADOW_TEXTURE_WIDTH);
        ty1 = topRightTile.top()      / SHADOW_TEXTURE_HEIGHT;
        tx2 = topRightTile.right()    / SHADOW_TEXTURE_WIDTH;
        ty2 = topRight.bottom()       / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(topRight, tx1, ty1, tx2, ty2);

        tx1 = bottomLeftTile.left()      / SHADOW_TEXTURE_WIDTH;
        ty1 = 1.0 - (bottomLeft.height() / SHADOW_TEXTURE_HEIGHT);
        tx2 = bottomLeft.width()         / SHADOW_TEXTURE_WIDTH;
        ty2 = bottomLeftTile.bottom()    / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(bottomLeft, tx1, ty1, tx2, ty2);

        tx1 = 1.0 - (bottomRight.width()  / SHADOW_TEXTURE_WIDTH);
        ty1 = 1.0 - (bottomRight.height() / SHADOW_TEXTURE_HEIGHT);
        tx2 = bottomRightTile.right()     / SHADOW_TEXTURE_WIDTH;
        ty2 = bottomRightTile.bottom()    / SHADOW_TEXTURE_HEIGHT;
        shadowQuads << makeShadowQuad(bottomRight, tx1, ty1, tx2, ty2);

        QTest::newRow("all corner tiles overlap")
            << windowSize
            << WindowQuadListWrapper(shadowQuads);
    }

    // Window is too small: do not render any shadow tiles.
    {
        QSize windowSize(1, 1);
        WindowQuadList shadowQuads;

        QTest::newRow("window too small")
            << windowSize
            << WindowQuadListWrapper(shadowQuads);
    }
}

void SceneOpenGLShadowTest::testShadowTileOverlaps()
{
    QFETCH(QSize, windowSize);
    QFETCH(WindowQuadListWrapper, expectedQuadsWrapper);
    WindowQuadList expectedQuads = expectedQuadsWrapper.quadList();

    // Create a decorated client.
    QScopedPointer<Surface> surface(Test::createSurface());
    QScopedPointer<ShellSurface> shellSurface(Test::createShellSurface(surface.data()));
    QScopedPointer<ServerSideDecoration> ssd(Test::waylandServerSideDecoration()->create(surface.data()));

    auto client = Test::renderAndWaitForShown(surface.data(), windowSize, Qt::blue);

    QSignalSpy sizeChangedSpy(shellSurface.data(), &ShellSurface::sizeChanged);
    QVERIFY(sizeChangedSpy.isValid());

    // Check the client is decorated.
    QVERIFY(client);
    QVERIFY(client->isDecorated());
    auto decoration = client->decoration();
    QVERIFY(decoration);

    // If speciefied decoration theme is not found, KWin loads a default one
    // so we have to check whether a client has right decoration.
    auto decoShadow = decoration->shadow();
    QCOMPARE(decoShadow->shadow().size(), QSize(SHADOW_TEXTURE_WIDTH, SHADOW_TEXTURE_HEIGHT));
    QCOMPARE(decoShadow->paddingTop(),    SHADOW_PADDING_TOP);
    QCOMPARE(decoShadow->paddingRight(),  SHADOW_PADDING_RIGHT);
    QCOMPARE(decoShadow->paddingBottom(), SHADOW_PADDING_BOTTOM);
    QCOMPARE(decoShadow->paddingLeft(),   SHADOW_PADDING_LEFT);

    // Get shadow.
    QVERIFY(client->effectWindow());
    QVERIFY(client->effectWindow()->sceneWindow());
    QVERIFY(client->effectWindow()->sceneWindow()->shadow());
    auto shadow = client->effectWindow()->sceneWindow()->shadow();

    // Validate shadow quads.
    const WindowQuadList &quads = shadow->shadowQuads();
    QCOMPARE(quads.size(), expectedQuads.size());

    QVector<bool> mask(expectedQuads.size(), false);
    for (const auto &q : quads) {
        for (int i = 0; i < expectedQuads.size(); i++) {
            if (! compareQuads(q, expectedQuads[i])) {
                continue;
            }
            if (! mask[i]) {
                mask[i] = true;
                break;
            } else {
                QFAIL("got a duplicate shadow quad");
            }
        }
    }

    for (const auto v : mask) {
        if (! v) {
            QFAIL("missed a shadow quad");
        }
    }
}

WAYLANDTEST_MAIN(SceneOpenGLShadowTest)
#include "scene_opengl_shadow_test.moc"
