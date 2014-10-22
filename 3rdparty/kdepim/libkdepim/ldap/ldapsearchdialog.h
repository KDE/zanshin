/*
 * This file is part of libkldap.
 *
 * Copyright (C) 2002 Klarälvdalens Datakonsult AB
 *
 * Author: Steffen Hansen <hansen@kde.org>
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
//AK_REVIEW: move back to libkdepim, not ready yet
#ifndef KLDAP_LDAPSEARCHDIALOG_H
#define KLDAP_LDAPSEARCHDIALOG_H

#include "libkdepim/kdepim_export.h"

#include <kabc/addressee.h>
#include <kdialog.h>

class QCloseEvent;

namespace KLDAP {

class LdapClient;
class LdapObject;

/**
 * @short A dialog to search contacts in a LDAP directory.
 *
 * This dialog allows the user to search for contacts inside
 * a LDAP directory.
 *
 * @author Steffen Hansen <hansen@kde.org>
 * @since 4.5
 */
class KDEPIM_EXPORT LdapSearchDialog : public KDialog
{
    Q_OBJECT

public:
    /**
     * Creates a new ldap search dialog.
     *
     * @param parent The parent widget.
     */
    explicit LdapSearchDialog( QWidget *parent = 0 );

    /**
     * Destroys the ldap search dialog.
     */
    ~LdapSearchDialog();

    /**
     * Sets the @p text in the search line edit.
     */
    void setSearchText( const QString &text );

    /**
     * Returns a list of contacts that have been selected
     * in the LDAP search.
     */
    KABC::Addressee::List selectedContacts() const;

Q_SIGNALS:
    /**
     * This signal is emitted whenever the user clicked the
     * 'Add Selected' button.
     */
    void contactsAdded();

protected Q_SLOTS:
    void slotUser1();
    void slotUser2();
    void slotCustomContextMenuRequested(const QPoint&);
    void slotCancelClicked();

protected:
    void closeEvent( QCloseEvent* );

private:
    //@cond PRIVATE
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void slotAddResult( const KLDAP::LdapClient&, const KLDAP::LdapObject& ) )
    Q_PRIVATE_SLOT( d, void slotSetScope( bool ) )
    Q_PRIVATE_SLOT( d, void slotStartSearch() )
    Q_PRIVATE_SLOT( d, void slotStopSearch() )
    Q_PRIVATE_SLOT( d, void slotSearchDone() )
    Q_PRIVATE_SLOT( d, void slotError( const QString& ) )
    Q_PRIVATE_SLOT( d, void slotSelectAll() )
    Q_PRIVATE_SLOT( d, void slotUnselectAll() )
    Q_PRIVATE_SLOT( d, void slotSelectionChanged() )
    //@endcond
};

}

#endif
