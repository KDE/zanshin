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

#include "modelutils.h"

#include "modelbuilderbehavior.h"

using namespace Zanshin::Test;

QModelIndex ModelUtils::locateItem(QAbstractItemModel *model, const ModelPath &root)
{
    QVariantList path = root.m_path;
    QModelIndex index;

    foreach (const QVariant &pathPart, path) {
        bool found = false;

        for (int row=0; row<model->rowCount(index); row++) {
            QModelIndex childIndex = model->index(row, 0, index);
            QVariant variant = model->data(childIndex, TestDslRole);

            if (variant.userType()!=pathPart.userType()) {
                continue;
            }

            if (variant.canConvert<T>()) {
                T t1 = variant.value<T>();
                T t2 = pathPart.value<T>();
                if (t1==t2) {
                    found = true;
                }
            } else if (variant.canConvert<C>()) {
                C c1 = variant.value<C>();
                C c2 = pathPart.value<C>();
                if (c1==c2) {
                    found = true;
                }
            } else {
                Q_ASSERT(variant.canConvert<V>());
                V v1 = variant.value<V>();
                V v2 = pathPart.value<V>();
                if (v1==v2) {
                    found = true;
                }
            }

            if (found) {
                index = childIndex;
                break;
            }
        }
        Q_ASSERT(found);
    }

    return index;
}



void ModelUtils::create(QStandardItemModel *model,
                        const ModelStructure &structure,
                        const ModelPath &root,
                        ModelBuilderBehaviorBase *behavior)
{
    QModelIndex rootIndex = locateItem(model, root);
    QStandardItem *rootItem = model->itemFromIndex(rootIndex);

    bool mustDeleteBehavior = false;
    if (behavior==0) {
        behavior = new StandardModelBuilderBehavior;
        mustDeleteBehavior = true;
    }

    QList< QList<QStandardItem*> > rows = createItems(structure, behavior);

    if (mustDeleteBehavior) {
        delete behavior;
        behavior = 0;
    }

    foreach (const QList<QStandardItem*> &row, rows) {
        if (rootItem) {
            rootItem->appendRow(row);
        } else {
            model->appendRow(row);
        }
    }
}

QList< QList<QStandardItem*> > ModelUtils::createItems(const ModelStructure &structure, ModelBuilderBehaviorBase *behavior)
{
    QList< QList<QStandardItem*> > items;

    foreach (ModelStructureTreeNode* node, structure.m_roots) {
        items << createItem(node, behavior);
    }

    return items;
}

QList<QStandardItem*> ModelUtils::createItem(const ModelStructureTreeNode *node, ModelBuilderBehaviorBase *behavior)
{
    QList<QStandardItem *> row;

    ModelNode modelNode = node->modelNode();
    QVariant variant = modelNode.entity();

    if (variant.canConvert<C>()) {
        C c = variant.value<C>();
        row = behavior->expandCollection(c);
    } else if (variant.canConvert<T>()) {
        T t = variant.value<T>();
        row = behavior->expandTodo(t);
    } else {
        V v = variant.value<V>();
        row = behavior->expandVirtual(v);
    }

    foreach (QStandardItem *item, row) {
        item->setData(variant, TestDslRole);
    }

    foreach (ModelStructureTreeNode* child, node->children()) {
        row.first()->appendRow(createItem(child, behavior));
    }

    return row;
}

bool ModelUtils::destroy(QAbstractItemModel *model, const ModelPath::List &paths)
{
    foreach (ModelPath path, paths) {
        if (!destroy(model, path)) {
            return false;
        }
    }
    return true;
}

bool ModelUtils::destroy(QAbstractItemModel *model, const ModelPath &path)
{
    QModelIndex index = ModelUtils::locateItem(model, path);
    return model->removeRow(index.row(), index.parent());
}

