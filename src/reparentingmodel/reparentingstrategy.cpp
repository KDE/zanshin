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
#include "globaldefs.h"
#include "reparentingmodel.h"
#include "todonode.h"
#include <pimitem.h>
#include <tagmanager.h>
#include <klocalizedstring.h>
#include <KIcon>



ReparentingStrategy::ReparentingStrategy()
:   mReparentOnRemoval(true),
    mMinIdCounter(10),
    mIdCounter(mMinIdCounter)
{

}


IdList ReparentingStrategy::getParents(const QModelIndex &, const IdList & )
{
    return IdList();
}

void ReparentingStrategy::onNodeRemoval(const qint64& changed)
{

}

void ReparentingStrategy::setModel(ReparentingModel* model)
{
    m_model = model;
}

Id ReparentingStrategy::getNextId()
{
    return mIdCounter++;
}

void ReparentingStrategy::setMinId(Id id)
{
    mMinIdCounter = id;
}

void ReparentingStrategy::reset()
{
    mIdCounter = mMinIdCounter;
}


TodoNode *ReparentingStrategy::createNode(Id id, IdList parents, QString name)
{
    kDebug() << id << parents << name;
    return m_model->createNode(id, parents, name);
}

void ReparentingStrategy::renameNode(Id id, QString name)
{
    m_model->renameNode(id, name);
}

void ReparentingStrategy::updateParents(Id id, IdList parents)
{
    m_model->reparentNode(id, parents);
}

void ReparentingStrategy::removeNode(Id id)
{
    kDebug() << id;
    m_model->removeNodeById(id);
}


bool ReparentingStrategy::reparentOnRemoval(Id) const
{
    return mReparentOnRemoval;
}

QVariant ReparentingStrategy::getData(Id id, int role)
{
    TodoNode *node = m_model->m_parentMap.value(id);
    Q_ASSERT(node);
    return node->data(0, role);
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

IdList TestReparentingStrategy::getParents(const QModelIndex &sourceChildIndex, const IdList &ignore)
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
    if (ignore.contains(parent)) {
        return IdList();
    }

    return IdList() << parent;
}




TestParentStructureStrategy::TestParentStructureStrategy()
: ReparentingStrategy()
{
}

void TestParentStructureStrategy::init()
{
    ReparentingStrategy::init();
    TodoNode *node = createNode(997, IdList(), "No Topic");
    node->setData(i18n("No Topic"), 0, Qt::DisplayRole);
    node->setData(KIcon("mail-folder-inbox"), 0, Qt::DecorationRole);
    node->setRowData(Zanshin::Inbox, Zanshin::ItemTypeRole);

    TodoNode *node2 = createNode(998, IdList(), "Topics");
    node2->setData(i18n("Topics"), 0, Qt::DisplayRole);
    node2->setData(KIcon("document-multiple"), 0, Qt::DecorationRole);
    node2->setRowData(Zanshin::TopicRoot, Zanshin::ItemTypeRole);
}

bool TestParentStructureStrategy::reparentOnRemoval(Id id) const
{
    if (id < 900) {
        kDebug() << "reparent " << id;
        return false;
    }
    return true;
}


Id TestParentStructureStrategy::getId(const QModelIndex &sourceChildIndex)
{
    if (!sourceChildIndex.isValid()) {
        kWarning() << "invalid index";
        return -1;
    }
    return sourceChildIndex.data(102).value<qint64>()+1000;

//     Zanshin::ItemType type = static_cast<Zanshin::ItemType>(sourceChildIndex.data(Zanshin::ItemTypeRole).toInt());
//     if (type == Zanshin::Inbox) {
//         return 997;
//     }
//     if (type == Zanshin::TopicRoot) {
//         return 998;
//     }
//     
//     if (!sourceChildIndex.data(TopicRole).isValid()) {
// //         Q_ASSERT(sourceChildIndex.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>().isValid());
// //         return sourceChildIndex.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>().id()+1000;
//         Q_ASSERT(sourceChildIndex.data(102).canConvert<qint64>());
//         kDebug() << sourceChildIndex.data(102).value<qint64>();
//         return sourceChildIndex.data(102).value<qint64>()+1000;
//     }
//     Q_ASSERT(sourceChildIndex.data(TopicRole).canConvert(QVariant::Int));
//     kDebug() << sourceChildIndex.data(TopicRole).toInt();
//     return sourceChildIndex.data(TopicRole).toInt();
}

IdList TestParentStructureStrategy::getParents(const QModelIndex &sourceChildIndex, const IdList &ignore)
{
    Q_ASSERT(sourceChildIndex.isValid());

    if (!sourceChildIndex.data(TopicParentRole).isValid()) {
//         if (sourceChildIndex.data(TopicRole).isValid()) {
//             kWarning() << "topic role";
//             return IdList() << 998; //Topics
//         }
        return IdList() << 997; //No Topics
    }
    const Id &parent = sourceChildIndex.data(TopicParentRole).value<Id>();
    if (ignore.contains(parent)) {
        return IdList() << 997; //No Topics
    }
    return IdList() << parent;
}

void TestParentStructureStrategy::addParent(Id identifier, Id parentIdentifier, const QString& name)
{
    kDebug() << identifier << parentIdentifier << name;
    if (parentIdentifier < 0 ) {
        parentIdentifier = 998;
    }
    createNode(identifier, IdList() << parentIdentifier, name);
}

void TestParentStructureStrategy::setParent(const QModelIndex &item, const qint64& parentIdentifier)
{
    updateParents(getId(item), IdList() << parentIdentifier);
}


void TestParentStructureStrategy::removeParent(const Id& identifier)
{
    removeNode(identifier);
}

// InboxStrategy::InboxStrategy(const QString& inboxName, const QString& rootName)
// {
// 
// }
// 
// void InboxStrategy::init()
// {
//     ReparentingStrategy::init();
// }



