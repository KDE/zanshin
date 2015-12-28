/*
 * Copyright (C) 2013-2015 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ldapclientsearchconfig.h"
#include <kldap/ldapserver.h>

#include <KConfig>
#include <KConfigGroup>
#include <KMessageBox>
#include <KLocalizedString>
#include <kwallet.h>

using namespace KLDAP;

class Q_DECL_HIDDEN LdapClientSearchConfig::Private
{
public:
    Private()
        : useWallet(false),
          askWallet(true),
          wallet(Q_NULLPTR)
    {

    }
    ~Private()
    {
        if (useWallet) {
            wallet->deleteLater();
            wallet = Q_NULLPTR;
        }
    }
    bool useWallet;
    bool askWallet;
    KWallet::Wallet *wallet;
};

Q_GLOBAL_STATIC_WITH_ARGS(KConfig, s_config, (QLatin1String("kabldaprc"), KConfig::NoGlobals))

KConfig *LdapClientSearchConfig::config()
{
    return s_config;
}

LdapClientSearchConfig::LdapClientSearchConfig(QObject *parent)
    : QObject(parent), d(new LdapClientSearchConfig::Private())
{
}

LdapClientSearchConfig::~LdapClientSearchConfig()
{
    delete d;
}

void LdapClientSearchConfig::readConfig(KLDAP::LdapServer &server, KConfigGroup &config, int j, bool active)
{
    QString prefix;
    if (active) {
        prefix = QStringLiteral("Selected");
    }

    const QString host =  config.readEntry(prefix + QStringLiteral("Host%1").arg(j),
                                           QString()).trimmed();
    if (!host.isEmpty()) {
        server.setHost(host);
    }

    const int port = config.readEntry(prefix + QStringLiteral("Port%1").arg(j), 389);
    server.setPort(port);

    const QString base = config.readEntry(prefix + QStringLiteral("Base%1").arg(j),
                                          QString()).trimmed();
    if (!base.isEmpty()) {
        server.setBaseDn(KLDAP::LdapDN(base));
    }

    const QString user = config.readEntry(prefix + QStringLiteral("User%1").arg(j),
                                          QString()).trimmed();
    if (!user.isEmpty()) {
        server.setUser(user);
    }

    const QString bindDN = config.readEntry(prefix + QStringLiteral("Bind%1").arg(j), QString()).trimmed();
    if (!bindDN.isEmpty()) {
        server.setBindDn(bindDN);
    }

    const QString pwdBindBNEntry = prefix + QStringLiteral("PwdBind%1").arg(j);
    QString pwdBindDN = config.readEntry(pwdBindBNEntry, QString());
    if (!pwdBindDN.isEmpty()) {
        if (d->askWallet && KMessageBox::Yes == KMessageBox::questionYesNo(Q_NULLPTR, i18n("LDAP password is stored as clear text, do you want to store it in kwallet?"),
                i18n("Store clear text password in KWallet"),
                KStandardGuiItem::yes(),
                KStandardGuiItem::no(),
                QStringLiteral("DoAskToStoreToKwallet"))) {
            d->wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), 0);
            if (d->wallet) {
                connect(d->wallet, &KWallet::Wallet::walletClosed, this, &LdapClientSearchConfig::slotWalletClosed);
                d->useWallet = true;
                if (!d->wallet->hasFolder(QStringLiteral("ldapclient"))) {
                    d->wallet->createFolder(QStringLiteral("ldapclient"));
                }
                d->wallet->setFolder(QStringLiteral("ldapclient"));
                d->wallet->writePassword(pwdBindBNEntry, pwdBindDN);
                config.deleteEntry(pwdBindBNEntry);
                config.sync();
            }
        }
        server.setPassword(pwdBindDN);
    } else if (d->askWallet) { //Look at in Wallet
        d->wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), 0);
        if (d->wallet) {
            d->useWallet = true;
            if (!d->wallet->setFolder(QStringLiteral("ldapclient"))) {
                d->wallet->createFolder(QStringLiteral("ldapclient"));
                d->wallet->setFolder(QStringLiteral("ldapclient"));
            }
            d->wallet->readPassword(pwdBindBNEntry, pwdBindDN);
            if (!pwdBindDN.isEmpty()) {
                server.setPassword(pwdBindDN);
            }
        } else {
            d->useWallet = false;
        }
    }

    server.setTimeLimit(config.readEntry(prefix + QStringLiteral("TimeLimit%1").arg(j), 0));
    server.setSizeLimit(config.readEntry(prefix + QStringLiteral("SizeLimit%1").arg(j), 0));
    server.setPageSize(config.readEntry(prefix + QStringLiteral("PageSize%1").arg(j), 0));
    server.setVersion(config.readEntry(prefix + QStringLiteral("Version%1").arg(j), 3));

    QString tmp;
    tmp = config.readEntry(prefix + QStringLiteral("Security%1").arg(j),
                           QStringLiteral("None"));
    server.setSecurity(KLDAP::LdapServer::None);
    if (tmp == QLatin1String("SSL")) {
        server.setSecurity(KLDAP::LdapServer::SSL);
    } else if (tmp == QLatin1String("TLS")) {
        server.setSecurity(KLDAP::LdapServer::TLS);
    }

    tmp = config.readEntry(prefix + QStringLiteral("Auth%1").arg(j),
                           QStringLiteral("Anonymous"));
    server.setAuth(KLDAP::LdapServer::Anonymous);
    if (tmp == QLatin1String("Simple")) {
        server.setAuth(KLDAP::LdapServer::Simple);
    } else if (tmp == QLatin1String("SASL")) {
        server.setAuth(KLDAP::LdapServer::SASL);
    }

    server.setMech(config.readEntry(prefix + QStringLiteral("Mech%1").arg(j), QString()));
    server.setFilter(config.readEntry(prefix + QStringLiteral("UserFilter%1").arg(j), QString()));
}

void LdapClientSearchConfig::writeConfig(const KLDAP::LdapServer &server, KConfigGroup &config, int j, bool active)
{
    QString prefix;
    if (active) {
        prefix = QStringLiteral("Selected");
    }

    config.writeEntry(prefix + QStringLiteral("Host%1").arg(j), server.host());
    config.writeEntry(prefix + QStringLiteral("Port%1").arg(j), server.port());
    config.writeEntry(prefix + QStringLiteral("Base%1").arg(j), server.baseDn().toString());
    config.writeEntry(prefix + QStringLiteral("User%1").arg(j), server.user());
    config.writeEntry(prefix + QStringLiteral("Bind%1").arg(j), server.bindDn());

    const QString passwordEntry = prefix + QStringLiteral("PwdBind%1").arg(j);
    const QString password = server.password();
    if (!password.isEmpty()) {
        if (d->useWallet && !d->wallet) {
            d->wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), 0);
        }
        if (d->wallet) {
            d->wallet->writePassword(passwordEntry, password);
        } else {
            config.writeEntry(passwordEntry, password);
            d->useWallet = false;
        }
    }

    config.writeEntry(prefix + QStringLiteral("TimeLimit%1").arg(j), server.timeLimit());
    config.writeEntry(prefix + QStringLiteral("SizeLimit%1").arg(j), server.sizeLimit());
    config.writeEntry(prefix + QStringLiteral("PageSize%1").arg(j), server.pageSize());
    config.writeEntry(prefix + QStringLiteral("Version%1").arg(j), server.version());
    QString tmp;
    switch (server.security()) {
    case KLDAP::LdapServer::TLS:
        tmp = QStringLiteral("TLS");
        break;
    case KLDAP::LdapServer::SSL:
        tmp = QStringLiteral("SSL");
        break;
    default:
        tmp = QStringLiteral("None");
    }
    config.writeEntry(prefix + QStringLiteral("Security%1").arg(j), tmp);
    switch (server.auth()) {
    case KLDAP::LdapServer::Simple:
        tmp = QStringLiteral("Simple");
        break;
    case KLDAP::LdapServer::SSL:
        tmp = QStringLiteral("SASL");
        break;
    default:
        tmp = QStringLiteral("Anonymous");
    }
    config.writeEntry(prefix + QStringLiteral("Auth%1").arg(j), tmp);
    config.writeEntry(prefix + QStringLiteral("Mech%1").arg(j), server.mech());
    config.writeEntry(prefix + QStringLiteral("UserFilter%1").arg(j), server.filter().trimmed());
}

void LdapClientSearchConfig::slotWalletClosed()
{
    delete d->wallet;
    d->wallet = Q_NULLPTR;
}

void LdapClientSearchConfig::askForWallet(bool askForWallet)
{
    d->askWallet = askForWallet;
}
