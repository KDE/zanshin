/*
 * Copyright (C) 2012  Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PIMITEMRELATIONSTRATEGY_H
#define PIMITEMRELATIONSTRATEGY_H

#include "reparentingmodel/reparentingstrategy.h"
#include "core/pimitemrelations.h"

class PimItemRelationStrategy : public QObject, public ReparentingStrategy
{
    Q_OBJECT
public:
    PimItemRelationStrategy(PimItemRelation::Type type);
    virtual void init();
    virtual Id getId(const QModelIndex& );
    virtual IdList getParents(const QModelIndex&, const IdList &ignore = IdList());
    virtual void reset();

    virtual void setNodeData(TodoNode* node, Id id);
    virtual void onNodeRemoval(const Id& changed);

    virtual QVariant data(Id index, int column, int role, bool &forward) const;

    virtual Qt::ItemFlags flags(const QModelIndex& index, Qt::ItemFlags flags);
    virtual QStringList mimeTypes();
    virtual QMimeData* mimeData(const QModelIndexList& indexes) const;
    virtual Qt::DropActions supportedDropActions() const;

    virtual bool onDropMimeData(Id id, const QMimeData* , Qt::DropAction );
    virtual bool onSetData(Id id, const QVariant& value, int role, int column);

    virtual bool reparentOnParentRemoval(Id child) const;
private slots:
    void createVirtualNode(Id id, IdList parents, const QString &name);
    void doRemoveNode(Id id);
    void doChangeParents(Id, IdList);
    void doRenameParent(Id, const QString &name);
    void doUpdateItems(const IdList &);

private:
    const Id mInbox;
    const Id mRoot;
    QScopedPointer<VirtualRelationCache> mRelations;
    const PimItemRelation::Type mType; ///< Either PimItemRelation::Context or PimItemRelation::Topic
};


#endif // PIMITEMRELATIONSTRATEGY_H
