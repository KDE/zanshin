/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.
*/

#include "todocategoriesmodel.h"

#include <akonadi/collection.h>
#include <akonadi/collectionmodifyjob.h>
#include <akonadi/item.h>

#include <qmimedata.h>

#include <KDebug>
#include <KIcon>

#include <QtCore/QStringList>

#include "todocategoriesattribute.h"
#include "todoflatmodel.h"

class TodoCategoryTreeNode
{
public:
    TodoCategoryTreeNode(const QString &c, TodoCategoryTreeNode *p = 0)
        : id(-1), category(c), parent(p) { }

    TodoCategoryTreeNode(Akonadi::Entity::Id i, TodoCategoryTreeNode *p = 0)
        : id(i), parent(p) { }

    ~TodoCategoryTreeNode() { qDeleteAll(children); }

    Akonadi::Entity::Id id;
    QString category;

    TodoCategoryTreeNode *parent;
    QList<TodoCategoryTreeNode*> children;
};

TodoCategoriesModel::TodoCategoriesModel(QObject *parent)
    : QAbstractProxyModel(parent)
{
}

TodoCategoriesModel::~TodoCategoriesModel()
{
    qDeleteAll(m_roots);
}

QModelIndex TodoCategoriesModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0
     || row >= rowCount(parent)
     || column >= columnCount(parent)) {
        return QModelIndex();
    }

    return createIndex(row, column, nodeForIndex(parent));
}

QModelIndex TodoCategoriesModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    TodoCategoryTreeNode *parent = nodeForIndex(index)->parent;

    if (parent == 0) {
        return QModelIndex();
    } else {
        return indexForNode(parent);
    }
}

int TodoCategoriesModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_roots.count();
    } else if (parent.column() == 0) { // Only one set of children per row
        TodoCategoryTreeNode *node = nodeForIndex(parent);
        return node->children.count();
    }

    return 0;
}

int TodoCategoriesModel::columnCount(const QModelIndex &/*parent*/) const
{
    return TodoFlatModel::LastColumn+1;
}

QVariant TodoCategoriesModel::data(const QModelIndex &index, int role) const
{
    TodoCategoryTreeNode *node = nodeForIndex(index);

    if (node == 0) {
        return QVariant();
    }

    if (node->id != -1) {
        return QAbstractProxyModel::data(index, role);
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (index.column() == TodoFlatModel::Summary) {
            return node->category;
        } else if (index.column() == TodoFlatModel::Categories && node->parent != 0) {
            return QStringList() << node->parent->category;
        }
    } else if (role == Qt::DecorationRole && index.column() == TodoFlatModel::Summary) {
        return KIcon("view-pim-notes");
    }

    return QVariant();
}

QMimeData *TodoCategoriesModel::mimeData(const QModelIndexList &indexes) const
{
    QModelIndexList sourceIndexes;
    foreach (const QModelIndex &proxyIndex, indexes) {
        sourceIndexes << mapToSource(proxyIndex);
    }

    return flatModel()->mimeData(sourceIndexes);
}

bool TodoCategoriesModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                       int /*row*/, int /*column*/, const QModelIndex &parent)
{
    if (action != Qt::MoveAction)
        return false;

    QStringList list = data->text().split(", ");
    QString parentCategory = categoryForIndex(parent);
    foreach(const QString remoteId, list) {
        QModelIndex index = flatModel()->indexForRemoteId(remoteId);
        Akonadi::Item item = flatModel()->itemForIndex(index);
        QModelIndex catIndex = flatModel()->indexForItem(item, TodoFlatModel::Categories);
        if (!flatModel()->setData(catIndex, parentCategory)) {
            return false;
        }
    }
    return true;
}

QVariant TodoCategoriesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return flatModel()->headerData(section, orientation, role);
}

Qt::ItemFlags TodoCategoriesModel::flags(const QModelIndex &index) const
{
    TodoCategoryTreeNode *node = nodeForIndex(index);
    if (node->id==-1) {
        switch (index.column()) {
        case TodoFlatModel::Summary:
        case TodoFlatModel::Categories:
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
        default:
            break;
        }
    } else if (node->id!=-1)
        return QAbstractProxyModel::flags(index);

    return Qt::NoItemFlags;
}

QModelIndex TodoCategoriesModel::mapToSource(const QModelIndex &proxyIndex) const
{
    TodoCategoryTreeNode *node = nodeForIndex(proxyIndex);

    if (!node || node->id == -1) {
        return QModelIndex();
    }

    return flatModel()->indexForItem(Akonadi::Item(node->id),
                                     proxyIndex.column());
}

QList<QModelIndex> TodoCategoriesModel::mapFromSourceAll(const QModelIndex &sourceIndex) const
{
    Akonadi::Item item = flatModel()->itemForIndex(sourceIndex);

    Akonadi::Entity::Id id = item.id();

    if (!m_itemMap.contains(id)) {
        return QList<QModelIndex>();
    }

    QList<TodoCategoryTreeNode*> nodes = m_itemMap[id];
    QList<QModelIndex> res;

    foreach (TodoCategoryTreeNode *node, nodes)
        res << indexForNode(node, sourceIndex.column());

    return res;
}

QModelIndex TodoCategoriesModel::mapFromSource(const QModelIndex &/*sourceIndex*/) const
{
    kError() << "Never call mapFromSource() on TodoCategoriesModel";
    return QModelIndex();
}

void TodoCategoriesModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    if (flatModel()) {
        disconnect(flatModel());
    }

    Q_ASSERT(sourceModel == 0 || qobject_cast<TodoFlatModel*>(sourceModel) != 0);
    QAbstractProxyModel::setSourceModel(sourceModel);
    m_collection = flatModel()->collection();

    onSourceInsertRows(QModelIndex(), 0, flatModel()->rowCount() - 1);

    connect(flatModel(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(onSourceDataChanged(const QModelIndex&, const QModelIndex&)));
    connect(flatModel(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(onSourceInsertRows(const QModelIndex&, int, int)));
    connect(flatModel(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
            this, SLOT(onSourceRemoveRows(const QModelIndex&, int, int)));
    connect(flatModel(), SIGNAL(collectionChanged(const Akonadi::Collection&)),
            this, SLOT(onSourceCollectionChanged(const Akonadi::Collection&)));
}

void TodoCategoriesModel::onSourceInsertRows(const QModelIndex &sourceIndex, int begin, int end)
{
    for (int i = end; i >= begin; i--) {
        QStringList categories = flatModel()->data(
            flatModel()->index(i, TodoFlatModel::Categories, sourceIndex)).toStringList();

        if (categories.isEmpty()) continue;

        foreach (const QString &category, categories) {
            if (!m_categoryMap.contains(category)) {
                TodoCategoryTreeNode *node = new TodoCategoryTreeNode(category);
                loadCategory(node);
            }

            TodoCategoryTreeNode *parent = m_categoryMap[category];
            QModelIndex proxyParentIndex = indexForNode(parent);
            int row = parent->children.size();

            beginInsertRows(proxyParentIndex, row, row);

            Akonadi::Item item = flatModel()->itemForIndex(flatModel()->index(i, 0));
            Akonadi::Entity::Id id = item.id();

            TodoCategoryTreeNode *child = new TodoCategoryTreeNode(id, parent);
            parent->children << child;
            m_itemMap[id] << child;

            endInsertRows();

            emit layoutChanged();
        }
    }

    serializeCategories();
}

void TodoCategoriesModel::onSourceRemoveRows(const QModelIndex &/*sourceIndex*/, int begin, int end)
{
    for (int i = begin; i <= end; ++i) {
        QModelIndex sourceIndex = flatModel()->index(i, 0);
        QModelIndexList proxyIndexes = mapFromSourceAll(sourceIndex);

        foreach (const QModelIndex &proxyIndex, proxyIndexes) {
            TodoCategoryTreeNode *node = nodeForIndex(proxyIndex);
            TodoCategoryTreeNode *parentNode = nodeForIndex(proxyIndex.parent());

            beginRemoveRows(proxyIndex.parent(), proxyIndex.row(), proxyIndex.row());
            m_itemMap.remove(itemForIndex(proxyIndex).id());
            parentNode->children.removeAll(node);
            delete node;
            endRemoveRows();
        }
    }
}

void TodoCategoriesModel::onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end)
{
    for (int row=begin.row(); row<=end.row(); ++row) {
        QModelIndex sourceIndex = flatModel()->index(row, 0);
        QModelIndexList proxyIndexes = mapFromSourceAll(sourceIndex);

        Akonadi::Entity::Id id = flatModel()->itemForIndex(sourceIndex).id();

        QSet<QString> oldCategories;
        QHash<QString, QModelIndex> indexMap;
        foreach (const QModelIndex &proxyIndex, proxyIndexes) {
            emit dataChanged(index(proxyIndex.row(), 0, proxyIndex.parent()),
                             index(proxyIndex.row(), TodoFlatModel::LastColumn, proxyIndex.parent()));
            TodoCategoryTreeNode *node = nodeForIndex(proxyIndex.parent());
            if (node!=0 && node->id==-1) {
                oldCategories << node->category;
                indexMap[node->category] = proxyIndex;
            }
        }

        QModelIndex catIndex = flatModel()->index(row, TodoFlatModel::Categories);
        QSet<QString> newCategories = QSet<QString>::fromList(flatModel()->data(catIndex).toStringList());

        QSet<QString> interCategories = newCategories;
        interCategories.intersect(oldCategories);
        newCategories-= interCategories;
        oldCategories-= interCategories;

        foreach (const QString &oldCategory, oldCategories) {
            TodoCategoryTreeNode *parentNode = m_categoryMap[oldCategory];
            TodoCategoryTreeNode *node = nodeForIndex(indexMap[oldCategory]);

            int oldRow = parentNode->children.indexOf(node);
            beginRemoveRows(indexForNode(parentNode), oldRow, oldRow);
            parentNode->children.removeAll(node);
            m_itemMap[id].removeAll(node);
            delete node;
            endRemoveRows();
        }

        foreach (const QString &newCategory, newCategories) {
            if (!m_categoryMap.contains(newCategory)) {
                TodoCategoryTreeNode *node = new TodoCategoryTreeNode(newCategory);
                loadCategory(node);
            }

            TodoCategoryTreeNode *parent = m_categoryMap[newCategory];
            QModelIndex proxyParentIndex = indexForNode(parent);
            int row = parent->children.size();

            beginInsertRows(proxyParentIndex, row, row);

            TodoCategoryTreeNode *child = new TodoCategoryTreeNode(id, parent);
            parent->children << child;
            m_itemMap[id] << child;

            endInsertRows();
        }
    }

    serializeCategories();
}

void TodoCategoriesModel::onSourceCollectionChanged(const Akonadi::Collection &collection)
{
    m_itemMap.clear();
    m_categoryMap.clear();
    qDeleteAll(m_roots);
    m_roots.clear();

    reset();

    m_collection = collection;
    emit collectionChanged(collection);

    deserializeCategories();
}

TodoCategoryTreeNode *TodoCategoriesModel::nodeForIndex(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return 0;
    }

    TodoCategoryTreeNode *parentNode = static_cast<TodoCategoryTreeNode*>(index.internalPointer());
    if (parentNode != 0) {
        return parentNode->children.at(index.row());
    } else {
        return m_roots.at(index.row());
    }
}

QModelIndex TodoCategoriesModel::indexForNode(TodoCategoryTreeNode *node, int column) const
{
    if (node == 0) {
        return QModelIndex();
    }

    TodoCategoryTreeNode *parentNode = node->parent;
    int row;
    if (parentNode != 0) {
        row = parentNode->children.indexOf(node);
    } else {
        row = m_roots.indexOf(node);
    }

    return createIndex(row, column, parentNode);
}

Akonadi::Item TodoCategoriesModel::itemForIndex(const QModelIndex &index) const
{
    return flatModel()->itemForIndex(mapToSource(index));
}

QList<QModelIndex> TodoCategoriesModel::indexesForItem(const Akonadi::Item &item, int column) const
{
    return mapFromSourceAll(flatModel()->indexForItem(item, column));
}

QString TodoCategoriesModel::categoryForIndex(const QModelIndex &index) const
{
    TodoCategoryTreeNode *node = nodeForIndex(index);
    if (node != 0 && node->id == -1) {
        return node->category;
    } else {
        return QString();
    }
}

QModelIndex TodoCategoriesModel::indexForCategory(const QString &category, int column) const
{
    if (m_categoryMap.contains(category)) {
        TodoCategoryTreeNode *node = m_categoryMap[category];
        return indexForNode(node, column);
    }

    return QModelIndex();
}

void TodoCategoriesModel::setCollection(const Akonadi::Collection &collection)
{
    flatModel()->setCollection(collection);
}

Akonadi::Collection TodoCategoriesModel::collection() const
{
    return flatModel()->collection();
}

TodoFlatModel *TodoCategoriesModel::flatModel() const
{
    return qobject_cast<TodoFlatModel*>(sourceModel());
}

void TodoCategoriesModel::loadDefaultCategories()
{
    TodoCategoryTreeNode *errands = new TodoCategoryTreeNode("Errands");
    TodoCategoryTreeNode *home = new TodoCategoryTreeNode("Home");
    TodoCategoryTreeNode *office = new TodoCategoryTreeNode("Office");
    TodoCategoryTreeNode *computer = new TodoCategoryTreeNode("Computer");
    TodoCategoryTreeNode *online = new TodoCategoryTreeNode("Online", computer);
    computer->children << online;
    TodoCategoryTreeNode *phone = new TodoCategoryTreeNode("Phone");

    QList<TodoCategoryTreeNode*> roots;
    roots << home << office << errands << computer << phone;

    foreach (TodoCategoryTreeNode *node, roots) {
        loadCategory(node);
    }

    serializeCategories();
}

void TodoCategoriesModel::loadCategory(TodoCategoryTreeNode *node)
{
    m_categoryMap[node->category] = node;

    int row;
    if (node->parent != 0) {
        row = node->parent->children.indexOf(node);
    } else {
        m_roots << node;
        row = m_roots.indexOf(node);
    }

    QModelIndex parentIndex = indexForNode(node->parent);
    beginInsertRows(parentIndex, row, row);
    endInsertRows();

    foreach (TodoCategoryTreeNode *child, node->children) {
        loadCategory(child);
    }
}

void TodoCategoriesModel::serializeCategories()
{
    if (!m_collection.isValid()) return;

    QStringList parentList;

    foreach (TodoCategoryTreeNode *node, m_categoryMap.values()) {
        QString key = node->category;
        QString value;
        if (node->parent != 0) {
            value = node->parent->category;
        }
        parentList << key << value;
    }

    TodoCategoriesAttribute *attribute = m_collection.attribute<TodoCategoriesAttribute>(Akonadi::Collection::AddIfMissing);
    attribute->setParentList(parentList);

    new Akonadi::CollectionModifyJob(m_collection, this);
}

void TodoCategoriesModel::deserializeCategories()
{
    if (!m_collection.isValid()) return;

    if (!m_collection.hasAttribute <TodoCategoriesAttribute>()) {
        loadDefaultCategories();
        return;
    }

    TodoCategoriesAttribute *attribute = m_collection.attribute<TodoCategoriesAttribute>();
    QStringList parentList = attribute->parentList();

    QHash<QString, TodoCategoryTreeNode*> categoryMap;
    QList<TodoCategoryTreeNode*> roots;

    for (int i = 0; i < parentList.size(); i += 2) {
        QString child = parentList.at(i);
        QString parent;
        if (i + 1 < parentList.size()) {
            parent = parentList.at(i + 1);
        }

        if (!categoryMap.contains(child)) {
            TodoCategoryTreeNode *node = new TodoCategoryTreeNode(child);
            categoryMap[child] = node;
        }

        if (!categoryMap.contains(parent)) {
            TodoCategoryTreeNode *node = new TodoCategoryTreeNode(parent);
            categoryMap[parent] = node;
        }
    }

    for (int i = 0; i < parentList.size(); i += 2) {
        QString child = parentList.at(i);
        QString parent;
        if (i + 1 < parentList.size()) {
            parent = parentList.at(i + 1);
        }

        TodoCategoryTreeNode *node = categoryMap[child];
        if (parent.isEmpty()) {
            roots << node;
        } else {
            TodoCategoryTreeNode *parentNode = categoryMap[parent];
            node->parent = parentNode;
            parentNode->children << node;
        }
    }

    foreach (TodoCategoryTreeNode *node, roots) {
        loadCategory(node);
    }
}
