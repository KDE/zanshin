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


#include "reparentingstrategy.h"

IdList ReparentingStrategy::onSourceInsertRow(const QModelIndex& )
{
    return IdList();
}

IdList ReparentingStrategy::onSourceDataChanged(const QModelIndex& )
{
    return IdList();
}

IdList ReparentingStrategy::getParents(const qint64 )
{
    return IdList();
}

void ReparentingStrategy::onNodeRemoval(const qint64& changed)
{

}



TestReparentingStrategy::TestReparentingStrategy()
: ReparentingStrategy()
{

}

Id TestReparentingStrategy::getId(const QModelIndex &sourceChildIndex)
{
    if (!sourceChildIndex.data(IdRole).isValid()) {
        kWarning() << "error: missing idRole";
    }
    return sourceChildIndex.data(IdRole).value<Id>();
}


IdList TestReparentingStrategy::onSourceInsertRow(const QModelIndex &sourceChildIndex)
{
    if (!sourceChildIndex.isValid()) {
        kWarning() << "invalid index";
        return IdList();
    }

    if (!sourceChildIndex.data(ParentRole).isValid()) {
        return IdList();
    }
    const Id &parent = sourceChildIndex.data(ParentRole).value<Id>();
    if (parent < 0) {
        return IdList();
    }

    return IdList() << parent;
}

IdList TestReparentingStrategy::onSourceDataChanged(const QModelIndex &sourceIndex)
{
    return onSourceInsertRow(sourceIndex);
}


// void TestReparentingStrategy::addParent(const Id& identifier, const Id& parentIdentifier, const QString& name)
// {
//     kDebug() << identifier << parentIdentifier << name;
// //     m_model->createOrUpdateParent(identifier, parentIdentifier, name);
// }
// 
// void TestReparentingStrategy::setParent(const QModelIndex &item, const qint64& parentIdentifier)
// {
// //     m_model->itemParentsChanged(item, ParentStructureModel::IdList() << parentIdentifier);
// }
// 
// 
// void TestReparentingStrategy::removeParent(const ParentStructureModel::Id& identifier)
// {
// //     m_model->removeNode(identifier);
// }

