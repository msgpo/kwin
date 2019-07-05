/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

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

#include "drm_swapchain.h"
#include "drm_image_view.h"
#include "drm_image.h"

namespace KWin
{

DrmFramebuffer::DrmFramebuffer(DrmImage *image, DrmImageView *view)
    : m_image(image)
    , m_view(view)
{
}

DrmFramebuffer::~DrmFramebuffer()
{
    delete m_view;
    delete m_image;
}

DrmImage *DrmFramebuffer::image() const
{
    return m_image;
}

DrmImageView *DrmFramebuffer::view() const
{
    return m_view;
}

uint DrmFramebuffer::sequence() const
{
    return m_sequence;
}

void DrmFramebuffer::setSequence(uint sequence)
{
    m_sequence = sequence;
}

bool DrmFramebuffer::isAcquired() const
{
    return m_isAcquired;
}

void DrmFramebuffer::acquire()
{
    m_isAcquired = true;
}

void DrmFramebuffer::release()
{
    m_isAcquired = false;
}

DrmSwapchain::DrmSwapchain(DrmDevice *device, QObject *parent)
    : QObject(parent)
{
}

DrmSwapchain::~DrmSwapchain()
{
}

bool DrmSwapchain::isValid() const
{
    return false;
}

int DrmSwapchain::imageCount() const
{
    return m_framebuffers.count();
}

DrmFramebuffer *DrmSwapchain::acquire()
{
    return nullptr;
}

} // namespace KWin
