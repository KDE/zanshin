/* kldapclient.cpp - LDAP access
 *      Copyright (C) 2002 Klarälvdalens Datakonsult AB
 *
 *      Author: Steffen Hansen <hansen@kde.org>
 *
 *      Ported to KABC by Daniel Molkentin <molkentin@kde.org>
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

#include "ldapclient.h"
#include "ldapclient_debug.h"

#include <kldap/ldapobject.h>
#include <kldap/ldapserver.h>
#include <kldap/ldapurl.h>
#include <kldap/ldif.h>

#include <kio/job.h>

#include <QPointer>

using namespace KLDAP;

class Q_DECL_HIDDEN LdapClient::Private
{
public:
    Private(LdapClient *qq)
        : q(qq),
          mJob(Q_NULLPTR),
          mActive(false),
          mClientNumber(0)
    {
    }

    ~Private()
    {
        q->cancelQuery();
    }

    void startParseLDIF();
    void parseLDIF(const QByteArray &data);
    void endParseLDIF();
    void finishCurrentObject();

    void slotData(KIO::Job *, const QByteArray &data);
    void slotData(const QByteArray &data);
    void slotInfoMessage(KJob *, const QString &info, const QString &);
    void slotDone();

    LdapClient *q;

    KLDAP::LdapServer mServer;
    QString mScope;
    QStringList mAttrs;

    QPointer<KJob> mJob;
    bool mActive;

    KLDAP::LdapObject mCurrentObject;
    KLDAP::Ldif mLdif;
    int mClientNumber;
    int mCompletionWeight;

};

LdapClient::LdapClient(int clientNumber, QObject *parent)
    : QObject(parent), d(new Private(this))
{
    d->mClientNumber = clientNumber;
    d->mCompletionWeight = 50 - d->mClientNumber;
}

LdapClient::~LdapClient()
{
    delete d;
}

bool LdapClient::isActive() const
{
    return d->mActive;
}

void LdapClient::setServer(const KLDAP::LdapServer &server)
{
    d->mServer = server;
}

const KLDAP::LdapServer LdapClient::server() const
{
    return d->mServer;
}

void LdapClient::setAttributes(const QStringList &attrs)
{
    d->mAttrs = attrs;
    d->mAttrs << QStringLiteral("objectClass"); // via objectClass we detect distribution lists
}

QStringList LdapClient::attributes() const
{
    return d->mAttrs;
}

void LdapClient::setScope(const QString &scope)
{
    d->mScope = scope;
}

void LdapClient::startQuery(const QString &filter)
{
    cancelQuery();
    KLDAP::LdapUrl url;

    url = d->mServer.url();

    url.setAttributes(d->mAttrs);
    url.setScope(d->mScope == QLatin1String("one") ? KLDAP::LdapUrl::One : KLDAP::LdapUrl::Sub);
    const QString userFilter = url.filter();
    QString finalFilter = filter;
    // combine the filter set by the user in the config dialog (url.filter()) and the filter from this query
    if (!userFilter.isEmpty()) {
        finalFilter = QLatin1String("&(") + finalFilter + QLatin1String(")(") + userFilter + QLatin1Char(')');
    }
    url.setFilter(QLatin1Char('(') + finalFilter + QLatin1Char(')'));

    qCDebug(LDAPCLIENT_LOG) << "LdapClient: Doing query:" << url.toDisplayString();

    d->startParseLDIF();
    d->mActive = true;
    d->mJob = KIO::get(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(d->mJob, SIGNAL(data(KIO::Job*,QByteArray)),
            this, SLOT(slotData(KIO::Job*,QByteArray)));
    connect(d->mJob, SIGNAL(infoMessage(KJob*,QString,QString)),
            this, SLOT(slotInfoMessage(KJob*,QString,QString)));
    connect(d->mJob, SIGNAL(result(KJob*)),
            this, SLOT(slotDone()));
}

void LdapClient::cancelQuery()
{
    if (d->mJob) {
        d->mJob->kill();
        d->mJob = Q_NULLPTR;
    }

    d->mActive = false;
}

void LdapClient::Private::slotData(KIO::Job *, const QByteArray &data)
{
    parseLDIF(data);
}

void LdapClient::Private::slotData(const QByteArray &data)
{
    parseLDIF(data);
}

void LdapClient::Private::slotInfoMessage(KJob *, const QString &, const QString &)
{
    //qDebug("Job said \"%s\"", info.toLatin1());
}

void LdapClient::Private::slotDone()
{
    endParseLDIF();
    mActive = false;
    if (!mJob) {
        return;
    }
    int err = mJob->error();
    if (err && err != KIO::ERR_USER_CANCELED) {
        Q_EMIT q->error(mJob->errorString());
    }
    Q_EMIT q->done();
}

void LdapClient::Private::startParseLDIF()
{
    mCurrentObject.clear();
    mLdif.startParsing();
}

void LdapClient::Private::endParseLDIF()
{
}

void LdapClient::Private::finishCurrentObject()
{
    mCurrentObject.setDn(mLdif.dn());
    KLDAP::LdapAttrValue objectclasses;
    KLDAP::LdapAttrMap::ConstIterator end = mCurrentObject.attributes().constEnd();
    for (KLDAP::LdapAttrMap::ConstIterator it = mCurrentObject.attributes().constBegin();
            it != end; ++it) {

        if (it.key().toLower() == QLatin1String("objectclass")) {
            objectclasses = it.value();
            break;
        }
    }

    bool groupofnames = false;
    KLDAP::LdapAttrValue::ConstIterator endValue(objectclasses.constEnd());
    for (KLDAP::LdapAttrValue::ConstIterator it = objectclasses.constBegin();
            it != endValue; ++it) {

        const QByteArray sClass = (*it).toLower();
        if (sClass == "groupofnames" || sClass == "kolabgroupofnames") {
            groupofnames = true;
        }
    }

    if (groupofnames) {
        KLDAP::LdapAttrMap::ConstIterator it = mCurrentObject.attributes().find(QStringLiteral("mail"));
        if (it == mCurrentObject.attributes().end()) {
            // No explicit mail address found so far?
            // Fine, then we use the address stored in the DN.
            QString sMail;
            const QStringList lMail = mCurrentObject.dn().toString().split(QStringLiteral(",dc="), QString::SkipEmptyParts);
            const int n = lMail.count();
            if (n) {
                if (lMail.first().toLower().startsWith(QStringLiteral("cn="))) {
                    sMail = lMail.first().simplified().mid(3);
                    if (1 < n) {
                        sMail.append(QLatin1Char('@'));
                    }
                    for (int i = 1; i < n; ++i) {
                        sMail.append(lMail.at(i));
                        if (i < n - 1) {
                            sMail.append(QLatin1Char('.'));
                        }
                    }
                    mCurrentObject.addValue(QStringLiteral("mail"), sMail.toUtf8());
                }
            }
        }
    }
    Q_EMIT q->result(*q, mCurrentObject);
    mCurrentObject.clear();
}

void LdapClient::Private::parseLDIF(const QByteArray &data)
{
    //qCDebug(LDAPCLIENT_LOG) <<"LdapClient::parseLDIF(" << QCString(data.data(), data.size()+1) <<" )";
    if (data.size()) {
        mLdif.setLdif(data);
    } else {
        mLdif.endLdif();
    }
    KLDAP::Ldif::ParseValue ret;
    QString name;
    do {
        ret = mLdif.nextItem();
        switch (ret) {
        case KLDAP::Ldif::Item: {
            name = mLdif.attr();
            const QByteArray value = mLdif.value();
            mCurrentObject.addValue(name, value);
        }
        break;
        case KLDAP::Ldif::EndEntry:
            finishCurrentObject();
            break;
        default:
            break;
        }
    } while (ret != KLDAP::Ldif::MoreData);
}

int LdapClient::clientNumber() const
{
    return d->mClientNumber;
}

int LdapClient::completionWeight() const
{
    return d->mCompletionWeight;
}

void LdapClient::setCompletionWeight(int weight)
{
    d->mCompletionWeight = weight;
}

#include "moc_ldapclient.cpp"
