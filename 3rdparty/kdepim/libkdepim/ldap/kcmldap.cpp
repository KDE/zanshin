/*
  This file is part of libkldap.

  Copyright (c) 2002-2009 Tobias Koenig <tokoe@kde.org>
  Copyright (C) 2013-2015 Laurent Montel <montel@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General  Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "kcmldap_p.h"

#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QToolButton>
#include <QVBoxLayout>
#include <QPushButton>

#include <KAboutData>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <qdialogbuttonbox.h>
#include <KPluginFactory>
#include <QHBoxLayout>
#include <kiconloader.h>
#include <KLocalizedString>
#include <KMessageBox>

#include "ldapclientsearch.h"
#include "ldapclientsearchconfig.h"
#include <kldap/ldapserver.h>

#include "addhostdialog.h"

K_PLUGIN_FACTORY_WITH_JSON(KCMLdapFactory, "kcmldap.json", registerPlugin<KCMLdap>();)

class LDAPItem : public QListWidgetItem
{
public:
    LDAPItem(QListWidget *parent, const KLDAP::LdapServer &server, bool isActive = false)
        : QListWidgetItem(parent, QListWidgetItem::UserType),
          mIsActive(isActive)
    {
        setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
        setCheckState(isActive ? Qt::Checked : Qt::Unchecked);
        setServer(server);
    }

    void setServer(const KLDAP::LdapServer &server)
    {
        mServer = server;

        setText(mServer.host());
    }

    const KLDAP::LdapServer &server() const
    {
        return mServer;
    }

    void setIsActive(bool isActive)
    {
        mIsActive = isActive;
    }
    bool isActive() const
    {
        return mIsActive;
    }

private:
    KLDAP::LdapServer mServer;
    bool mIsActive;
};

KCMLdap::KCMLdap(QWidget *parent, const QVariantList &)
    : KCModule(parent)
{
    setButtons(KCModule::Apply);
    KAboutData *about = new KAboutData(QStringLiteral("kcmldap"),
                                       i18n("kcmldap"),
                                       QString(),
                                       i18n("LDAP Server Settings"),
                                       KAboutLicense::LGPL,
                                       i18n("(c) 2009 - 2010 Tobias Koenig"));
    about->addAuthor(i18n("Tobias Koenig"), QString(), QStringLiteral("tokoe@kde.org"));
    setAboutData(about);

    mClientSearchConfig = new KLDAP::LdapClientSearchConfig;
    initGUI();

    connect(mHostListView, &QListWidget::currentItemChanged, this, &KCMLdap::slotSelectionChanged);
    connect(mHostListView, &QListWidget::itemDoubleClicked, this, &KCMLdap::slotEditHost);
    connect(mHostListView, &QListWidget::itemClicked, this, &KCMLdap::slotItemClicked);

    connect(mUpButton, &QToolButton::clicked, this, &KCMLdap::slotMoveUp);
    connect(mDownButton, &QToolButton::clicked, this, &KCMLdap::slotMoveDown);
}

KCMLdap::~KCMLdap()
{
    delete mClientSearchConfig;
}

void KCMLdap::slotSelectionChanged(QListWidgetItem *item)
{
    bool state = (item != Q_NULLPTR);
    mEditButton->setEnabled(state);
    mRemoveButton->setEnabled(state);
    mDownButton->setEnabled(item && (mHostListView->row(item) != (mHostListView->count() - 1)));
    mUpButton->setEnabled(item && (mHostListView->row(item) != 0));
}

void KCMLdap::slotItemClicked(QListWidgetItem *item)
{
    LDAPItem *ldapItem = dynamic_cast<LDAPItem *>(item);
    if (!ldapItem) {
        return;
    }

    if ((ldapItem->checkState() == Qt::Checked) != ldapItem->isActive()) {
        Q_EMIT changed(true);
        ldapItem->setIsActive(ldapItem->checkState() == Qt::Checked);
    }
}

void KCMLdap::slotAddHost()
{
    KLDAP::LdapServer server;
    KLDAP::AddHostDialog dlg(&server, dialogParent());

    if (dlg.exec() && !server.host().isEmpty()) {   //krazy:exclude=crashy
        new LDAPItem(mHostListView, server);

        Q_EMIT changed(true);
    }
}

void KCMLdap::slotEditHost()
{
    LDAPItem *item = dynamic_cast<LDAPItem *>(mHostListView->currentItem());
    if (!item) {
        return;
    }

    KLDAP::LdapServer server = item->server();
    KLDAP::AddHostDialog dlg(&server, dialogParent());
    dlg.setWindowTitle(i18n("Edit Host"));

    if (dlg.exec() && !server.host().isEmpty()) {   //krazy:exclude=crashy
        item->setServer(server);

        Q_EMIT changed(true);
    }
}

void KCMLdap::slotRemoveHost()
{
    QListWidgetItem *item = mHostListView->currentItem();
    if (!item) {
        return;
    }
    LDAPItem *ldapItem = dynamic_cast<LDAPItem *>(item);
    if (KMessageBox::No == KMessageBox::questionYesNo(this, i18n("Do you want to remove setting for host \"%1\"?", ldapItem->server().host()), i18n("Remove Host"))) {
        return;
    }

    delete mHostListView->takeItem(mHostListView->currentRow());

    slotSelectionChanged(mHostListView->currentItem());

    Q_EMIT changed(true);
}

static void swapItems(LDAPItem *item, LDAPItem *other)
{
    KLDAP::LdapServer server = item->server();
    bool isActive = item->isActive();
    item->setServer(other->server());
    item->setIsActive(other->isActive());
    item->setCheckState(other->isActive() ? Qt::Checked : Qt::Unchecked);
    other->setServer(server);
    other->setIsActive(isActive);
    other->setCheckState(isActive ? Qt::Checked : Qt::Unchecked);
}

void KCMLdap::slotMoveUp()
{
    const QList<QListWidgetItem *> selectedItems = mHostListView->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }

    LDAPItem *item = static_cast<LDAPItem *>(mHostListView->selectedItems().first());
    if (!item) {
        return;
    }

    LDAPItem *above = static_cast<LDAPItem *>(mHostListView->item(mHostListView->row(item) - 1));
    if (!above) {
        return;
    }

    swapItems(item, above);

    mHostListView->setCurrentItem(above);
    above->setSelected(true);

    Q_EMIT changed(true);
}

void KCMLdap::slotMoveDown()
{
    const QList<QListWidgetItem *> selectedItems = mHostListView->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }

    LDAPItem *item = static_cast<LDAPItem *>(mHostListView->selectedItems().first());
    if (!item) {
        return;
    }

    LDAPItem *below = static_cast<LDAPItem *>(mHostListView->item(mHostListView->row(item) + 1));
    if (!below) {
        return;
    }

    swapItems(item, below);

    mHostListView->setCurrentItem(below);
    below->setSelected(true);

    Q_EMIT changed(true);
}

void KCMLdap::load()
{
    mHostListView->clear();
    KConfig *config = KLDAP::LdapClientSearchConfig::config();
    KConfigGroup group(config, "LDAP");

    uint count = group.readEntry("NumSelectedHosts", 0);
    for (uint i = 0; i < count; ++i) {
        KLDAP::LdapServer server;
        mClientSearchConfig->readConfig(server, group, i, true);
        LDAPItem *item = new LDAPItem(mHostListView, server, true);
        item->setCheckState(Qt::Checked);
    }

    count = group.readEntry("NumHosts", 0);
    for (uint i = 0; i < count; ++i) {
        KLDAP::LdapServer server;
        mClientSearchConfig->readConfig(server, group, i, false);
        new LDAPItem(mHostListView, server);
    }

    Q_EMIT changed(false);
}

void KCMLdap::save()
{
    KConfig *config = KLDAP::LdapClientSearchConfig::config();
    config->deleteGroup("LDAP");

    KConfigGroup group(config, "LDAP");

    uint selected = 0;
    uint unselected = 0;
    for (int i = 0; i < mHostListView->count(); ++i) {
        LDAPItem *item = dynamic_cast<LDAPItem *>(mHostListView->item(i));
        if (!item) {
            continue;
        }

        KLDAP::LdapServer server = item->server();
        if (item->checkState() == Qt::Checked) {
            mClientSearchConfig->writeConfig(server, group, selected, true);
            selected++;
        } else {
            mClientSearchConfig->writeConfig(server, group, unselected, false);
            unselected++;
        }
    }

    group.writeEntry("NumSelectedHosts", selected);
    group.writeEntry("NumHosts", unselected);
    config->sync();

    Q_EMIT changed(false);
}

void KCMLdap::defaults()
{
    // add default configuration here
}

void KCMLdap::initGUI()
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    setLayout(layout);

    QGroupBox *groupBox = new QGroupBox(i18n("LDAP Servers"), this);
    QVBoxLayout *mainLayout = new QVBoxLayout(groupBox);

    // Contents of the QVGroupBox: label and hbox
    QLabel *label = new QLabel(i18n("Check all servers that should be used:"));
    mainLayout->addWidget(label);

    QWidget *hBox = new QWidget;
    QHBoxLayout *hBoxHBoxLayout = new QHBoxLayout(hBox);
    hBoxHBoxLayout->setMargin(0);
    hBoxHBoxLayout->setSpacing(6);
    mainLayout->addWidget(hBox);
    // Contents of the hbox: listview and up/down buttons on the right (vbox)
    mHostListView = new QListWidget(hBox);
    hBoxHBoxLayout->addWidget(mHostListView);
    mHostListView->setSortingEnabled(false);

    QWidget *upDownBox = new QWidget(hBox);
    QVBoxLayout *upDownBoxVBoxLayout = new QVBoxLayout(upDownBox);
    upDownBoxVBoxLayout->setMargin(0);
    hBoxHBoxLayout->addWidget(upDownBox);
    upDownBoxVBoxLayout->setSpacing(6);
    mUpButton = new QToolButton(upDownBox);
    upDownBoxVBoxLayout->addWidget(mUpButton);
    mUpButton->setIcon(QIcon::fromTheme(QStringLiteral("go-up")));
    mUpButton->setIconSize(QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall));
    mUpButton->setEnabled(false);   // b/c no item is selected yet

    mDownButton = new QToolButton(upDownBox);
    upDownBoxVBoxLayout->addWidget(mDownButton);
    mDownButton->setIcon(QIcon::fromTheme(QStringLiteral("go-down")));
    mDownButton->setIconSize(QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall));
    mDownButton->setEnabled(false);   // b/c no item is selected yet

    QWidget *spacer = new QWidget(upDownBox);
    upDownBoxVBoxLayout->addWidget(spacer);
    upDownBoxVBoxLayout->setStretchFactor(spacer, 100);

    layout->addWidget(groupBox);

    QDialogButtonBox *buttons = new QDialogButtonBox(this);
    QPushButton *add = buttons->addButton(i18n("&Add Host..."),
                                          QDialogButtonBox::ActionRole);
    connect(add, &QPushButton::clicked, this, &KCMLdap::slotAddHost);
    mEditButton = buttons->addButton(i18n("&Edit Host..."),
                                     QDialogButtonBox::ActionRole);
    connect(mEditButton, &QPushButton::clicked, this, &KCMLdap::slotEditHost);
    mEditButton->setEnabled(false);
    mRemoveButton = buttons->addButton(i18n("&Remove Host"),
                                       QDialogButtonBox::ActionRole);
    connect(mRemoveButton, &QPushButton::clicked, this, &KCMLdap::slotRemoveHost);
    mRemoveButton->setEnabled(false);
    buttons->layout();

    layout->addWidget(buttons);

    resize(QSize(460, 300).expandedTo(sizeHint()));
}

QWidget *KCMLdap::dialogParent()
{
    return this;
}

#include "kcmldap.moc"
