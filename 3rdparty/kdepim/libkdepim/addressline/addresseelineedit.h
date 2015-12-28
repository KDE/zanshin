/*
  This file is part of libkdepim.

  Copyright (c) 2002 Helge Deller <deller@gmx.de>
  Copyright (c) 2002 Lubos Lunak <llunak@suse.cz>
  Copyright (c) 2001,2003 Carsten Pfeiffer <pfeiffer@kde.org>
  Copyright (c) 2001 Waldo Bastian <bastian@kde.org>
  Copyright (c) 2004 Daniel Molkentin <danimo@klaralvdalens-datakonsult.se>
  Copyright (c) 2004 Karl-Heinz Zimmer <khz@klaralvdalens-datakonsult.se>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
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

#ifndef KDEPIM_ADDRESSEELINEEDIT_H
#define KDEPIM_ADDRESSEELINEEDIT_H

#include "kdepim_export.h"

#include <KLineEdit>

class QDropEvent;
class QEvent;
class QKeyEvent;
class QMenu;
class QMouseEvent;
class QObject;
class KJob;
class KConfig;
namespace Akonadi
{
class Item;
}

namespace KContacts
{
class Addressee;
class ContactGroup;
}
namespace KLDAP
{
class LdapClientSearch;
}

namespace KPIM
{

class AddresseeLineEditPrivate;
class KDEPIM_EXPORT AddresseeLineEdit : public KLineEdit
{
    Q_OBJECT

public:
    /**
     * Creates a new addressee line edit.
     *
     * @param parent The parent object.
     * @param enableCompletion Whether autocompletion shall be enabled.
     */
    explicit AddresseeLineEdit(QWidget *parent, bool enableCompletion = true);

    /**
     * Destroys the addressee line edit.
     */
    virtual ~AddresseeLineEdit();

    /**
     * Sets whether semicolons are allowed as separators.
     */
    void allowSemicolonAsSeparator(bool allow);

    /**
     * Reimplemented for setting the @p font for line edit and completion box.
     */
    void setFont(const QFont &font);

    void setEnableBalooSearch(bool enable);

    bool isCompletionEnabled() const;

    void setExpandIntern(bool);

    bool expandIntern() const;

    bool groupsIsEmpty() const;
    /**
     * Adds a new @p contact to the completion with a given
     * @p weight
     * @p source index
     * @p append  is added to completion string, but removed, when mail is selected.
     */
    void addContact(const KContacts::Addressee &contact, int weight, int source = -1, QString append = QString());

    /**
     * Same as the above, but this time with contact groups.
     */
    void addContactGroup(const KContacts::ContactGroup &group, int weight, int source = -1);

    void addItem(const Akonadi::Item &item, int weight, int source = -1);

    /**
     * Adds the @p name of a completion source and its @p weight
     * to the internal list of completion sources and returns its index,
     * which can be used for insertion of items associated with that source.
     *
     * If the source already exists, the weight will be updated.
     */
    int addCompletionSource(const QString &name, int weight);

    void removeCompletionSource(const QString &source);
    void emitTextCompleted();

    void callUserCancelled(const QString &str);
    void callSetCompletedText(const QString & /*text*/, bool /*marked*/);
    void callSetCompletedText(const QString &text);
    void callSetUserSelection(bool);

    void updateBalooBlackList();
    void updateCompletionOrder();
    KLDAP::LdapClientSearch *ldapSearch() const;
    QStringList balooBlackList() const;

    void setAutoGroupExpand(bool autoGroupExpand);
    bool autoGroupExpand() const;
    void setShowRecentAddresses(bool b);
    bool showRecentAddresses() const;
    void setRecentAddressConfig(KConfig *config);
    KConfig *recentAddressConfig() const;
    void configureCompletion();

Q_SIGNALS:
    void textCompleted();
    void addAddress(const QString &address);

public Q_SLOTS:
    /**
     * Moves the cursor at the end of the line edit.
     */
    void cursorAtEnd();

    /**
     * Sets whether autocompletion shall be enabled.
     */
    void enableCompletion(bool enable);

    /**
     * Reimplemented for stripping whitespace after completion
     * Danger: This is _not_ virtual in the base class!
     */
    void setText(const QString &text) Q_DECL_OVERRIDE;

    void expandGroups();
    void slotEditingFinished();
    void slotGroupSearchResult(KJob *job);

protected:
    /**
     * Reimplemented for smart insertion of email addresses.
     * Features:
     * - Automatically adds ',' if necessary to separate email addresses
     * - Correctly decodes mailto URLs
     * - Recognizes email addresses which are protected against address
     *   harvesters, i.e. "name at kde dot org" and "name(at)kde.org"
     */
    virtual void insert(const QString &);

    /**
     * Reimplemented for smart insertion of pasted email addresses.
     */
    virtual void paste();

    /**
     * Reimplemented for smart insertion with middle mouse button.
     */
    void mouseReleaseEvent(QMouseEvent *) Q_DECL_OVERRIDE;

#ifndef QT_NO_DRAGANDDROP
    /**
     * Reimplemented for smart insertion of dragged email addresses.
     */
    void dropEvent(QDropEvent *) Q_DECL_OVERRIDE;
#endif

    /**
     * Reimplemented for internal reasons.
     */
    void keyPressEvent(QKeyEvent *) Q_DECL_OVERRIDE;

#ifndef QT_NO_CONTEXTMENU
    /**
     * Reimplemented for subclass access to menu
     */
    virtual QMenu *createStandardContextMenu();

    /**
     * Reimplemented for internal reasons.  API not affected.
     *
     * See QLineEdit::contextMenuEvent().
     */
    void contextMenuEvent(QContextMenuEvent *) Q_DECL_OVERRIDE;
#endif

    QStringList cleanupEmailList(const QStringList &inputList);
    void insertEmails(const QStringList &emails);
    void loadContacts();

private Q_SLOTS:
    void groupExpandResult(KJob *job);
    void slotToggleExpandGroups();
private:
    bool eventFilter(QObject *, QEvent *) Q_DECL_OVERRIDE;

    AddresseeLineEditPrivate *const d;
};

}

#endif
