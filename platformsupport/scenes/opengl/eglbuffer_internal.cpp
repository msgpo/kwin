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

#include "eglbuffer_internal_p.h"
#include "abstract_egl_backend.h"

namespace KWin
{

EGLBufferInternalPrivate::EGLBufferInternalPrivate(AbstractEglBackend *backend)
    : OpenGLBufferInternalPrivate(backend)
{
}

bool EGLBufferInternalPrivate::create()
{
    return false;
}

void EGLBufferInternalPrivate::update()
{
}

} // namespace KWin
