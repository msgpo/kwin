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

#pragma once

namespace KWin
{

class DrmConnector;
class DrmCrtc;
class DrmDevice;
class DrmSwapchain;

class DrmOutput
{
public:
    ~DrmOutput();

    /**
     * Returns a DRM device this output connected to.
     */
    DrmDevice *device() const;

    /**
     * Returns connector this output attached to.
     */
    DrmConnector *connector() const;

    /**
     * Returns a CRTC that drives this output.
     */
    DrmCrtc *crtc() const;

    /**
     * Returns the swapchain of the output.
     */
    DrmSwapchain *swapchain() const;

private:
    void createSwapchain();
    void destroySwapchain();

    DrmConnector *m_connector = nullptr;
    DrmCrtc *m_crtc = nullptr;
    DrmDevice *m_device = nullptr;
    DrmSwapchain *m_swapchain = nullptr;

    friend class DrmDevice;
};

} // namespace KWin
