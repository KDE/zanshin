/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012  Christian Mollekopf <chrigi_1@fastmail.fm>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef CATEGORIESSTRATEGY_H
#define CATEGORIESSTRATEGY_H
#include "reparentingstrategy.h"
#include "pimitemrelations.h"

class CategoriesStrategy : public QObject, public ReparentingStrategy
{
    Q_OBJECT
public:
    CategoriesStrategy();
    virtual void init();
    virtual Id getId(const QModelIndex& );
    virtual IdList getParents(const QModelIndex&, const IdList &ignore = IdList());
    virtual void reset();

    virtual void setData(TodoNode* node, Id id);
    virtual void onNodeRemoval(const Id& changed);


    virtual Qt::ItemFlags flags(const QModelIndex& index, Qt::ItemFlags flags);
    virtual QStringList mimeTypes();
    virtual QMimeData* mimeData(const QModelIndexList& indexes) const;
    virtual Qt::DropActions supportedDropActions() const;
    
    virtual bool onDropMimeData(Id id, const QMimeData* , Qt::DropAction );
    virtual bool onSetData(Id id, const QVariant& value, int role);

    virtual bool reparentOnParentRemoval(Id child) const;
private slots:
    void createVirtualNode(Id id, IdList parents, const QString &name);
//     void createCategoryNode(const QString &categoryPath);
//     void removeCategoryNode(const QString &categoryPath);
//     void renameCategoryNode(const QString &oldCategoryPath, const QString &newCategoryPath);
//     void moveCategoryNode(const QString &oldCategoryPath, const QString &newCategoryPath);
private:
//     QHash<QString, Id> mUidMapping; //sourceitems to id
//     QHash<Akonadi::Collection::Id, Id> mCollectionMapping;
//     QMap<Id, IdList> mNodeCategories; //nodes categories
//     QMap<Id, Id> mNodeParentMap; //categories hierarchy
//     QMap<QString, Id> mCategoryMap; //category string to node id
    const Id mInbox;
    const Id mRoot;
    QScopedPointer<PimItemRelations> mRelations;
};

#endif // CATEGORIESSTRATEGY_H
