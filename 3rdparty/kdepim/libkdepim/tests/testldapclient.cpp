/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "testldapclient.h"

#include <qdebug.h>

#include <kldap/ldapobject.h>

#include <QEventLoop>

#include <assert.h>
#include <stdlib.h>
#include <QApplication>
#include <KAboutData>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QStandardPaths::setTestModeEnabled(true);
    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(app);

    TestLDAPClient test;
    test.setup();
    test.runAll();
    test.cleanup();
    qDebug() << "All tests OK.";
    return 0;
}

TestLDAPClient::TestLDAPClient()
    : mClient(Q_NULLPTR)
{
}

void TestLDAPClient::setup()
{
}

void TestLDAPClient::runAll()
{
    testIntevation();
}

bool TestLDAPClient::check(const QString &txt, QString a, QString b)
{
    if (a.isEmpty()) {
        a.clear();
    }

    if (b.isEmpty()) {
        b.clear();
    }

    if (a == b) {
        qDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'..." << "ok";
    } else {
        qDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'..." << "KO !";
        cleanup();
        exit(1);
    }

    return true;
}

void TestLDAPClient::cleanup()
{
    mClient = Q_NULLPTR;
}

void TestLDAPClient::testIntevation()
{
    qDebug();
    mClient = new KLDAP::LdapClient(0, this);

#ifdef __GNUC__
#warning TODO!
#endif
#if 0
    mClient->setHost("ca.intevation.de");
    mClient->setPort("389");
    mClient->setBase("o=Intevation GmbH,c=de");
#endif

    // Same list as in kaddressbook's ldapsearchdialog
    QStringList attrs;
    attrs << QStringLiteral("l") << QStringLiteral("Company") << QStringLiteral("co") << QStringLiteral("department") << QStringLiteral("description") << QStringLiteral("mail")
          << QStringLiteral("facsimileTelephoneNumber") << QStringLiteral("cn") << QStringLiteral("homePhone") << QStringLiteral("mobile") << QStringLiteral("o")
          << QStringLiteral("pager") << QStringLiteral("postalAddress") << QStringLiteral("st") << QStringLiteral("street")
          << QStringLiteral("title") << QStringLiteral("uid") << QStringLiteral("telephoneNumber") << QStringLiteral("postalCode") << QStringLiteral("objectClass");
    // the list from ldapclient.cpp
    //attrs << "cn" << "mail" << "givenname" << "sn" << "objectClass";
    mClient->setAttributes(attrs);

    // Taken from LdapSearch
    /*
      QString mSearchText = QString::fromUtf8( "Till" );
      QString filter = QString( "&(|(objectclass=person)(objectclass=groupOfNames)(mail=*))"
                                "(|(cn=%1*)(mail=%2*)(givenName=%3*)(sn=%4*))" )
                       .arg( mSearchText ).arg( mSearchText ).arg( mSearchText ).arg( mSearchText );
     */

    // For some reason a fromUtf8 broke the search for me (no results).
    // But this certainly looks fishy, it might break on non-utf8 systems.
    QString filter = QStringLiteral("&(|(objectclass=person)(objectclass=groupofnames)(mail=*))"
                                    "(|(cn=*Ägypten MDK*)(sn=*Ägypten MDK*))");

    connect(mClient, &KLDAP::LdapClient::result, this, &TestLDAPClient::slotLDAPResult);
    connect(mClient, &KLDAP::LdapClient::done, this, &TestLDAPClient::slotLDAPDone);
    connect(mClient, &KLDAP::LdapClient::error, this, &TestLDAPClient::slotLDAPError);
    mClient->startQuery(filter);

    QEventLoop eventLoop;
    connect(this, &TestLDAPClient::leaveModality, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    delete mClient;
    mClient = Q_NULLPTR;
}

// from kaddressbook... ugly though...
static QString asUtf8(const QByteArray &val)
{
    if (val.isEmpty()) {
        return QString();
    }

    const char *data = val.data();

    //QString::fromUtf8() bug workaround
    if (data[ val.size() - 1 ] == '\0') {
        return QString::fromUtf8(data, val.size() - 1);
    } else {
        return QString::fromUtf8(data, val.size());
    }
}

static QString join(const KLDAP::LdapAttrValue &lst, const QString &sep)
{
    QString res;
    bool already = false;
    for (KLDAP::LdapAttrValue::ConstIterator it = lst.begin(); it != lst.end(); ++it) {
        if (already) {
            res += sep;
        }

        already = true;
        res += asUtf8(*it);
    }

    return res;
}

void TestLDAPClient::slotLDAPResult(const KLDAP::LdapClient &, const KLDAP::LdapObject &obj)
{
    QString cn = join(obj.attributes()[ QStringLiteral("cn") ], QStringLiteral(", "));
    qDebug() << " cn:" << cn;
    assert(!obj.attributes()[ QStringLiteral("mail") ].isEmpty());
    QString mail = join(obj.attributes()[ QStringLiteral("mail") ], QStringLiteral(", "));
    qDebug() << " mail:" << mail;
    assert(mail.contains(QLatin1Char('@')));
}

void TestLDAPClient::slotLDAPError(const QString &err)
{
    qDebug() << err;
    ::exit(1);
}

void TestLDAPClient::slotLDAPDone()
{
    qDebug();
    Q_EMIT leaveModality();
}

