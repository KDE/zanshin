/*
  This file is part of libkldap.

  Copyright (c) 2002-2010 Tobias Koenig <tokoe@kde.org>

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

#include "addhostdialog.h"

#include <QHBoxLayout>
#include <KSharedConfig>
#include <kacceleratormanager.h>
#include <kldap/ldapserver.h>
#include <kldap/ldapconfigwidget.h>
#include <KLocalizedString>
#include <QDialogButtonBox>
#include <KConfigGroup>
#include <QPushButton>
#include <QVBoxLayout>

using namespace KLDAP;
class KLDAP::AddHostDialogPrivate
{
public:
    AddHostDialogPrivate(AddHostDialog *qq)
        : mCfg(Q_NULLPTR),
          mServer(Q_NULLPTR),
          mOkButton(Q_NULLPTR),
          q(qq)
    {
    }
    ~AddHostDialogPrivate()
    {
        writeConfig();
    }

    void readConfig();
    void writeConfig();
    KLDAP::LdapConfigWidget *mCfg;
    KLDAP::LdapServer *mServer;
    QPushButton *mOkButton;
    AddHostDialog *q;
};

void AddHostDialogPrivate::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "AddHostDialog");
    const QSize size = group.readEntry("Size", QSize(600, 400));
    if (size.isValid()) {
        q->resize(size);
    }
}

void AddHostDialogPrivate::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "AddHostDialog");
    group.writeEntry("Size", q->size());
    group.sync();
}

AddHostDialog::AddHostDialog(KLDAP::LdapServer *server, QWidget *parent)
    : QDialog(parent),
      d(new KLDAP::AddHostDialogPrivate(this))
{
    setWindowTitle(i18n("Add Host"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    d->mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    d->mOkButton->setDefault(true);
    d->mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AddHostDialog::reject);
    d->mOkButton->setDefault(true);
    setModal(true);

    d->mServer = server;

    QWidget *page = new QWidget(this);
    mainLayout->addWidget(page);
    mainLayout->addWidget(buttonBox);
    QHBoxLayout *layout = new QHBoxLayout(page);
    layout->setMargin(0);

    d->mCfg = new KLDAP::LdapConfigWidget(
        KLDAP::LdapConfigWidget::W_USER |
        KLDAP::LdapConfigWidget::W_PASS |
        KLDAP::LdapConfigWidget::W_BINDDN |
        KLDAP::LdapConfigWidget::W_REALM |
        KLDAP::LdapConfigWidget::W_HOST |
        KLDAP::LdapConfigWidget::W_PORT |
        KLDAP::LdapConfigWidget::W_VER |
        KLDAP::LdapConfigWidget::W_TIMELIMIT |
        KLDAP::LdapConfigWidget::W_SIZELIMIT |
        KLDAP::LdapConfigWidget::W_PAGESIZE |
        KLDAP::LdapConfigWidget::W_DN |
        KLDAP::LdapConfigWidget::W_FILTER |
        KLDAP::LdapConfigWidget::W_SECBOX |
        KLDAP::LdapConfigWidget::W_AUTHBOX,
        page);

    layout->addWidget(d->mCfg);
    d->mCfg->setHost(d->mServer->host());
    d->mCfg->setPort(d->mServer->port());
    d->mCfg->setDn(d->mServer->baseDn());
    d->mCfg->setUser(d->mServer->user());
    d->mCfg->setBindDn(d->mServer->bindDn());
    d->mCfg->setPassword(d->mServer->password());
    d->mCfg->setTimeLimit(d->mServer->timeLimit());
    d->mCfg->setSizeLimit(d->mServer->sizeLimit());
    d->mCfg->setPageSize(d->mServer->pageSize());
    d->mCfg->setVersion(d->mServer->version());
    d->mCfg->setFilter(d->mServer->filter());
    switch (d->mServer->security()) {
    case KLDAP::LdapServer::TLS:
        d->mCfg->setSecurity(KLDAP::LdapConfigWidget::TLS);
        break;
    case KLDAP::LdapServer::SSL:
        d->mCfg->setSecurity(KLDAP::LdapConfigWidget::SSL);
        break;
    default:
        d->mCfg->setSecurity(KLDAP::LdapConfigWidget::None);
    }

    switch (d->mServer->auth()) {
    case KLDAP::LdapServer::Simple:
        d->mCfg->setAuth(KLDAP::LdapConfigWidget::Simple);
        break;
    case KLDAP::LdapServer::SASL:
        d->mCfg->setAuth(KLDAP::LdapConfigWidget::SASL);
        break;
    default:
        d->mCfg->setAuth(KLDAP::LdapConfigWidget::Anonymous);
    }
    d->mCfg->setMech(d->mServer->mech());

    KAcceleratorManager::manage(this);
    connect(d->mCfg, &KLDAP::LdapConfigWidget::hostNameChanged, this, &AddHostDialog::slotHostEditChanged);
    connect(d->mOkButton, &QPushButton::clicked, this, &AddHostDialog::slotOk);
    d->mOkButton->setEnabled(!d->mServer->host().isEmpty());
    d->readConfig();
}

AddHostDialog::~AddHostDialog()
{
    delete d;
}

void AddHostDialog::slotHostEditChanged(const QString &text)
{
    d->mOkButton->setEnabled(!text.isEmpty());
}

void AddHostDialog::slotOk()
{
    d->mServer->setHost(d->mCfg->host());
    d->mServer->setPort(d->mCfg->port());
    d->mServer->setBaseDn(d->mCfg->dn());
    d->mServer->setUser(d->mCfg->user());
    d->mServer->setBindDn(d->mCfg->bindDn());
    d->mServer->setPassword(d->mCfg->password());
    d->mServer->setTimeLimit(d->mCfg->timeLimit());
    d->mServer->setSizeLimit(d->mCfg->sizeLimit());
    d->mServer->setPageSize(d->mCfg->pageSize());
    d->mServer->setVersion(d->mCfg->version());
    d->mServer->setFilter(d->mCfg->filter());
    switch (d->mCfg->security()) {
    case KLDAP::LdapConfigWidget::TLS:
        d->mServer->setSecurity(KLDAP::LdapServer::TLS);
        break;
    case KLDAP::LdapConfigWidget::SSL:
        d->mServer->setSecurity(KLDAP::LdapServer::SSL);
        break;
    default:
        d->mServer->setSecurity(KLDAP::LdapServer::None);
    }
    switch (d->mCfg->auth()) {
    case KLDAP::LdapConfigWidget::Simple:
        d->mServer->setAuth(KLDAP::LdapServer::Simple);
        break;
    case KLDAP::LdapConfigWidget::SASL:
        d->mServer->setAuth(KLDAP::LdapServer::SASL);
        break;
    default:
        d->mServer->setAuth(KLDAP::LdapServer::Anonymous);
    }
    d->mServer->setMech(d->mCfg->mech());
    QDialog::accept();
}

#include "moc_addhostdialog.cpp"
