/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    Author: Volker Krause <volker.krause@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef KLDAP_LDAPSESSION_H
#define KLDAP_LDAPSESSION_H

#include <QThread>
#include <kldap/ldapconnection.h>
#include <QtCore/QQueue>
#include <QtCore/QMutex>

class KJob;

namespace KLDAP {

class LdapServer;
class LdapQueryJob;


class LdapSession : public QThread
{
  Q_OBJECT
  public:
    explicit LdapSession( QObject * parent = 0 );

    /// call this instead of start()
    void connectToServer( const LdapServer &server );
    /// call this instead of the dtor
    void disconnectAndDelete();

    LdapQueryJob* get( const LdapUrl &url );

    LdapServer server() const;
    LdapConnection& connection();

  protected:
    void connectToServerInternal();
    void disconnectFromServerInternal();
    void run();

  private:
    void authenticate();

  private slots:
    void executeNext();
    void jobDone( KJob* job );

  private:
    enum State {
      Disconnected,
      Connected,
      Authenticated
    };
    State m_state;
    LdapConnection m_conn;
    LdapServer m_server;
    QMutex m_mutex;
    QQueue<LdapQueryJob*> m_jobQueue;
    LdapQueryJob* m_currentJob;
};

}

#endif
