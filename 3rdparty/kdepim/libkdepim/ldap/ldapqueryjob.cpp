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

#include "ldapqueryjob.h"

#include <kldap/ldif.h>
#include <kldap/ldapcontrol.h>
#include <kldap/ldapdefs.h>
#include <kldap/ldapconnection.h>
#include <kldap/ldapdn.h>

#include <KDebug>

using namespace KLDAP;

LdapQueryJob::LdapQueryJob( const LdapUrl& url, LdapSession* session ) :
  KJob( 0 ), // to not break moveToThread
  m_url( url ),
  m_session( session )
{
  setAutoDelete( false ); // auto-deletion is bad in combination with cross-thread queued connections
}

void LdapQueryJob::triggerStart()
{
  start();
}

void LdapQueryJob::start()
{
  kDebug();
  m_op.setConnection( m_session->connection() );

  LdapControls serverctrls, clientctrls;
  if ( m_session->server().pageSize() ) {
    LdapControls ctrls = serverctrls;
    ctrls.append( LdapControl::createPageControl( m_session->server().pageSize() ) );
    kDebug() << "page size: " << m_session->server().pageSize();
    m_op.setServerControls( ctrls );
  } else {
    m_op.setServerControls( serverctrls );
  }
  m_op.setClientControls( clientctrls );

  int ret, id;
  if ( (id = m_op.search( m_url.dn(), m_url.scope(), m_url.filter(), m_url.attributes() )) == -1 ) {
//     LDAPErr();
    return;
  }

  QByteArray result;
  while( true ) {
    ret = m_op.waitForResult( id, -1 );
    if ( ret == -1 || m_session->connection().ldapErrorCode() != KLDAP_SUCCESS ) {
//       LDAPErr();
      return;
    }
    kDebug() << " ldap_result: " << ret;
    if ( ret == LdapOperation::RES_SEARCH_RESULT ) {

      if ( m_session->server().pageSize() ) {
        QByteArray cookie;
        int estsize = -1;
        for ( int i = 0; i < m_op.controls().count(); ++i ) {
          kDebug() << " control oid: " << m_op.controls()[i].oid();
          estsize = m_op.controls()[i].parsePageControl( cookie );
          if ( estsize != -1 ) break;
        }
        kDebug() << " estimated size: " << estsize;
        if ( estsize != -1 && !cookie.isEmpty() ) {
          LdapControls ctrls;
          ctrls = serverctrls;
          kDebug() << "page size: " << m_session->server().pageSize() << " estimated size: " << estsize;
          ctrls.append( LdapControl::createPageControl( m_session->server().pageSize(), cookie ) );
          m_op.setServerControls( ctrls );
          if ( (id = m_op.search( m_url.dn(), m_url.scope(), m_url.filter(), m_url.attributes() )) == -1 ) {
//             LDAPErr();
            return;
          }
          continue;
        }
      }
      break;
    }
    if ( ret != LdapOperation::RES_SEARCH_ENTRY ) continue;

    QByteArray entry = m_op.object().toString().toUtf8() + '\n';
    emit data(entry);
  }

  emit data( QByteArray() );
  emitResult();
}


