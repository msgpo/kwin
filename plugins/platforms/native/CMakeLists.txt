set(NativePlatform_SOURCES
    native_platform.cpp
    drm_allocator.cpp
    drm_buffer.cpp
    drm_connector.cpp
    drm_context.cpp
    drm_crtc.cpp
    drm_device_manager.cpp
    drm_device.cpp
    drm_dumb_allocator.cpp
    drm_dumb_image_view.cpp
    drm_dumb_image.cpp
    drm_edid.cpp
    drm_gbm_allocator.cpp
    drm_gbm_image_view.cpp
    drm_gbm_image.cpp
    drm_image_view.cpp
    drm_image.cpp
    drm_object.cpp
    drm_output.cpp
    drm_pixel_format.cpp
    drm_plane.cpp
    drm_swapchain.cpp
    egl_cursor_renderer.cpp
    egl_scene_renderer.cpp
    qpainter_cursor_renderer.cpp
    qpainter_scene_renderer.cpp
)

add_library(KWinWaylandNativeBackend MODULE ${NativePlatform_SOURCES})

target_link_libraries(KWinWaylandNativeBackend
    kwin
    gbm::gbm
    Libdrm::Libdrm
)

install(TARGETS KWinWaylandNativeBackend DESTINATION ${PLATFORM_INSTALL_DIR})
