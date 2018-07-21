#######################################
# Config
set(kwin_fadedesktop_config_SRCS fadedesktop_config.cpp)
ki18n_wrap_ui(kwin_fadedesktop_config_SRCS fadedesktop_config.ui)
qt5_add_dbus_interface(kwin_fadedesktop_config_SRCS ${kwin_effects_dbus_xml} kwineffects_interface)
kconfig_add_kcfg_files(kwin_fadedesktop_config_SRCS fadedesktopconfig.kcfgc)

add_library(kwin_fadedesktop_config MODULE ${kwin_fadedesktop_config_SRCS})

target_link_libraries(kwin_fadedesktop_config
    Qt5::DBus
    KF5::ConfigWidgets
    KF5::I18n
    KF5::Service
)

kcoreaddons_desktop_to_json(kwin_fadedesktop_config fadedesktop_config.desktop SERVICE_TYPES kcmodule.desktop)

install(
    TARGETS
        kwin_fadedesktop_config
    DESTINATION
        ${PLUGIN_INSTALL_DIR}/kwin/effects/configs
)
