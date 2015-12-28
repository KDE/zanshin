/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef COMPLETIONORDERWIDGET_H
#define COMPLETIONORDERWIDGET_H

#include <QWidget>
#include "kdepim_export.h"
#include <QDBusAbstractAdaptor>
#include <KConfig>

class QPushButton;
class QAbstractItemModel;
class QModelIndex;
class QTreeWidget;

namespace KLDAP
{
class LdapClientSearch;
}

namespace KPIM
{

class CompletionOrderEditorAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.pim.CompletionOrder")
public:
    explicit CompletionOrderEditorAdaptor(QObject *parent);
Q_SIGNALS:
    void completionOrderChanged();
};

class CompletionOrderWidget;

// Base class for items in the list
class CompletionItem
{
public:
    virtual ~CompletionItem() {}
    virtual QString label() const = 0;
    virtual QIcon icon() const = 0;
    virtual int completionWeight() const = 0;
    virtual void setCompletionWeight(int weight) = 0;
    virtual void save(CompletionOrderWidget *) = 0;
    virtual bool hasEnableSupport() const = 0;
    virtual bool isEnabled() const = 0;
    virtual void setIsEnabled(bool b) = 0;
};

class KDEPIM_EXPORT CompletionOrderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CompletionOrderWidget(QWidget *parent = Q_NULLPTR);
    ~CompletionOrderWidget();
    void save();

    KConfig *configFile();
    void loadCompletionItems();
    void setLdapClientSearch(KLDAP::LdapClientSearch *ldapSearch);

Q_SIGNALS:
    void completionOrderChanged();

private Q_SLOTS:
    void rowsInserted(const QModelIndex &parent, int start, int end);
    void slotSelectionChanged();
    void slotMoveUp();
    void slotMoveDown();

private:
    void readConfig();
    void writeConfig();
    void addRecentAddressItem();
    void addCompletionItemForCollection(const QModelIndex &);

    KConfig mConfig;
    QTreeWidget *mListView;
    QPushButton *mUpButton;
    QPushButton *mDownButton;
    QAbstractItemModel *mCollectionModel;
    KLDAP::LdapClientSearch *mLdapSearch;

    bool mDirty;

};
}

#endif // COMPLETIONORDERWIDGET_H
