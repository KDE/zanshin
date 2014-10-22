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

#include "addhostdialog_p.h"

#include <QHBoxLayout>

#include <kacceleratormanager.h>
#include <kldap/ldapserver.h>
#include <kldap/ldapconfigwidget.h>
#include <klocale.h>

AddHostDialog::AddHostDialog( KLDAP::LdapServer *server, QWidget *parent )
    : KDialog( parent )
{
    setCaption( i18n( "Add Host" ) );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );
    setModal( true );
    showButtonSeparator( true );

    mServer = server;

    QWidget *page = new QWidget( this );
    setMainWidget( page );
    QHBoxLayout *layout = new QHBoxLayout( page );
    layout->setSpacing( spacingHint() );
    layout->setMargin( 0 );

    mCfg = new KLDAP::LdapConfigWidget(
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
                KLDAP::LdapConfigWidget::W_SECBOX |
                KLDAP::LdapConfigWidget::W_AUTHBOX,
                page );

    layout->addWidget( mCfg );
    mCfg->setHost( mServer->host() );
    mCfg->setPort( mServer->port() );
    mCfg->setDn( mServer->baseDn() );
    mCfg->setUser( mServer->user() );
    mCfg->setBindDn( mServer->bindDn() );
    mCfg->setPassword( mServer->password() );
    mCfg->setTimeLimit( mServer->timeLimit() );
    mCfg->setSizeLimit( mServer->sizeLimit() );
    mCfg->setPageSize( mServer->pageSize() );
    mCfg->setVersion( mServer->version() );

    switch ( mServer->security() ) {
    case KLDAP::LdapServer::TLS:
        mCfg->setSecurity( KLDAP::LdapConfigWidget::TLS );
        break;
    case KLDAP::LdapServer::SSL:
        mCfg->setSecurity( KLDAP::LdapConfigWidget::SSL );
        break;
    default:
        mCfg->setSecurity( KLDAP::LdapConfigWidget::None );
    }

    switch ( mServer->auth() ) {
    case KLDAP::LdapServer::Simple:
        mCfg->setAuth( KLDAP::LdapConfigWidget::Simple );
        break;
    case KLDAP::LdapServer::SASL:
        mCfg->setAuth( KLDAP::LdapConfigWidget::SASL );
        break;
    default:
        mCfg->setAuth( KLDAP::LdapConfigWidget::Anonymous );
    }
    mCfg->setMech( mServer->mech() );

    KAcceleratorManager::manage( this );
    connect(mCfg, SIGNAL(hostNameChanged(QString)), this, SLOT(slotHostEditChanged(QString)));
    connect( this, SIGNAL(okClicked()), SLOT(slotOk()) );
    enableButtonOk(!mServer->host().isEmpty());
}

AddHostDialog::~AddHostDialog()
{
}

void AddHostDialog::slotHostEditChanged( const QString &text )
{
    enableButtonOk( !text.isEmpty() );
}

void AddHostDialog::slotOk()
{
    mServer->setHost( mCfg->host() );
    mServer->setPort( mCfg->port() );
    mServer->setBaseDn( mCfg->dn() );
    mServer->setUser( mCfg->user() );
    mServer->setBindDn( mCfg->bindDn() );
    mServer->setPassword( mCfg->password() );
    mServer->setTimeLimit( mCfg->timeLimit() );
    mServer->setSizeLimit( mCfg->sizeLimit() );
    mServer->setPageSize( mCfg->pageSize() );
    mServer->setVersion( mCfg->version() );
    switch ( mCfg->security() ) {
    case KLDAP::LdapConfigWidget::TLS:
        mServer->setSecurity( KLDAP::LdapServer::TLS );
        break;
    case KLDAP::LdapConfigWidget::SSL:
        mServer->setSecurity( KLDAP::LdapServer::SSL );
        break;
    default:
        mServer->setSecurity( KLDAP::LdapServer::None );
    }
    switch ( mCfg->auth() ) {
    case KLDAP::LdapConfigWidget::Simple:
        mServer->setAuth( KLDAP::LdapServer::Simple );
        break;
    case KLDAP::LdapConfigWidget::SASL:
        mServer->setAuth( KLDAP::LdapServer::SASL );
        break;
    default:
        mServer->setAuth( KLDAP::LdapServer::Anonymous );
    }
    mServer->setMech( mCfg->mech() );
    KDialog::accept();
}

#include "moc_addhostdialog_p.cpp"
