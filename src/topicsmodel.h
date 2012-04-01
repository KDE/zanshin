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

class TopicsModel : public TodoProxyModelBase
{
     Q_OBJECT
     friend class StructureAdapter;
     friend class NepomukAdapter;
     friend class TestStructureAdapter;
     
     typedef qint64 Id;
     typedef QList<qint64> IdList;
public:
//     enum Roles {
//         Begin = Qt::UserRole+1000,
//         ParentRole
//     };
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

private:
    virtual void init();
    virtual TodoNode *createInbox() const;
    
    /**
     * Creates a new parent
     * 
     * @param identifier must be unique
     * 
     * The 
     */
    void createOrRenameParent(const Id &identifier, const Id &parentIdentifier, const QString &name);
    /**
     * Updates the parents of @param item.
     * 
     * Moves/adds/removes from parents.
     */
    void itemParentsChanged(const QModelIndex &item, const IdList &parents);
    /**
     * Renames @param parent
     */
    void renameParent(const Id &id, const QString &name);
    
    void createNode(const Id &identifier, const Id &parentIdentifier, const QString &name);
    void removeNode(const Id &identifier);
    //void renameNode(const QString &oldCategoryPath, const QString &newCategoryPath);
    //void moveNode(const QString &oldCategoryPath, const QString &newCategoryPath);
    
    
    
    
    
//     void propertyChanged(const Id &identifier, const Id &parentIdentifier, const QString &name);

    TodoNode *m_rootNode;
    QMap<Id, TodoNode*> m_resourceMap;

    StructureAdapter *m_nepomukAdapter;
    
};

#endif // TOPICSMODEL_H
