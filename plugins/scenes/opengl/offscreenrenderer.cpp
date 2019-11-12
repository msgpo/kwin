/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2019 Vlad Zahorodnii <vladzzag@gmail.com>

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

#include "offscreenrenderer.h"
#include "effects.h"
#include "kwineffects.h"
#include "kwinglplatform.h"
#include "kwingltexture.h"
#include "kwinglutils.h"
#include "scene_opengl.h"
#include "workspace.h"

namespace KWin
{

static SceneOpenGL2Window *sceneWindowFromEffectWindow(EffectWindowImpl *window)
{
    return static_cast<SceneOpenGL2Window *>(window->sceneWindow());
}

static EffectWindowImpl *effectWindowFromSceneWindow(SceneOpenGL2Window *window)
{
    return static_cast<EffectWindowImpl *>(window->window()->effectWindow());
}

static WindowQuadList transformWindowQuads(const Toplevel *toplevel, const WindowQuadList &windowQuads)
{
    const QRect frameGeometry = toplevel->frameGeometry();
    const QRect expandedGeometry = toplevel->visibleRect();

    const qreal correctionU = frameGeometry.x() - expandedGeometry.x();
    const qreal correctionV = frameGeometry.y() - expandedGeometry.y();

    WindowQuadList transformedQuads;
    transformedQuads.reserve(windowQuads.count());

    for (const WindowQuad &originalQuad : windowQuads) {
        WindowQuad transformedQuad(originalQuad.type());
        for (int i = 0; i < 4; ++i) {
            const qreal x = originalQuad[i].x();
            const qreal y = originalQuad[i].y();
            const qreal u = originalQuad[i].originalX() + correctionU;
            const qreal v = originalQuad[i].originalY() + correctionV;
            transformedQuad[i] = WindowVertex(x, y, u, v);
        }
        transformedQuads.append(transformedQuad);
    }

    return transformedQuads;
}

static int initializeVertexBuffer(GLVertexBuffer *buffer, const WindowQuadList &windowQuads,
                                  const QMatrix4x4 &matrix, int primitiveType)
{
    const int verticesPerQuad = primitiveType == GL_QUADS ? 4 : 6;
    const int vertexCount = windowQuads.count() * verticesPerQuad;
    const int vertexBufferSize = vertexCount * sizeof(GLVertex2D);

    GLVertex2D *vertices = static_cast<GLVertex2D *>(buffer->map(vertexBufferSize));
    windowQuads.makeInterleavedArrays(primitiveType, vertices, matrix);
    buffer->unmap();

    return vertexCount;
}

static void discardOffscreenTexture(EffectWindowImpl *effectWindow)
{
    SceneOpenGL2Window *sceneWindow = sceneWindowFromEffectWindow(effectWindow);
    delete sceneWindow->offscreenTarget();
    sceneWindow->setOffscreenTarget(nullptr);
    delete sceneWindow->offscreenTexture();
    sceneWindow->setOffscreenTexture(nullptr);
}

OffscreenRenderer *OffscreenRenderer::create(SceneOpenGL *scene)
{
    const GLPlatform *platform = GLPlatform::instance();

    if (platform->isSoftwareEmulation()) {
        return nullptr;
    }

    if (!GLRenderTarget::supported()) {
        return nullptr;
    }

    return new OffscreenRenderer(scene);
}

OffscreenRenderer::OffscreenRenderer(SceneOpenGL *scene)
    : QObject(scene)
    , m_scene(scene)
{
    m_discardTimer = new QTimer(this);
    m_discardTimer->setSingleShot(true);
    m_discardTimer->setInterval(5000);
    connect(m_discardTimer, &QTimer::timeout, this, &OffscreenRenderer::discardOffscreenTextures);
}

OffscreenRenderer::~OffscreenRenderer()
{
}

void OffscreenRenderer::performPaint(EffectWindowImpl *effectWindow, int mask, const QRegion &region, WindowPaintData &data)
{
    SceneOpenGL2Window *sceneWindow = sceneWindowFromEffectWindow(effectWindow);
    GLTexture *offscreenTexture = sceneWindow->offscreenTexture();
    Toplevel *toplevel = sceneWindow->window();
    ShaderManager *shaderManager = ShaderManager::instance();

    const QRect expandedGeometry = toplevel->visibleRect();
    const int primitiveType = GLVertexBuffer::supportsIndexedQuads() ? GL_QUADS : GL_TRIANGLES;
    const bool performClipping = region != infiniteRegion();
    const bool hardwareClipping = mask & Scene::PAINT_WINDOW_TRANSFORMED &&
        !(mask & Scene::PAINT_SCREEN_TRANSFORMED);

    if (offscreenTexture) {
        if (expandedGeometry.size() != offscreenTexture->size()) {
            discardOffscreenTexture(effectWindow);
        }
    }

    if (!offscreenTexture || !toplevel->damage().isEmpty()) {
        offscreenTexture = render(sceneWindow, mask);
    }

    WindowQuadList windowQuads = data.quads;
    if (performClipping && !hardwareClipping) {
        windowQuads = windowQuads.intersected(region.translated(-toplevel->pos()));
    }

    windowQuads = transformWindowQuads(toplevel, windowQuads);

    const GLVertexAttrib vertexAttributes[] = {
        { VA_Position, 2, GL_FLOAT, offsetof(GLVertex2D, position) },
        { VA_TexCoord, 2, GL_FLOAT, offsetof(GLVertex2D, texcoord) },
    };

    GLVertexBuffer *vertexBufferObject = GLVertexBuffer::streamingBuffer();
    vertexBufferObject->reset();
    vertexBufferObject->setAttribLayout(vertexAttributes, 2, sizeof(GLVertex2D));
    const QMatrix4x4 textureMatrix = offscreenTexture->matrix(UnnormalizedCoordinates);
    const int vertexCount = initializeVertexBuffer(vertexBufferObject, windowQuads, textureMatrix, primitiveType);

    if (performClipping && hardwareClipping) {
        glEnable(GL_SCISSOR_TEST);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    const QMatrix4x4 windowMatrix = sceneWindow->transformation(mask, data);
    const QMatrix4x4 modelViewProjection = sceneWindow->modelViewProjectionMatrix(mask, data);
    const QMatrix4x4 mvpMatrix = modelViewProjection * windowMatrix;

    GLShader *shader = data.shader;
    if (!shader) {
        const ShaderTraits traits = ShaderTrait::MapTexture |
                                    ShaderTrait::Modulate |
                                    ShaderTrait::AdjustSaturation;
        shader = shaderManager->pushShader(traits);
    }

    const qreal rgb = data.brightness() * data.opacity();
    const qreal alpha = data.opacity();

    shader->setUniform(GLShader::ModelViewProjectionMatrix, mvpMatrix);
    shader->setUniform(GLShader::ModulationConstant, QVector4D(rgb, rgb, rgb, alpha));
    shader->setUniform(GLShader::Saturation, data.saturation());

    vertexBufferObject->bindArrays();
    offscreenTexture->bind();
    vertexBufferObject->draw(region, primitiveType, 0, vertexCount, performClipping && hardwareClipping);
    offscreenTexture->unbind();
    vertexBufferObject->unbindArrays();

    if (!data.shader) {
        shaderManager->popShader();
    }

    glDisable(GL_BLEND);

    if (performClipping && hardwareClipping) {
        glDisable(GL_SCISSOR_TEST);
    }

    m_discardTimer->start();
}

void OffscreenRenderer::discardOffscreenTextures()
{
    m_scene->makeOpenGLContextCurrent();

    workspace()->forEachToplevel([](Toplevel *toplevel) {
        discardOffscreenTexture(toplevel->effectWindow());
    });
}

GLTexture *OffscreenRenderer::render(SceneOpenGL2Window *sceneWindow, int mask) const
{
    EffectWindowImpl *effectWindow = effectWindowFromSceneWindow(sceneWindow);

    const QRect expandedGeometry = effectWindow->expandedGeometry();

    GLTexture *offscreenTexture = sceneWindow->offscreenTexture();
    if (!offscreenTexture) {
        offscreenTexture = new GLTexture(GL_RGBA8, expandedGeometry.size());
        offscreenTexture->setFilter(GL_LINEAR);
        offscreenTexture->setWrapMode(GL_CLAMP_TO_EDGE);
        sceneWindow->setOffscreenTexture(offscreenTexture);
    }

    GLRenderTarget *offscreenTarget = sceneWindow->offscreenTarget();
    if (!offscreenTarget) {
        offscreenTarget = new GLRenderTarget(*offscreenTexture);
        sceneWindow->setOffscreenTarget(offscreenTarget);
    }

    QMatrix4x4 modelViewProjectionMatrix;
    modelViewProjectionMatrix.ortho(0, offscreenTexture->width(), offscreenTexture->height(), 0, 0, 65535);

    WindowPaintData windowPaintData(effectWindow);
    windowPaintData.setXTranslation(-expandedGeometry.x());
    windowPaintData.setYTranslation(-expandedGeometry.y());
    windowPaintData.setProjectionMatrix(modelViewProjectionMatrix);

    const int additionalMaskFlags = Scene::PAINT_WINDOW_TRANSFORMED |
        Scene::PAINT_WINDOW_TRANSLUCENT;

    GLRenderTarget::pushRenderTarget(offscreenTarget);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    sceneWindow->performPaint(mask | additionalMaskFlags, infiniteRegion(), windowPaintData);
    GLRenderTarget::popRenderTarget();

    return offscreenTexture;
}

} // namespace KWin
