/* -*- c++ -*-
 * completionordereditor.h
 *
 *  Copyright (c) 2004 David Faure <faure@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */

#ifndef KDEPIM_COMPLETIONORDEREDITOR_H
#define KDEPIM_COMPLETIONORDEREDITOR_H

#include "kdepim_export.h"

#include <KDE/KConfig>
#include <KDE/KDialog>

class KPushButton;
class QAbstractItemModel;
class QModelIndex;
class QTreeWidget;

namespace KLDAP {
class LdapClientSearch;
}

namespace KPIM {

class CompletionOrderEditor;

// Base class for items in the list
class CompletionItem
{
public:
    virtual ~CompletionItem() {}
    virtual QString label() const = 0;
    virtual QIcon icon() const = 0;
    virtual int completionWeight() const = 0;
    virtual void setCompletionWeight( int weight ) = 0;
    virtual void save( CompletionOrderEditor* ) = 0;
};


class KDEPIM_EXPORT CompletionOrderEditor : public KDialog
{
    Q_OBJECT

public:
    CompletionOrderEditor( KLDAP::LdapClientSearch* ldapSearch, QWidget* parent );
    ~CompletionOrderEditor();

    KConfig* configFile() { return &mConfig; }

Q_SIGNALS:
    void completionOrderChanged();

private Q_SLOTS:
    void rowsInserted( const QModelIndex &parent, int start, int end );
    void slotSelectionChanged();
    void slotMoveUp();
    void slotMoveDown();
    void slotOk();

private:
    void readConfig();
    void writeConfig();
    void loadCompletionItems();
    void addCompletionItemForIndex( const QModelIndex& );

    KConfig mConfig;
    QTreeWidget* mListView;
    KPushButton* mUpButton;
    KPushButton* mDownButton;
    QAbstractItemModel *mCollectionModel;
    KLDAP::LdapClientSearch *mLdapSearch;

    bool mDirty;
};

} // namespace

#endif /* COMPLETIONORDEREDITOR_H */

