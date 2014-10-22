/* kldapclient.h - LDAP access
 *      Copyright (C) 2002 Klarälvdalens Datakonsult AB
 *
 *      Author: Steffen Hansen <hansen@kde.org>
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

#ifndef LDAPCLIENTSEARCH_H
#define LDAPCLIENTSEARCH_H

#include "libkdepim/kdepim_export.h"
#include <kldap/ldapobject.h>
#include <QtCore/QObject>
#include <QtCore/QStringList>



namespace KLDAP {
class LdapClient;


/**
 * Describes the result returned by an LdapClientSearch query.
 *
 * @since 4.14
 */
struct LdapResultObject {
    const LdapClient *client;
    KLDAP::LdapObject object;
};

/**
 * Describes the result returned by an LdapClientSearch query.
 *
 * @since 4.5
 */
struct LdapResult
{
    /**
   * A list of LdapResult objects.
   */
    typedef QList<LdapResult> List;

    LdapDN dn;
    QString name;         ///< The full name of the contact.
    QStringList email;    ///< The list of emails of the contact.
    int clientNumber;     ///< The client the contact comes from (used for sorting in a ldap-only lookup).
    int completionWeight; ///< The weight of the contact (used for sorting in a completion list).
};

/**
 * @since 4.5
 */
class KDEPIM_EXPORT LdapClientSearch : public QObject
{
    Q_OBJECT

public:
    /**
     * Creates a new ldap client search object.
     *
     * @param parent The parent object.
     */
    explicit LdapClientSearch( QObject *parent = 0 );

    /**
     * Destroys the ldap client search object.
     */
    ~LdapClientSearch();

    /**
     * Starts the LDAP search on all configured LDAP clients with the given search @p query.
     */
    void startSearch( const QString &query );

    /**
     * Cancels the currently running search query.
     */
    void cancelSearch();

    /**
     * Returns whether LDAP search is possible at all.
     *
     * @note This method can return @c false if either no LDAP is configured
     *       or the system does not support the KIO LDAP protocol.
     */
    bool isAvailable() const;

    /**
     * Updates the completion weights for the configured LDAP clients from
     * the configuration file.
     */
    void updateCompletionWeights();

    /**
     * Returns the list of configured LDAP clients.
     */
    QList<LdapClient*> clients() const;


    /**
     * Returns the filter for the Query
     *
     * @since 4.14
     */
    QString filter() const;

    /**
     * Sets the filter for the Query
     *
     * @since 4.14
     */
    void setFilter(const QString &);

    /**
     * Returns the attributes, that are queried the LDAP Server.
     *
     * @since 4.14
     */
    QStringList attributes() const;

     /**
     * Sets the attributes, that are queried the LDAP Server.
     *
     * @since 4.14
     */
    void setAttributes(const QStringList&);

Q_SIGNALS:
    /**
     * This signal is emitted whenever new contacts have been found
     * during the lookup.
     *
     * @param results The contacts in the form "Full Name <email>"
     */
    void searchData( const QStringList &results );

    /**
     * This signal is emitted whenever new contacts have been found
     * during the lookup.
     *
     * @param results The list of found contacts.
     */
    void searchData( const KLDAP::LdapResult::List &results );

    /**
     * This signal is emitted whenever new contacts have been found
     * during the lookup.
     *
     * @param results The list of found contacts.
     */
    void searchData( const QList<KLDAP::LdapResultObject> &results );

    /**
     * This signal is emitted whenever the lookup is complete or the
     * user has canceled the query.
     */
    void searchDone();

private:
    //@cond PRIVATE
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void slotLDAPResult( const KLDAP::LdapClient&, const KLDAP::LdapObject& ) )
    Q_PRIVATE_SLOT( d, void slotLDAPError( const QString& ) )
    Q_PRIVATE_SLOT( d, void slotLDAPDone() )
    Q_PRIVATE_SLOT( d, void slotDataTimer() )
    Q_PRIVATE_SLOT( d, void slotFileChanged( const QString& ) )
    //@endcond
};

}

#endif // LDAPCLIENTSEARCH_H
