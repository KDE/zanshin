/* This file is part of Zanshin Todo.

   Copyright 2011 Christian Mollekopf <chrigi_1@fastmail.fm>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.
*/

#ifndef TOPICSMODEL_H
#define TOPICSMODEL_H
#include "todoproxymodelbase.h"
#include "nepomukadapter.h"
#include <akonadi/item.h>

namespace Akonadi
{
class Item;
}

class TopicsModel : public TodoProxyModelBase
{
     Q_OBJECT

public:
    enum Roles {
        Begin = Qt::UserRole+1000,
        TopicRole
    };
    TopicsModel(StructureAdapter *adapter, QObject *parent = 0);
    virtual ~TopicsModel();

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    //virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    virtual QStringList mimeTypes() const;
    virtual Qt::DropActions supportedDropActions() const;

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

private slots:
    void onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end);
    void onSourceInsertRows(const QModelIndex &sourceIndex, int begin, int end);
    void onSourceRemoveRows(const QModelIndex &sourceIndex, int begin, int end);
    //void onTopicAdded();
    //void onTopicRemoved();
    
    void createNode(const QString &identifier, const QString &parentIdentifier, const QString &name);
    void removeNode(const QString &identifier);
    //void renameNode(const QString &oldCategoryPath, const QString &newCategoryPath);
    //void moveNode(const QString &oldCategoryPath, const QString &newCategoryPath);
    
   
    void itemsWithTopicAdded(const QString &, const QModelIndexList &);
    void itemsFromTopicRemoved(const QString &, const QModelIndexList &);

    void propertyChanged(const QString &identifier, const QString &parentIdentifier, const QString &name);

private:
    virtual void init();
    virtual TodoNode *createInbox() const;

    TodoNode *m_rootNode;
    QMap<QUrl, TodoNode*> m_resourceMap;

    QHash<Akonadi::Item::Id, QList <QUrl> > m_itemTopics;
    StructureAdapter *m_nepomukAdapter;
    
};

#endif // TOPICSMODEL_H
