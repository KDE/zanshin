/* This file is part of Zanshin Todo.

   Copyright 2011 Kevin Ottens <ervin@kde.org>

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

#include "modelbuilder.h"

#include <algorithm>

#include "modelbuilderbehavior.h"
#include "modelutils.h"

using namespace Zanshin::Test;

ModelBuilder::ModelBuilder()
    : m_model(0), m_behavior(new StandardModelBuilderBehavior)
{
}

QStandardItemModel *ModelBuilder::model() const
{
    return m_model;
}

void ModelBuilder::setModel(QStandardItemModel *model)
{
    m_model = model;
}

ModelBuilderBehaviorBase *ModelBuilder::behavior() const
{
    return m_behavior;
}

void ModelBuilder::setBehavior(ModelBuilderBehaviorBase *behavior)
{
    delete m_behavior;
    if (behavior) {
        m_behavior = behavior;
    } else {
        m_behavior = new StandardModelBuilderBehavior;
    }
}

void ModelBuilder::create(const ModelStructure &structure, const ModelPath &root)
{
    QStandardItem *rootItem = locateItem(root);
    QList< QList<QStandardItem*> > rows = createItems(structure);

    foreach (const QList<QStandardItem*> &row, rows) {
        if (rootItem) {
            rootItem->appendRow(row);
        } else {
            m_model->appendRow(row);
        }
    }
}

QStandardItem *ModelBuilder::locateItem(const ModelPath &root)
{
    QModelIndex index = ModelUtils::locateItem(m_model, root);
    return m_model->itemFromIndex(index);
}

QList< QList<QStandardItem*> > ModelBuilder::createItems(const ModelStructure &structure)
{
    QList< QList<QStandardItem*> > items;

    foreach (ModelStructureTreeNode* node, structure.m_roots) {
        items << createItem(node);
    }

    return items;
}

QList<QStandardItem*> ModelBuilder::createItem(const ModelStructureTreeNode *node)
{
    QList<QStandardItem *> row;

    ModelNode modelNode = node->modelNode();
    QVariant v = modelNode.entity();

    if (v.canConvert<C>()) {
        C c = v.value<C>();
        row = m_behavior->expandCollection(c);
    } else {
        T t = v.value<T>();
        row = m_behavior->expandTodo(t);
    }

    foreach (QStandardItem *item, row) {
        item->setData(v, TestDslRole);
    }

    foreach (ModelStructureTreeNode* child, node->children()) {
        row.first()->appendRow(createItem(child));
    }

    return row;
}

