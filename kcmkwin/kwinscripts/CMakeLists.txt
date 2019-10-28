include(ECMQMLModules)
ecm_find_qmlmodule(org.kde.plasma.core 2.0)

# KI18N Translation Domain for this library.
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_kwin_scripts\")

########### next target ###############

set(kcm_kwin_scripts_PART_SRCS
    kcm.cpp
    scriptsfilterproxymodel.cpp
)

add_library(kcm_kwin_scripts MODULE ${kcm_kwin_scripts_PART_SRCS})

target_link_libraries(kcm_kwin_scripts
    Qt5::DBus

    KF5::I18n
    KF5::KCMUtils
    KF5::NewStuff
    KF5::QuickAddons
    KF5::XmlGui

    kcmkwincommon
)

kcoreaddons_desktop_to_json(kcm_kwin_scripts "kcm_kwin_scripts.desktop")

########### install files ###############

install(TARGETS kcm_kwin_scripts DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)
install(FILES kcm_kwin_scripts.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES kwinscripts.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
kpackage_install_package(package kcm_kwin_scripts kcms)
