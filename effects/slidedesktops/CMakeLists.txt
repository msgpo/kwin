#######################################
# Config
set(kwin_slidedesktops_config_SRCS slidedesktops_config.cpp)
ki18n_wrap_ui(kwin_slidedesktops_config_SRCS slidedesktops_config.ui)
qt5_add_dbus_interface(kwin_slidedesktops_config_SRCS ${kwin_effects_dbus_xml} kwineffects_interface)
kconfig_add_kcfg_files(kwin_slidedesktops_config_SRCS slidedesktopsconfig.kcfgc)

add_library(kwin_slidedesktops_config MODULE ${kwin_slidedesktops_config_SRCS})

target_link_libraries(kwin_slidedesktops_config
    Qt5::DBus
    KF5::ConfigWidgets
    KF5::I18n
    KF5::Service
)

kcoreaddons_desktop_to_json(kwin_slidedesktops_config slidedesktops_config.desktop SERVICE_TYPES kcmodule.desktop)

install(
    TARGETS
        kwin_slidedesktops_config
    DESTINATION
        ${PLUGIN_INSTALL_DIR}/kwin/effects/configs
)
