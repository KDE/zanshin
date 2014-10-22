/*
  This file is part of libkldap.

  Copyright (c) 2003 - 2009 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2013 Laurent Montel <montel@kde.org>

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

#ifndef KCMLDAP_H
#define KCMLDAP_H

#include <kcmodule.h>

class QListWidget;
class QListWidgetItem;
class QPushButton;
class QToolButton;

namespace KLDAP {
class LdapClientSearchConfig;
}

class KCMLdap : public KCModule
{
  Q_OBJECT

  public:
    explicit KCMLdap( QWidget *parent, const QVariantList &args );
    ~KCMLdap();

    void load();
    void save();
    void defaults();

  private Q_SLOTS:
    void slotAddHost();
    void slotEditHost();
    void slotRemoveHost();
    void slotSelectionChanged( QListWidgetItem* );
    void slotItemClicked( QListWidgetItem* );
    void slotMoveUp();
    void slotMoveDown();

  private:
    void initGUI();
    QWidget* dialogParent();

    QListWidget* mHostListView;

    QPushButton* mAddButton;
    QPushButton* mEditButton;
    QPushButton* mRemoveButton;

    QToolButton* mUpButton;
    QToolButton* mDownButton;
    KLDAP::LdapClientSearchConfig *mClientSearchConfig;
};

#endif
