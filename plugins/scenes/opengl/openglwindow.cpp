/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006 Lubos Lunak <l.lunak@kde.org>
Copyright (C) 2009, 2010, 2011 Martin Gräßlin <mgraesslin@kde.org>
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

#include "openglwindow.h"
#include "opengldecorationscenenode.h"
#include "openglshadowscenenode.h"
#include "openglsurfacescenenode.h"
#include "openglwindowpixmap.h"
#include "scene_opengl.h"

#include "kwinglutils.h"

#include "options.h"
#include "wayland_server.h"

#include <QGraphicsScale>

namespace KWin
{

OpenGLWindow::OpenGLWindow(Toplevel *t, SceneOpenGL2 *scene)
    : Scene::Window(t)
    , m_scene(scene)
{
}

OpenGLWindow::~OpenGLWindow()
{
}

void OpenGLWindow::performPaint(int mask, QRegion region, WindowPaintData data)
{
    if (!beginRenderWindow(mask, region, data)) {
        return;
    }

    QMatrix4x4 windowMatrix = transformation(mask, data);
    const QMatrix4x4 modelViewProjection = modelViewProjectionMatrix(mask, data);
    const QMatrix4x4 mvpMatrix = modelViewProjection * windowMatrix;

    GLShader *shader = data.shader;
    if (!shader) {
        ShaderTraits traits = ShaderTrait::MapTexture;
        if (data.opacity() != 1.0 || data.brightness() != 1.0) {
            traits |= ShaderTrait::Modulate;
        }
        if (data.saturation() != 1.0) {
            traits |= ShaderTrait::AdjustSaturation;
        }
        shader = ShaderManager::instance()->pushShader(traits);
    }

    shader->setUniform(GLShader::ModelViewProjectionMatrix, mvpMatrix);
    shader->setUniform(GLShader::Saturation, data.saturation());

    // FIXME: Split the window quads.

    // FIXME: Upload the window quads.

    if (!data.shader) {
        ShaderManager::instance()->popShader();
    }

    endRenderWindow();
}

WindowPixmap *OpenGLWindow::createWindowPixmap()
{
    return new OpenGLWindowPixmap(this, m_scene);
}

ShadowSceneNode *OpenGLWindow::createShadowNode()
{
    return new OpenGLShadowSceneNode(toplevel);
}

DecorationSceneNode *OpenGLWindow::createDecorationNode()
{
    return new OpenGLDecorationSceneNode(toplevel);
}

SurfaceSceneNode *OpenGLWindow::createSurfaceNode()
{
    return new OpenGLSurfaceSceneNode(toplevel);
}

QMatrix4x4 OpenGLWindow::modelViewProjectionMatrix(int mask, const WindowPaintData &data) const
{
    const QMatrix4x4 pMatrix = data.projectionMatrix();
    const QMatrix4x4 mvMatrix = data.modelViewMatrix();

    // An effect may want to override the default projection matrix in some cases,
    // such as when it is rendering a window on a render target that doesn't have
    // the same dimensions as the default framebuffer.
    //
    // Note that the screen transformation is not applied here.
    if (!pMatrix.isIdentity()) {
        return pMatrix * mvMatrix;
    }

    // If an effect has specified a model-view matrix, we multiply that matrix
    // with the default projection matrix.  If the effect hasn't specified a
    // model-view matrix, mvMatrix will be the identity matrix.
    if (mask & Scene::PAINT_SCREEN_TRANSFORMED) {
        return m_scene->screenProjectionMatrix() * mvMatrix;
    }

    return m_scene->projectionMatrix() * mvMatrix;
}

QMatrix4x4 OpenGLWindow::transformation(int mask, const WindowPaintData &data) const
{
    QMatrix4x4 matrix;
    matrix.translate(x(), y());

    if (!(mask & Scene::PAINT_WINDOW_TRANSFORMED)) {
        return matrix;
    }

    matrix.translate(data.translation());
    data.scale().applyTo(&matrix);

    if (data.rotationAngle() == 0.0) {
        return matrix;
    }

    // Apply the rotation
    // cannot use data.rotation.applyTo(&matrix) as QGraphicsRotation uses projectedRotate to map back to 2D
    matrix.translate(data.rotationOrigin());
    const QVector3D axis = data.rotationAxis();
    matrix.rotate(data.rotationAngle(), axis.x(), axis.y(), axis.z());
    matrix.translate(-data.rotationOrigin());

    return matrix;
}

QVector4D OpenGLWindow::modulate(float opacity, float brightness) const
{
    const float a = opacity;
    const float rgb = opacity * brightness;

    return QVector4D(rgb, rgb, rgb, a);
}

bool OpenGLWindow::beginRenderWindow(int mask, const QRegion &region, WindowPaintData &data)
{
    if (region.isEmpty())
        return false;

    m_hardwareClipping = region != infiniteRegion() && (mask & Scene::PAINT_WINDOW_TRANSFORMED) && !(mask & Scene::PAINT_SCREEN_TRANSFORMED);
    if (region != infiniteRegion() && !m_hardwareClipping) {
        data.quads = data.quads.intersected(region.translated(-x(), -y()));
    }

    if (data.quads.isEmpty())
        return false;

//    if (!bindTexture() || !s_frameTexture) {
//        return false;
//    }

    if (m_hardwareClipping) {
        glEnable(GL_SCISSOR_TEST);
    }

    // Update the texture filter
    if (waylandServer()) {
        filter = Scene::ImageFilterGood;
//        s_frameTexture->setFilter(GL_LINEAR);
    } else {
        if (options->glSmoothScale() != 0 &&
            (mask & (Scene::PAINT_WINDOW_TRANSFORMED | Scene::PAINT_SCREEN_TRANSFORMED)))
            filter = Scene::ImageFilterGood;
        else
            filter = Scene::ImageFilterFast;

//        s_frameTexture->setFilter(filter == ImageFilterGood ? GL_LINEAR : GL_NEAREST);
    }

    const GLVertexAttrib attribs[] = {
        { VA_Position, 2, GL_FLOAT, offsetof(GLVertex2D, position) },
        { VA_TexCoord, 2, GL_FLOAT, offsetof(GLVertex2D, texcoord) },
    };

    GLVertexBuffer *vbo = GLVertexBuffer::streamingBuffer();
    vbo->reset();
    vbo->setAttribLayout(attribs, 2, sizeof(GLVertex2D));

    return true;
}

void OpenGLWindow::endRenderWindow()
{
    if (m_hardwareClipping) {
        glDisable(GL_SCISSOR_TEST);
    }
}

void OpenGLWindow::setBlendEnabled(bool enabled)
{
    if (enabled && !m_blendingEnabled) {
        glEnable(GL_BLEND);
    } else if (!enabled && m_blendingEnabled) {
        glDisable(GL_BLEND);
    }
    m_blendingEnabled = enabled;
}

} // namespace KWin
