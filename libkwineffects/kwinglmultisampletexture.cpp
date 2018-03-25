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

#include "kwinglmultisampletexture.h"

#include <QSharedData>


namespace KWin
{

class Q_DECL_HIDDEN GLMultisampleTexture::Private : public QSharedData
{
public:
    Private();
    ~Private();

    GLuint texture = 0;
    GLenum target = GL_TEXTURE_2D_MULTISAMPLE;
    GLenum internalFormat = GL_RGBA8;
    int width = 0;
    int height = 0;
    int samples = 0;

private:
    Q_DISABLE_COPY(Private)
};

GLMultisampleTexture::Private::Private()
{
}

GLMultisampleTexture::Private::~Private()
{
    if (texture != 0) {
        glDeleteTextures(1, &texture);
    }
}

GLMultisampleTexture::GLMultisampleTexture(int width, int height, int samples,
                                           GLenum format, GLenum target)
    : d(new GLMultisampleTexture::Private)
{
    d->target = target;
    d->internalFormat = format;
    d->width = width;
    d->height = height;
    d->samples = samples;

    glGenTextures(1, &d->texture);

    bind();
    glTexImage2DMultisample(d->target, d->samples, d->internalFormat,
                            d->width, d->height, false);
    unbind();
}

GLMultisampleTexture::GLMultisampleTexture(const QSize &size, int samples, GLenum format, GLenum target)
    : GLMultisampleTexture(size.width(), size.height(), samples, format, target)
{
}

GLMultisampleTexture::~GLMultisampleTexture()
{
}

void GLMultisampleTexture::bind()
{
    glBindTexture(d->target, d->texture);
}

void GLMultisampleTexture::unbind()
{
    glBindTexture(d->target, 0);
}

GLuint GLMultisampleTexture::texture() const
{
    return d->texture;
}

GLenum GLMultisampleTexture::target() const
{
    return d->target;
}

GLenum GLMultisampleTexture::internalFormat() const
{
    return d->internalFormat;
}

int GLMultisampleTexture::samples() const
{
    return d->samples;
}

int GLMultisampleTexture::width() const
{
    return d->width;
}

int GLMultisampleTexture::height() const
{
    return d->height;
}

QSize GLMultisampleTexture::size() const
{
    return QSize(d->width, d->height);
}

void GLMultisampleTexture::resize(int width, int height)
{
    if (d->width == width && d->height == height) {
        return;
    }

    d->width = width;
    d->height = height;

    bind();
    glTexImage2DMultisample(d->target, d->samples, d->internalFormat,
                            d->width, d->height, false);
    unbind();
}

void GLMultisampleTexture::resize(const QSize &size)
{
    resize(size.width(), size.height());
}

} // namespace KWin
