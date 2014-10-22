/* kldapclient.cpp - LDAP access
 *      Copyright (C) 2002 Klarälvdalens Datakonsult AB
 *
 *      Author: Steffen Hansen <hansen@kde.org>
 *
 *      Ported to KABC by Daniel Molkentin <molkentin@kde.org>
 *
 * Copyright (C) 2013 Laurent Montel <montel@kde.org>
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

#include "ldapclientsearch.h"
#include "ldapclientsearchconfig.h"

#include "ldapclient.h"
#include "ldapsession.h"
#include "ldapqueryjob.h"

#include <kldap/ldapserver.h>
#include <kldap/ldapurl.h>
#include <kldap/ldif.h>

#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KDirWatch>
#include <KProtocolInfo>
#include <KStandardDirs>
#include <kio/job.h>

#include <QtCore/QPointer>
#include <QtCore/QTimer>


using namespace KLDAP;

class LdapClientSearch::Private
{
public:
    Private( LdapClientSearch *qq )
        : q( qq ),
          mActiveClients( 0 ),
          mNoLDAPLookup( false )
    {
        mClientSearchConfig = new LdapClientSearchConfig;
    }

    ~Private()
    {
        delete mClientSearchConfig;
    }

    void readWeighForClient( LdapClient *client, const KConfigGroup &config, int clientNumber );
    void readConfig();
    void finish();
    void makeSearchData( QStringList &ret, LdapResult::List &resList );

    void slotLDAPResult( const KLDAP::LdapClient &client, const KLDAP::LdapObject& );
    void slotLDAPError( const QString& );
    void slotLDAPDone();
    void slotDataTimer();
    void slotFileChanged( const QString& );

    LdapClientSearch *q;
    QList<LdapClient*> mClients;
    QStringList mAttributes;
    QString mSearchText;
    QString mFilter;
    QTimer mDataTimer;
    int mActiveClients;
    bool mNoLDAPLookup;
    QList<LdapResultObject> mResults;
    QString mConfigFile;
    LdapClientSearchConfig *mClientSearchConfig;
};

LdapClientSearch::LdapClientSearch( QObject *parent )
    : QObject( parent ), d( new Private( this ) )
{
    if ( !KProtocolInfo::isKnownProtocol( KUrl( "ldap://localhost" ) ) ) {
        d->mNoLDAPLookup = true;
        return;
    }


    d->mAttributes << QLatin1String("cn")
                   << QLatin1String("mail")
                   << QLatin1String("givenname")
                   << QLatin1String("sn");

    // Set the filter, to make sure old usage (before 4.14) of this object still works.
    d->mFilter = QString::fromLatin1("&(|(objectclass=person)(objectclass=groupOfNames)(mail=*))"
                                "(|(cn=%1*)(mail=%1*)(givenName=%1*)(sn=%1*))");

    d->readConfig();
    connect( KDirWatch::self(), SIGNAL(dirty(QString)), this,
             SLOT(slotFileChanged(QString)) );

}

LdapClientSearch::~LdapClientSearch()
{
    delete d;
}

void LdapClientSearch::Private::readWeighForClient( LdapClient *client, const KConfigGroup &config,
                                                    int clientNumber )
{
    const int completionWeight = config.readEntry( QString::fromLatin1( "SelectedCompletionWeight%1" ).arg( clientNumber ), -1 );
    if ( completionWeight != -1 ) {
        client->setCompletionWeight( completionWeight );
    }
}

void LdapClientSearch::updateCompletionWeights()
{
    KConfigGroup config( KLDAP::LdapClientSearchConfig::config(), "LDAP" );
    for ( int i = 0; i < d->mClients.size(); ++i ) {
        d->readWeighForClient( d->mClients[ i ], config, i );
    }
}

QList<LdapClient*> LdapClientSearch::clients() const
{
    return d->mClients;
}

QString LdapClientSearch::filter() const
{
    return d->mFilter;
}

void LdapClientSearch::setFilter(const QString &filter)
{
    d->mFilter = filter;
}

QStringList LdapClientSearch::attributes() const
{
    return d->mAttributes;
}

void LdapClientSearch::setAttributes(const QStringList &attrs)
{

    if ( attrs != d->mAttributes ) {
        d->mAttributes = attrs;
        d->readConfig();
    }
}


void LdapClientSearch::Private::readConfig()
{
    q->cancelSearch();
    qDeleteAll( mClients );
    mClients.clear();

    // stolen from KAddressBook
    KConfigGroup config( KLDAP::LdapClientSearchConfig::config(), "LDAP" );
    const int numHosts = config.readEntry( "NumSelectedHosts", 0 );
    if ( !numHosts ) {
        mNoLDAPLookup = true;
    } else {
        for ( int j = 0; j < numHosts; ++j ) {
            LdapClient *ldapClient = new LdapClient( j, q );
            KLDAP::LdapServer server;
            mClientSearchConfig->readConfig( server, config, j, true );
            if ( !server.host().isEmpty() ) {
                mNoLDAPLookup = false;
            }
            ldapClient->setServer( server );

            readWeighForClient( ldapClient, config, j );

            ldapClient->setAttributes( mAttributes );

            q->connect( ldapClient, SIGNAL(result(KLDAP::LdapClient,KLDAP::LdapObject)),
                        q, SLOT(slotLDAPResult(KLDAP::LdapClient,KLDAP::LdapObject)) );
            q->connect( ldapClient, SIGNAL(done()),
                        q, SLOT(slotLDAPDone()) );
            q->connect( ldapClient, SIGNAL(error(QString)),
                        q, SLOT(slotLDAPError(QString)) );

            mClients.append( ldapClient );
        }

        q->connect( &mDataTimer, SIGNAL(timeout()), SLOT(slotDataTimer()) );
    }
    mConfigFile = KStandardDirs::locateLocal( "config", QLatin1String("kabldaprc") );
    KDirWatch::self()->addFile( mConfigFile );
}

void LdapClientSearch::Private::slotFileChanged( const QString &file )
{
    if ( file == mConfigFile ) {
        readConfig();
    }
}

void LdapClientSearch::startSearch( const QString &txt )
{
    if ( d->mNoLDAPLookup ) {
        return;
    }

    cancelSearch();

    int pos = txt.indexOf( QLatin1Char('\"') );
    if ( pos >= 0 ) {
        ++pos;
        const int pos2 = txt.indexOf( QLatin1Char('\"'), pos );
        if ( pos2 >= 0 ) {
            d->mSearchText = txt.mid( pos, pos2 - pos );
        } else {
            d->mSearchText = txt.mid( pos );
        }
    } else {
        d->mSearchText = txt;
    }

    const QString filter = d->mFilter.arg( d->mSearchText );

    QList<LdapClient*>::Iterator it;
    QList<LdapClient*>::Iterator end(d->mClients.end());
    for ( it = d->mClients.begin(); it != end; ++it ) {
        (*it)->startQuery( filter );
        kDebug(5300) <<"LdapClientSearch::startSearch()" << filter;
        ++d->mActiveClients;
    }
}

void LdapClientSearch::cancelSearch()
{
    QList<LdapClient*>::Iterator it;
    QList<LdapClient*>::Iterator end(d->mClients.end());
    for ( it = d->mClients.begin(); it != end; ++it ) {
        (*it)->cancelQuery();
    }

    d->mActiveClients = 0;
    d->mResults.clear();
}

void LdapClientSearch::Private::slotLDAPResult( const LdapClient &client,
                                                const KLDAP::LdapObject &obj )
{
    LdapResultObject result;
    result.client = &client;
    result.object = obj;

    mResults.append( result );
    if ( !mDataTimer.isActive() ) {
        mDataTimer.setSingleShot( true );
        mDataTimer.start( 500 );
    }
}

void LdapClientSearch::Private::slotLDAPError( const QString& )
{
    slotLDAPDone();
}

void LdapClientSearch::Private::slotLDAPDone()
{
    if ( --mActiveClients > 0 ) {
        return;
    }

    finish();
}

void LdapClientSearch::Private::slotDataTimer()
{
    QStringList lst;
    LdapResult::List reslist;

    emit q->searchData(mResults);

    makeSearchData( lst, reslist );
    if ( !lst.isEmpty() ) {
        emit q->searchData( lst );
    }
    if ( !reslist.isEmpty() ) {
        emit q->searchData( reslist );
    }
}

void LdapClientSearch::Private::finish()
{
    mDataTimer.stop();

    slotDataTimer(); // emit final bunch of data
    emit q->searchDone();
}

void LdapClientSearch::Private::makeSearchData( QStringList &ret, LdapResult::List &resList )
{

    QList< LdapResultObject >::ConstIterator it1;
    QList< LdapResultObject >::ConstIterator end1(mResults.constEnd());
    for ( it1 = mResults.constBegin(); it1 != end1; ++it1 ) {
        QString name, mail, givenname, sn;
        QStringList mails;
        bool isDistributionList = false;
        bool wasCN = false;
        bool wasDC = false;

        //kDebug(5300) <<"\n\nLdapClientSearch::makeSearchData()";

        KLDAP::LdapAttrMap::ConstIterator it2;
        for ( it2 = (*it1).object.attributes().constBegin();
              it2 != (*it1).object.attributes().constEnd(); ++it2 ) {
            QByteArray val = (*it2).first();
            int len = val.size();
            if ( len > 0 && '\0' == val[len-1] ) {
                --len;
            }
            const QString tmp = QString::fromUtf8( val, len );
            //kDebug(5300) <<"      key: \"" << it2.key() <<"\" value: \"" << tmp <<"\"";
            if ( it2.key() == QLatin1String("cn") ) {
                name = tmp;
                if ( mail.isEmpty() ) {
                    mail = tmp;
                } else {
                    if ( wasCN ) {
                        mail.prepend( QLatin1String(".") );
                    } else {
                        mail.prepend( QLatin1String("@") );
                    }
                    mail.prepend( tmp );
                }
                wasCN = true;
            } else if ( it2.key() == QLatin1String("dc") ) {
                if ( mail.isEmpty() ) {
                    mail = tmp;
                } else {
                    if ( wasDC ) {
                        mail.append( QLatin1String(".") );
                    } else {
                        mail.append( QLatin1String("@") );
                    }
                    mail.append( tmp );
                }
                wasDC = true;
            } else if ( it2.key() == QLatin1String("mail") ) {
                mail = tmp;
                KLDAP::LdapAttrValue::ConstIterator it3 = it2.value().constBegin();
                for ( ; it3 != it2.value().constEnd(); ++it3 ) {
                    mails.append( QString::fromUtf8( (*it3).data(), (*it3).size() ) );
                }
            } else if ( it2.key() == QLatin1String("givenName") ) {
                givenname = tmp;
            } else if ( it2.key() == QLatin1String("sn") ) {
                sn = tmp;
            } else if ( it2.key() == QLatin1String("objectClass") &&
                        (tmp == QLatin1String("groupOfNames") || tmp == QLatin1String("kolabGroupOfNames")) ) {
                isDistributionList = true;
            }
        }

        if ( mails.isEmpty() ) {
            if ( !mail.isEmpty() ) {
                mails.append( mail );
            }
            if ( isDistributionList ) {
                //kDebug(5300) <<"\n\nLdapClientSearch::makeSearchData() found a list:" << name;
                ret.append( name );
                // following lines commented out for bugfixing kolab issue #177:
                //
                // Unlike we thought previously we may NOT append the server name here.
                //
                // The right server is found by the SMTP server instead: Kolab users
                // must use the correct SMTP server, by definition.
                //
                //mail = (*it1).client->base().simplified();
                //mail.replace( ",dc=", ".", false );
                //if( mail.startsWith("dc=", false) )
                //  mail.remove(0, 3);
                //mail.prepend( '@' );
                //mail.prepend( name );
                //mail = name;
            } else {
                continue; // nothing, bad entry
            }
        } else if ( name.isEmpty() ) {
            ret.append( mail );
        } else {
            ret.append( QString::fromLatin1( "%1 <%2>" ).arg( name ).arg( mail ) );
        }

        LdapResult sr;
        sr.dn = (*it1).object.dn();
        sr.clientNumber = (*it1).client->clientNumber();
        sr.completionWeight = (*it1).client->completionWeight();
        sr.name = name;
        sr.email = mails;
        resList.append( sr );
    }

    mResults.clear();
}

bool LdapClientSearch::isAvailable() const
{
    return !d->mNoLDAPLookup;
}

#include "moc_ldapclientsearch.cpp"
