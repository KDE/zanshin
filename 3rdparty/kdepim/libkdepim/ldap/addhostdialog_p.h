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

#ifndef ADDHOSTDIALOG_H
#define ADDHOSTDIALOG_H

#include <kdialog.h>

namespace KLDAP {
class LdapConfigWidget;
class LdapServer;
}

class AddHostDialog : public KDialog
{
    Q_OBJECT

  public:
    explicit AddHostDialog( KLDAP::LdapServer *server, QWidget *parent = 0 );
    ~AddHostDialog();

  Q_SIGNALS:
    void changed( bool );

  private Q_SLOTS:
    void slotHostEditChanged( const QString& );
    void slotOk();

  private:
    KLDAP::LdapConfigWidget *mCfg;
    KLDAP::LdapServer *mServer;
};

#endif // ADDHOSTDIALOG_H
