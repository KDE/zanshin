/*
    This file is part of Zanshin Todo.

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


#ifndef PROJECTSTRATEGY_H
#define PROJECTSTRATEGY_H

#include "reparentingmodel/reparentingstrategy.h"
#include "core/pimitemstructurecache.h"

class ProjectStrategy : public QObject, public ReparentingStrategy
{
    Q_OBJECT
public:
    ProjectStrategy(ProjectStructureCache *structure = new ProjectStructureCache());
    virtual void init();
    virtual Id getId(const QModelIndex& );
    virtual IdList getParents(const QModelIndex&, const IdList &ignore = IdList());
    virtual void reset();
    
    virtual void onNodeRemoval(const Id& changed);

    virtual Qt::ItemFlags flags(const QModelIndex& index, Qt::ItemFlags flags);
    virtual Qt::DropActions supportedDropActions() const;
    virtual bool onDropMimeData(Id id, const QMimeData* , Qt::DropAction );

    virtual QVariant data(Id id, int column, int role, bool &forward) const;
private slots:
    void doRemoveNode(Id id);
    void doChangeParents(Id, IdList);
private:
    bool isProject(Id id, Zanshin::ItemType itemType) const;
    void checkParents(const IdList &);
//     QHash<QString, Id> mUidMapping;
//     QHash<Akonadi::Collection::Id, Id> mCollectionMapping;
    const Id mInbox;
    QScopedPointer<ProjectStructureCache> mRelations;
};

#endif // PROJECTSTRATEGY_H
