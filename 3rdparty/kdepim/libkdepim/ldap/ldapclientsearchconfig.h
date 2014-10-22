/*
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

#ifndef LDAPCLIENTSEARCHCONFIG_H
#define LDAPCLIENTSEARCHCONFIG_H

#include "libkdepim/kdepim_export.h"

#include <QObject>

class KConfigGroup;
class KConfig;

namespace KLDAP {
class LdapServer;
class LdapClient;
class KDEPIM_EXPORT LdapClientSearchConfig : public QObject
{
    Q_OBJECT
public:
    explicit LdapClientSearchConfig(QObject *parent = 0);
    ~LdapClientSearchConfig();

    /**
     * Returns the global config object, which stores the LdapClient configurations.
     */
    static KConfig *config();

    /**
     * Reads the LDAP @p server settings from the given config @p group for the
     * given LDAP @p clientNumber.
     *
     * @param active Defines whether the active settings shall be read.
     */
    void readConfig( KLDAP::LdapServer &server, KConfigGroup &group,
                     int clientNumber, bool active );

    /**
     * Writes the LDAP @p server settings to the given config @p group for the
     * given LDAP @p clientNumber.
     *
     * @param active Defines whether the active settings shall be written.
     */
    void writeConfig( const KLDAP::LdapServer &server, KConfigGroup &group,
                      int clientNumber, bool active );
private Q_SLOTS:
    void slotWalletClosed();

private:
    //@cond PRIVATE
    class Private;
    Private* const d;
};

}

#endif // LDAPCLIENTSEARCHCONFIG_H
