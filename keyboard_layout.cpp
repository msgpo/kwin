/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2016, 2017 Martin Gräßlin <mgraesslin@kde.org>
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
#include "keyboard_layout.h"
#include "keyboard_layout_icon_provider.h"
#include "keyboard_layout_switching.h"
#include "keyboard_input.h"
#include "input_event.h"
#include "main.h"
#include "platform.h"
#include "utils.h"

#include <KConfigGroup>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <KNotifications/KStatusNotifierItem>
#include <QAction>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QMenu>

namespace KWin
{

// libxkbcommon has API only for getting the full name of the current layout,
// there is no API for getting the current layout in the form of the ISO
// two-character country code (the one that is passed in RMLVO).
// This translation table is derived from /usr/share/X11/xkb/rules/base.lst
static const QHash<QString, QString> s_layoutNameToLayout {
    {QStringLiteral("English (US)"),                              QStringLiteral("us")},
    {QStringLiteral("Afghani"),                                   QStringLiteral("af")},
    {QStringLiteral("Arabic"),                                    QStringLiteral("ara")},
    {QStringLiteral("Albanian"),                                  QStringLiteral("al")},
    {QStringLiteral("Armenian"),                                  QStringLiteral("am")},
    {QStringLiteral("German (Austria)"),                          QStringLiteral("at")},
    {QStringLiteral("English (Australian)"),                      QStringLiteral("au")},
    {QStringLiteral("Azerbaijani"),                               QStringLiteral("az")},
    {QStringLiteral("Belarusian"),                                QStringLiteral("by")},
    {QStringLiteral("Belgian"),                                   QStringLiteral("be")},
    {QStringLiteral("Bangla"),                                    QStringLiteral("bd")},
    {QStringLiteral("Indian"),                                    QStringLiteral("in")},
    {QStringLiteral("Bosnian"),                                   QStringLiteral("ba")},
    {QStringLiteral("Portuguese (Brazil)"),                       QStringLiteral("br")},
    {QStringLiteral("Bulgarian"),                                 QStringLiteral("bg")},
    {QStringLiteral("Berber (Algeria, Latin)"),                   QStringLiteral("dz")},
    {QStringLiteral("Arabic (Morocco)"),                          QStringLiteral("ma")},
    {QStringLiteral("English (Cameroon)"),                        QStringLiteral("cm")},
    {QStringLiteral("Burmese"),                                   QStringLiteral("mm")},
    {QStringLiteral("French (Canada)"),                           QStringLiteral("ca")},
    {QStringLiteral("French (Democratic Republic of the Congo)"), QStringLiteral("cd")},
    {QStringLiteral("Chinese"),                                   QStringLiteral("cn")},
    {QStringLiteral("Croatian"),                                  QStringLiteral("hr")},
    {QStringLiteral("Czech"),                                     QStringLiteral("cz")},
    {QStringLiteral("Danish"),                                    QStringLiteral("dk")},
    {QStringLiteral("Dutch"),                                     QStringLiteral("nl")},
    {QStringLiteral("Dzongkha"),                                  QStringLiteral("bt")},
    {QStringLiteral("Estonian"),                                  QStringLiteral("ee")},
    {QStringLiteral("Persian"),                                   QStringLiteral("ir")},
    {QStringLiteral("Iraqi"),                                     QStringLiteral("iq")},
    {QStringLiteral("Faroese"),                                   QStringLiteral("fo")},
    {QStringLiteral("Finnish"),                                   QStringLiteral("fi")},
    {QStringLiteral("French"),                                    QStringLiteral("fr")},
    {QStringLiteral("English (Ghana)"),                           QStringLiteral("gh")},
    {QStringLiteral("French (Guinea)"),                           QStringLiteral("gn")},
    {QStringLiteral("Georgian"),                                  QStringLiteral("ge")},
    {QStringLiteral("German"),                                    QStringLiteral("de")},
    {QStringLiteral("Greek"),                                     QStringLiteral("gr")},
    {QStringLiteral("Hungarian"),                                 QStringLiteral("hu")},
    {QStringLiteral("Icelandic"),                                 QStringLiteral("is")},
    {QStringLiteral("Hebrew"),                                    QStringLiteral("il")},
    {QStringLiteral("Italian"),                                   QStringLiteral("it")},
    {QStringLiteral("Japanese"),                                  QStringLiteral("jp")},
    {QStringLiteral("Kyrgyz"),                                    QStringLiteral("kg")},
    {QStringLiteral("Khmer (Cambodia)"),                          QStringLiteral("kh")},
    {QStringLiteral("Kazakh"),                                    QStringLiteral("kz")},
    {QStringLiteral("Lao"),                                       QStringLiteral("la")},
    {QStringLiteral("Spanish (Latin American)"),                  QStringLiteral("latam")},
    {QStringLiteral("Lithuanian"),                                QStringLiteral("lt")},
    {QStringLiteral("Latvian"),                                   QStringLiteral("lv")},
    {QStringLiteral("Maori"),                                     QStringLiteral("mao")},
    {QStringLiteral("Montenegrin"),                               QStringLiteral("me")},
    {QStringLiteral("Macedonian"),                                QStringLiteral("mk")},
    {QStringLiteral("Maltese"),                                   QStringLiteral("mt")},
    {QStringLiteral("Mongolian"),                                 QStringLiteral("mn")},
    {QStringLiteral("Norwegian"),                                 QStringLiteral("no")},
    {QStringLiteral("Polish"),                                    QStringLiteral("pl")},
    {QStringLiteral("Portuguese"),                                QStringLiteral("pt")},
    {QStringLiteral("Romanian"),                                  QStringLiteral("ro")},
    {QStringLiteral("Russian"),                                   QStringLiteral("ru")},
    {QStringLiteral("Serbian"),                                   QStringLiteral("rs")},
    {QStringLiteral("Slovenian"),                                 QStringLiteral("si")},
    {QStringLiteral("Slovak"),                                    QStringLiteral("sk")},
    {QStringLiteral("Spanish"),                                   QStringLiteral("es")},
    {QStringLiteral("Swedish"),                                   QStringLiteral("se")},
    {QStringLiteral("German (Switzerland)"),                      QStringLiteral("ch")},
    {QStringLiteral("Arabic (Syria)"),                            QStringLiteral("sy")},
    {QStringLiteral("Tajik"),                                     QStringLiteral("tj")},
    {QStringLiteral("Sinhala (phonetic)"),                        QStringLiteral("lk")},
    {QStringLiteral("Thai"),                                      QStringLiteral("th")},
    {QStringLiteral("Turkish"),                                   QStringLiteral("tr")},
    {QStringLiteral("Taiwanese"),                                 QStringLiteral("tw")},
    {QStringLiteral("Ukrainian"),                                 QStringLiteral("ua")},
    {QStringLiteral("English (UK)"),                              QStringLiteral("gb")},
    {QStringLiteral("Uzbek"),                                     QStringLiteral("uz")},
    {QStringLiteral("Vietnamese"),                                QStringLiteral("vn")},
    {QStringLiteral("Korean"),                                    QStringLiteral("kr")},
    {QStringLiteral("Japanese (PC-98)"),                          QStringLiteral("nec_vndr/jp")},
    {QStringLiteral("Irish"),                                     QStringLiteral("ie")},
    {QStringLiteral("Urdu (Pakistan)"),                           QStringLiteral("pk")},
    {QStringLiteral("Dhivehi"),                                   QStringLiteral("mv")},
    {QStringLiteral("English (South Africa)"),                    QStringLiteral("za")},
    {QStringLiteral("Esperanto"),                                 QStringLiteral("epo")},
    {QStringLiteral("Nepali"),                                    QStringLiteral("np")},
    {QStringLiteral("English (Nigeria)"),                         QStringLiteral("ng")},
    {QStringLiteral("Amharic"),                                   QStringLiteral("et")},
    {QStringLiteral("Wolof"),                                     QStringLiteral("sn")},
    {QStringLiteral("Braille"),                                   QStringLiteral("brai")},
    {QStringLiteral("Turkmen"),                                   QStringLiteral("tm")},
    {QStringLiteral("Bambara"),                                   QStringLiteral("ml")},
    {QStringLiteral("Swahili (Tanzania)"),                        QStringLiteral("tz")},
    {QStringLiteral("French (Togo)"),                             QStringLiteral("tg")},
    {QStringLiteral("Swahili (Kenya)"),                           QStringLiteral("ke")},
    {QStringLiteral("Tswana"),                                    QStringLiteral("bw")},
    {QStringLiteral("Filipino"),                                  QStringLiteral("ph")},
    {QStringLiteral("Moldavian"),                                 QStringLiteral("md")},
    {QStringLiteral("Indonesian (Jawi)"),                         QStringLiteral("id")},
    {QStringLiteral("Malay (Jawi, Arabic Keyboard)"),             QStringLiteral("my")}
};

static QString layoutNameToLayout(const QString &layoutName)
{
    auto it = s_layoutNameToLayout.constFind(layoutName);
    if (it == s_layoutNameToLayout.constEnd()) {
        return QStringLiteral("--");
    }
    return *it;
}

KeyboardLayout::KeyboardLayout(Xkb *xkb)
    : QObject()
    , m_xkb(xkb)
    , m_notifierItem(nullptr)
{
}

KeyboardLayout::~KeyboardLayout() = default;

static QString translatedLayout(const QString &layout)
{
    return i18nd("xkeyboard-config", layout.toUtf8().constData());
}

void KeyboardLayout::init()
{
    QAction *switchKeyboardAction = new QAction(this);
    switchKeyboardAction->setObjectName(QStringLiteral("Switch to Next Keyboard Layout"));
    switchKeyboardAction->setProperty("componentName", QStringLiteral("KDE Keyboard Layout Switcher"));
    const QKeySequence sequence = QKeySequence(Qt::ALT+Qt::CTRL+Qt::Key_K);
    KGlobalAccel::self()->setDefaultShortcut(switchKeyboardAction, QList<QKeySequence>({sequence}));
    KGlobalAccel::self()->setShortcut(switchKeyboardAction, QList<QKeySequence>({sequence}));
    kwinApp()->platform()->setupActionForGlobalAccel(switchKeyboardAction);
    connect(switchKeyboardAction, &QAction::triggered, this, &KeyboardLayout::switchToNextLayout);

    QDBusConnection::sessionBus().connect(QString(),
                                          QStringLiteral("/Layouts"),
                                          QStringLiteral("org.kde.keyboard"),
                                          QStringLiteral("reloadConfig"),
                                          this,
                                          SLOT(reconfigure()));

    reconfigure();
}

void KeyboardLayout::initDBusInterface()
{
    if (m_xkb->numberOfLayouts() <= 1) {
        delete m_dbusInterface;
        m_dbusInterface = nullptr;
        return;
    }
    if (m_dbusInterface) {
        return;
    }
    m_dbusInterface = new KeyboardLayoutDBusInterface(m_xkb, this);
    connect(this, &KeyboardLayout::layoutChanged, m_dbusInterface,
        [this] {
            emit m_dbusInterface->currentLayoutChanged(m_xkb->layoutName());
        }
    );
    // TODO: the signal might be emitted even if the list didn't change
    connect(this, &KeyboardLayout::layoutsReconfigured, m_dbusInterface, &KeyboardLayoutDBusInterface::layoutListChanged);
}

void KeyboardLayout::initNotifierItem()
{
    bool showNotifier = true;
    bool showSingle = false;
    bool showFlag = false;
    bool showLabel = true;

    if (m_config) {
        const auto config = m_config->group(QStringLiteral("Layout"));
        showNotifier = config.readEntry("ShowLayoutIndicator", true);
        showSingle = config.readEntry("ShowSingle", false);
        showFlag = config.readEntry(QStringLiteral("ShowFlag"), false);
        showLabel = config.readEntry(QStringLiteral("ShowLabel"), true);
    }

    const bool shouldShow = showNotifier && (showSingle || m_xkb->numberOfLayouts() > 1);
    if (!shouldShow) {
        delete m_notifierItem;
        m_notifierItem = nullptr;
        m_iconProvider = nullptr;
        return;
    }

    auto displayMode = [showFlag, showLabel] {
        if (showFlag && showLabel) {
            return KeyboardLayoutIconProvider::Mode::LabelOnFlag;
        }
        if (showFlag) {
            return KeyboardLayoutIconProvider::Mode::Flag;
        }
        if (showLabel) {
            return KeyboardLayoutIconProvider::Mode::Label;
        }
        Q_UNREACHABLE();
    };

    if (m_notifierItem) {
        m_iconProvider->setMode(displayMode());
        return;
    }

    m_notifierItem = new KStatusNotifierItem(this);
    m_notifierItem->setCategory(KStatusNotifierItem::Hardware);
    m_notifierItem->setStatus(KStatusNotifierItem::Active);
    m_notifierItem->setToolTipTitle(i18nc("tooltip title", "Keyboard Layout"));
    m_notifierItem->setTitle(i18nc("tooltip title", "Keyboard Layout"));
    m_notifierItem->setToolTipIconByName(QStringLiteral("preferences-desktop-keyboard"));
    m_notifierItem->setStandardActionsEnabled(false);
    m_notifierItem->setIconByName(QStringLiteral("preferences-desktop-keyboard"));

    m_iconProvider = new KeyboardLayoutIconProvider(m_notifierItem);
    m_iconProvider->setFallbackIcon(QStringLiteral("preferences-desktop-keyboard"));
    m_iconProvider->setMode(displayMode());

    connect(m_notifierItem, &KStatusNotifierItem::activateRequested, this, &KeyboardLayout::switchToNextLayout);
    connect(m_notifierItem, &KStatusNotifierItem::scrollRequested, this,
        [this] (int delta, Qt::Orientation orientation) {
            if (orientation == Qt::Horizontal) {
                return;
            }
            if (delta > 0) {
                switchToNextLayout();
            } else {
                switchToPreviousLayout();
            }
        }
    );

    connect(m_iconProvider, &KeyboardLayoutIconProvider::iconsInvalidated,
            this, &KeyboardLayout::updateNotifier);

    m_notifierItem->setStatus(KStatusNotifierItem::Active);
}

void KeyboardLayout::switchToNextLayout()
{
    m_xkb->switchToNextLayout();
    checkLayoutChange();
}

void KeyboardLayout::switchToPreviousLayout()
{
    m_xkb->switchToPreviousLayout();
    checkLayoutChange();
}

void KeyboardLayout::switchToLayout(xkb_layout_index_t index)
{
    m_xkb->switchToLayout(index);
    checkLayoutChange();
}

void KeyboardLayout::reconfigure()
{
    if (m_config) {
        m_config->reparseConfiguration();
        const QString policyKey = m_config->group(QStringLiteral("Layout")).readEntry("SwitchMode", QStringLiteral("Global"));
        if (!m_policy || m_policy->name() != policyKey) {
            delete m_policy;
            m_policy = KeyboardLayoutSwitching::Policy::create(m_xkb, this, policyKey);
        }
    }
    m_xkb->reconfigure();
    resetLayout();
}

void KeyboardLayout::resetLayout()
{
    m_layout = m_xkb->currentLayout();
    initNotifierItem();
    updateNotifier();
    reinitNotifierMenu();
    loadShortcuts();
    emit layoutsReconfigured();

    initDBusInterface();
}

void KeyboardLayout::loadShortcuts()
{
    qDeleteAll(m_layoutShortcuts);
    m_layoutShortcuts.clear();
    const auto layouts = m_xkb->layoutNames();
    const QString componentName = QStringLiteral("KDE Keyboard Layout Switcher");
    for (auto it = layouts.begin(); it != layouts.end(); it++) {
        // layout name is translated in the action name in keyboard kcm!
        const QString action = QStringLiteral("Switch keyboard layout to %1").arg(translatedLayout(it.value()));
        const auto shortcuts = KGlobalAccel::self()->globalShortcut(componentName, action);
        if (shortcuts.isEmpty()) {
            continue;
        }
        QAction *a = new QAction(this);
        a->setObjectName(action);
        a->setProperty("componentName", componentName);
        connect(a, &QAction::triggered, this,
                std::bind(&KeyboardLayout::switchToLayout, this, it.key()));
        KGlobalAccel::self()->setShortcut(a, shortcuts, KGlobalAccel::Autoloading);
        m_layoutShortcuts << a;
    }
}

void KeyboardLayout::keyEvent(KeyEvent *event)
{
    if (!event->isAutoRepeat()) {
        checkLayoutChange();
    }
}

void KeyboardLayout::checkLayoutChange()
{
    const auto layout = m_xkb->currentLayout();
    if (m_layout == layout) {
        return;
    }
    m_layout = layout;
    notifyLayoutChange();
    updateNotifier();
    emit layoutChanged();
}

void KeyboardLayout::notifyLayoutChange()
{
    // notify OSD service about the new layout
    QDBusMessage msg = QDBusMessage::createMethodCall(
        QStringLiteral("org.kde.plasmashell"),
        QStringLiteral("/org/kde/osdService"),
        QStringLiteral("org.kde.osdService"),
        QStringLiteral("kbdLayoutChanged"));

    msg << translatedLayout(m_xkb->layoutName());

    QDBusConnection::sessionBus().asyncCall(msg);
}

void KeyboardLayout::updateNotifier()
{
    if (!m_notifierItem) {
        return;
    }

    const QString layoutName = m_xkb->layoutName();
    const QString layout = layoutNameToLayout(layoutName);

    m_notifierItem->setToolTipSubTitle(translatedLayout(layoutName));

    switch (m_iconProvider->mode()) {
    case KeyboardLayoutIconProvider::Mode::Label:
        m_notifierItem->setToolTipIconByName(QStringLiteral("preferences-desktop-keyboard"));
        break;

    case KeyboardLayoutIconProvider::Mode::Flag:
    case KeyboardLayoutIconProvider::Mode::LabelOnFlag:
        m_notifierItem->setToolTipIconByPixmap(m_iconProvider->flag(layout));
        break;

    default:
        Q_UNREACHABLE();
        break;
    }

    m_notifierItem->setIconByPixmap(m_iconProvider->icon(layout));
}

void KeyboardLayout::reinitNotifierMenu()
{
    if (!m_notifierItem) {
        return;
    }
    const auto layouts = m_xkb->layoutNames();

    QMenu *menu = m_notifierItem->contextMenu();
    if (!menu) {
        menu = new QMenu();
        m_notifierItem->setContextMenu(menu);
    }

    menu->clear();

    for (auto it = layouts.begin(); it != layouts.end(); it++) {
        const QString text = translatedLayout(*it);
        auto handler = std::bind(&KeyboardLayout::switchToLayout, this, it.key());

        QIcon icon;
        switch (m_iconProvider->mode()) {
        case KeyboardLayoutIconProvider::Mode::Label:
            break;

        case KeyboardLayoutIconProvider::Mode::Flag:
        case KeyboardLayoutIconProvider::Mode::LabelOnFlag:
            icon = m_iconProvider->flag(layoutNameToLayout(*it));
            break;

        default:
            Q_UNREACHABLE();
            break;
        }

        menu->addAction(icon, text, handler);
    }

    menu->addSeparator();
    menu->addAction(QIcon::fromTheme(QStringLiteral("configure")), i18n("Configure Layouts..."), this,
        [this] {
            // TODO: introduce helper function to start kcmshell5
            QProcess *p = new Process(this);
            p->setArguments(QStringList{QStringLiteral("--args=--tab=layouts"), QStringLiteral("kcm_keyboard")});
            p->setProcessEnvironment(kwinApp()->processStartupEnvironment());
            p->setProgram(QStringLiteral("kcmshell5"));
            connect(p, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), p, &QProcess::deleteLater);
            connect(p, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error), this,
                [] (QProcess::ProcessError e) {
                    if (e == QProcess::FailedToStart) {
                        qCDebug(KWIN_CORE) << "Failed to start kcmshell5";
                    }
                }
            );
            p->start();
        }
    );
}

static const QString s_keyboardService = QStringLiteral("org.kde.keyboard");
static const QString s_keyboardObject = QStringLiteral("/Layouts");

KeyboardLayoutDBusInterface::KeyboardLayoutDBusInterface(Xkb *xkb, KeyboardLayout *parent)
    : QObject(parent)
    , m_xkb(xkb)
    , m_keyboardLayout(parent)
{
    QDBusConnection::sessionBus().registerService(s_keyboardService);
    QDBusConnection::sessionBus().registerObject(s_keyboardObject, this, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
}

KeyboardLayoutDBusInterface::~KeyboardLayoutDBusInterface()
{
    QDBusConnection::sessionBus().unregisterService(s_keyboardService);
}

bool KeyboardLayoutDBusInterface::setLayout(const QString &layout)
{
    const auto layouts = m_xkb->layoutNames();
    auto it = layouts.begin();
    for (; it !=layouts.end(); it++) {
        if (it.value() == layout) {
            break;
        }
    }
    if (it == layouts.end()) {
        return false;
    }
    m_xkb->switchToLayout(it.key());
    m_keyboardLayout->checkLayoutChange();
    return true;
}

QString KeyboardLayoutDBusInterface::getCurrentLayout()
{
    return m_xkb->layoutName();
}

QStringList KeyboardLayoutDBusInterface::getLayoutsList()
{
    const auto layouts = m_xkb->layoutNames();
    QStringList ret;
    for (auto it = layouts.begin(); it != layouts.end(); it++) {
        ret << it.value();
    }
    return ret;
}

QString KeyboardLayoutDBusInterface::getLayoutDisplayName(const QString &layout)
{
    return translatedLayout(layout);
}

}
