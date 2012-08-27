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


#ifndef PROJECTSTRATEGY_H
#define PROJECTSTRATEGY_H

#include "reparentingstrategy.h"
#include "pimitemrelations.h"

class ProjectStrategy : public QObject, public ReparentingStrategy
{
    Q_OBJECT
public:
    ProjectStrategy(ProjectStructure *structure = new ProjectStructure());
    virtual void init();
    virtual Id getId(const QModelIndex& );
    virtual IdList getParents(const QModelIndex&, const IdList &ignore = IdList());
    virtual void reset();
    
    virtual void onNodeRemoval(const Id& changed);

    virtual Qt::ItemFlags flags(const QModelIndex& index, Qt::ItemFlags flags);
    virtual Qt::DropActions supportedDropActions() const;
    virtual bool onDropMimeData(Id id, const QMimeData* , Qt::DropAction );
private slots:
    void doRemoveNode(Id id);
    void doChangeParents(Id, IdList);
private:
    void checkParents(const IdList &);
//     QHash<QString, Id> mUidMapping;
//     QHash<Akonadi::Collection::Id, Id> mCollectionMapping;
    const Id mInbox;
    QScopedPointer<ProjectStructure> mRelations;
};

#endif // PROJECTSTRATEGY_H
