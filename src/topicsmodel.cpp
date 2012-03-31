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

#include "topicsmodel.h"
#include "todonode.h"
#include "globaldefs.h"
#include <KIcon>
#include <KLocalizedString>
#include <Nepomuk/Vocabulary/PIMO>
#include "todonodemanager.h"
#include <tagmanager.h>
#include <QMimeData>
#include <pimitemmodel.h>
#include <abstractpimitem.h>
#include <queries.h>
#include <pimitem.h>

TopicsModel::TopicsModel(StructureAdapter *adapter, QObject* parent)
: TodoProxyModelBase(MultiMapping, parent), m_rootNode(0), m_nepomukAdapter(adapter)
{

}

TopicsModel::~TopicsModel()
{

}

TodoNode* TopicsModel::createInbox() const
{
    TodoNode *node = new TodoNode;

    node->setData(i18n("No Topic"), 0, Qt::DisplayRole);
    node->setData(KIcon("mail-folder-inbox"), 0, Qt::DecorationRole);
    node->setRowData(Zanshin::Inbox, Zanshin::ItemTypeRole);

    return node;
}

void TopicsModel::init()
{
    TodoProxyModelBase::init();
    
    if (!m_rootNode) {
        beginInsertRows(QModelIndex(), 1, 1);

        TodoNode *node = new TodoNode;
        node->setData(i18n("Topics"), 0, Qt::DisplayRole);
        node->setData(KIcon("document-multiple"), 0, Qt::DecorationRole);
        node->setRowData(Zanshin::TopicRoot, Zanshin::ItemTypeRole);

        m_rootNode = node;
        m_manager->insertNode(m_rootNode);

        endInsertRows();
    }

    //TODO fetch topics
/*
    foreach (const QString &category, CategoryManager::instance().categories()) {
        if (!m_categoryMap.contains(category)) {
            createCategoryNode(category);
        }
    }
*/

    connect(m_nepomukAdapter, SIGNAL(parentAdded(QString,QString,QString)), this, SLOT(createNode(QString,QString,QString)));
    connect(m_nepomukAdapter, SIGNAL(parentRemoved(QString)), this, SLOT(removeNode(QString)));
    
    connect(m_nepomukAdapter, SIGNAL(itemsAdded(QString,QModelIndexList)), this, SLOT(itemsWithTopicAdded(QString,QModelIndexList)));
    connect(m_nepomukAdapter, SIGNAL(itemsRemovedFromParent(QString,QModelIndexList)), this, SLOT(itemsFromTopicRemoved(QString,QModelIndexList)));
    
    connect(m_nepomukAdapter, SIGNAL(parentChanged(QString,QString,QString)), this, SLOT(propertyChanged(QString,QString,QString)));
    m_nepomukAdapter->setType(Nepomuk::Vocabulary::PIMO::Topic()); //Generalize that we only have to parametrize the adapter and the model is fully generic
    
}






void TopicsModel::itemsWithTopicAdded(const QString &identifier, const QModelIndexList &items)
{
//     const QUrl &topic = sender()->property("resourceuri").toUrl();
//     kDebug() << topic;
    foreach (const QModelIndex &index, items) {
        //kDebug() << res.resourceUri() << res.label() << res.types() << res.className();
        if (!index.isValid()) {
            continue;
        }
        QList<TodoNode*> nodes = m_manager->nodesForSourceIndex(index);
        foreach (TodoNode *node, nodes) {
            QStringList list = node->data(0, TopicRole).toStringList();
            list.append(identifier);
            node->setData(list, 0, TopicRole);
        }

//         m_itemTopics[index.internalId()].append(identifier);
        TodoNode *parent = m_resourceMap[identifier];
        if (!parent) { //Topic is not yet in map, wait for it
            kDebug() << identifier;
            kDebug() << m_resourceMap;
            createNode(identifier, QString(), "tempname");
            parent = m_resourceMap[identifier];
//             return;
        }
        Q_ASSERT(parent);
//         const QModelIndexList &indexes = Akonadi::EntityTreeModel::modelIndexesForItem(sourceModel(), item);
//         if (indexes.isEmpty()) {
//             kDebug() << "item not found" << item.url();
//             return;
//         }

        
        foreach (TodoNode *node, nodes) {
            TodoNode *parentNode = node->parent();
            if (parentNode && parentNode->data(0, Zanshin::ItemTypeRole).toInt()==Zanshin::Inbox) {
                kDebug() << "removed node from inbox";
                int oldRow = parentNode->children().indexOf(node);
                beginRemoveRows(m_manager->indexForNode(parentNode, 0), oldRow, oldRow); //FIXME triggers multimapping warning, but there shouldn't be multiple instances of the same item under inbox
                m_manager->removeNode(node);
                delete node;
                endRemoveRows();
            }
        }

//         kDebug() << "add item: " << item.url();
        addChildNode(index, parent);
    }

}

void TopicsModel::itemsFromTopicRemoved(const QString &identifier, const QModelIndexList &items)
{
    TodoNode *parentNode = m_resourceMap[identifier];
    if (!parentNode) {
        kWarning() << "topic not in model";
        return;
    }
    kDebug() << "removing nodes from topic: " << identifier;
    foreach (const QModelIndex &index, items) {
        if (!index.isValid()) {
            continue;
        }
//         kDebug() << item.url();

//         const QModelIndexList &indexes = Akonadi::EntityTreeModel::modelIndexesForItem(sourceModel(), item);
//         if (indexes.isEmpty()) {
//             kDebug() << "item not found" << item.url();
//             continue;
//         }
        QList<TodoNode*> nodes = m_manager->nodesForSourceIndex(index);
        foreach (TodoNode *childNode, parentNode->children()) {
            if (childNode && nodes.contains(childNode)) {
                kDebug() << "removed item from topic";
                int oldRow = parentNode->children().indexOf(childNode);
                beginRemoveRows(m_manager->indexForNode(parentNode, 0), oldRow, oldRow);
                m_manager->removeNode(childNode);
                delete childNode;
                endRemoveRows();
                break;
            }
        }

        if (m_manager->nodesForSourceIndex(index).isEmpty()) {
            kDebug() << "added to inbox";
            addChildNode(index, m_inboxNode);
        }

    }
}

void TopicsModel::propertyChanged(const QString &identifier, const QString &parentIdentifier, const QString &name)
{
    kDebug() << "renamed " << identifier << " to " << name;
    TodoNode *node = m_resourceMap[identifier];
    node->setData(name, 0, Qt::DisplayRole);
    node->setData(name, 0, Qt::EditRole);
    const QModelIndex &begin = m_manager->indexForNode(node, 0);
    const QModelIndex &end = m_manager->indexForNode(node, 0);
    emit dataChanged(begin, end);
}


void TopicsModel::createNode(const QString &identifier, const QString &parentIdentifier, const QString &name)
{
    kDebug() << "add topic" << name << identifier;
    //TODO: Order them along a tree
    TodoNode* parentNode = m_rootNode;
    Q_ASSERT(parentNode);
    //TODO find super topic
    /*QString categoryName = categoryPath;
    if (categoryPath.contains(CategoryManager::pathSeparator())) {
        QString parentCategory = categoryPath.left(categoryPath.lastIndexOf(CategoryManager::pathSeparator()));
        categoryName = categoryPath.split(CategoryManager::pathSeparator()).last();
        parentNode = m_categoryMap[parentCategory];
        if (!parentNode) {
            CategoryManager::instance().addCategory(parentCategory);
            parentNode = m_categoryMap[parentCategory];
        }
    }*/

    int row = parentNode->children().size();
    kDebug() << "beforeindsert";
    beginInsertRows(m_manager->indexForNode(parentNode, 0), row, row);
    kDebug() << "afterinsert";
    TodoNode *node = new TodoNode(parentNode);
    node->setData(name, 0, Qt::DisplayRole);
    node->setData(name, 0, Qt::EditRole);
    //node->setData(categoryPath, 0, Zanshin::CategoryPathRole);
    node->setData(KIcon("view-pim-notes"), 0, Qt::DecorationRole); //TODO get icon from reesource
    node->setRowData(Zanshin::Topic, Zanshin::ItemTypeRole);
    node->setRowData(identifier, Zanshin::UriRole); //TODO don't rely on the identifier being the uri

    m_resourceMap[identifier] = node;
    m_manager->insertNode(node);
    kDebug() << identifier << node;
    endInsertRows();
}

void TopicsModel::removeNode(const QString &identifier)
{
    kDebug() << identifier;
    if (!m_resourceMap.contains(identifier)) {
        return;
    }

    TodoNode *node = m_resourceMap[identifier];

    QList<TodoNode*> children = node->children();
    foreach (TodoNode* child, children) {
        child->setParent(m_inboxNode);
        
        QModelIndex childIndex = m_manager->indexForNode(child, 0);
        if (childIndex.data(Zanshin::ItemTypeRole).toInt() == Zanshin::Topic) {
            NepomukUtils::deleteTopic(childIndex.data(Zanshin::UriRole).toUrl()); //TODO maybe leave this up to nepomuk subresource handling?
        } else {
            child->setParent(m_inboxNode);
            /*
            QStringList categories = childIndex.data(Zanshin::CategoriesRole).toStringList();
            if (categories.empty()) {
                child->setParent(m_inboxNode);
            } else {
                beginRemoveRows(childIndex.parent(), childIndex.row(), childIndex.row());
                m_manager->removeNode(child);
                delete child;
                endRemoveRows();
            }*/
        }
    }

    QModelIndex index = m_manager->indexForNode(node, 0);
    beginRemoveRows(index.parent(), index.row(), index.row());
    m_manager->removeNode(node);
    m_resourceMap.remove(identifier);
    delete node;
    endRemoveRows();
}

void TopicsModel::onSourceInsertRows(const QModelIndex& sourceIndex, int begin, int end)
{
    kDebug() << begin << end << sourceIndex;
    kDebug() << sourceModel()->rowCount();
    for (int i = begin; i <= end; i++) {
        QModelIndex sourceChildIndex = sourceModel()->index(i, 0, sourceIndex);

        if (!sourceChildIndex.isValid()) {
            kDebug() << "invalid sourceIndex";
            continue;
        }

        AbstractPimItem::ItemType type = (AbstractPimItem::ItemType) sourceChildIndex.data(PimItemModel::ItemTypeRole).toInt();
        if (type & AbstractPimItem::All) {
            QStringList topics;// = m_itemTopics[sourceChildIndex.data(PimItemModel::ItemIdRole).value<Akonadi::Item::Id>()];
            QList<TodoNode*> nodes = m_manager->nodesForSourceIndex(sourceChildIndex);
            foreach (TodoNode *node, nodes) {
                topics = node->data(0, TopicRole).toStringList();
                break;
            }
            if (topics.isEmpty()) {
                //kDebug() << "add node to inbox";
                addChildNode(sourceChildIndex, m_inboxNode);
            } else {
                foreach (const QUrl &res, topics) {
                    kDebug() << "added node to topic: " << res;
                    TodoNode *parent = m_resourceMap[res];
                    Q_ASSERT(parent);
                    addChildNode(sourceChildIndex, parent);
                }
            }
        } else {
            kDebug() << "no valid item";
        }
    }
}

void TopicsModel::onSourceRemoveRows(const QModelIndex& sourceIndex, int begin, int end)
{
    for (int i = begin; i <= end; ++i) {
        QModelIndex sourceChildIndex = sourceModel()->index(i, 0, sourceIndex);
        QList<TodoNode*> nodes = m_manager->nodesForSourceIndex(sourceChildIndex);
        foreach (TodoNode *node, nodes) {
            TodoNode *parentNode = node->parent();
            if (parentNode) {
                kDebug() << "removed node";
                int oldRow = parentNode->children().indexOf(node);
                beginRemoveRows(m_manager->indexForNode(parentNode, 0), oldRow, oldRow);
                m_manager->removeNode(node);
                delete node;
                endRemoveRows();
            }
        }
    }
}

void TopicsModel::onSourceDataChanged(const QModelIndex& begin, const QModelIndex& end)
{   
    for (int row = begin.row(); row <= end.row(); row++) {
        const QModelIndexList &list = mapFromSourceAll(sourceModel()->index(row, 0, begin.parent()));
        foreach (const QModelIndex &proxyIndex, list) {
            dataChanged(proxyIndex, proxyIndex);
        }
    }
}

QStringList TopicsModel::mimeTypes() const
{
    QStringList list = QAbstractItemModel::mimeTypes();
    list.append("text/uri-list");
    list.append("text/plain");
    return list;
}

Qt::ItemFlags TopicsModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    Zanshin::ItemType type = (Zanshin::ItemType) index.data(Zanshin::ItemTypeRole).toInt();
    if (type == Zanshin::Inbox || type == Zanshin::TopicRoot) {
        return Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
    }
    return TodoProxyModelBase::flags(index) | Qt::ItemIsDropEnabled | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
}



Qt::DropActions TopicsModel::supportedDropActions() const
{
    if (!sourceModel()) {
        return Qt::IgnoreAction;
    }
    return sourceModel()->supportedDropActions();
}

bool TopicsModel::dropMimeData(const QMimeData* mimeData, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
        //kDebug() << mimeData->formats();
    //kDebug() << mimeData->text();

    bool moveToTrash = false;
    QUrl targetTopic;
    if (parent.isValid()) {
        //kDebug() << "dropped on item " << data(parent, UriRole) << data(parent, Qt::DisplayRole).toString();
        targetTopic = parent.data(Zanshin::UriRole).value<QUrl>();
    }
/*
    if (mimeData->hasText()) { //plain text is interpreted as topic anyway, so you can drag a textfragment from anywhere and create a new topic when dropped
        const QUrl &sourceTopic = QUrl(mimeData->text());
        //beginResetModel();
        if (targetTopic.isValid()) {
            kDebug() << "set topic: " << targetTopic << " on dropped topic: " << sourceTopic;
            NepomukUtils::moveToTopic(sourceTopic, targetTopic);
        } else {
            kDebug() << "remove all topics from topic:" << sourceTopic;
            NepomukUtils::removeAllTopics(sourceTopic);
        }
        //endResetModel(); //TODO emit item move instead
        return true;
    }*/

    //TODO support also drop of other urls (files), and add to topic contextview?

    if (!mimeData->hasUrls()) {
        kWarning() << "no urls in drop";
        return false;
    }
    kDebug() << mimeData->urls();

    foreach (const KUrl &url, mimeData->urls()) {
        const Akonadi::Item item = Akonadi::Item::fromUrl(url);
        if (!item.isValid()) {
            kDebug() << "invalid item";
            continue;
        }

        if (targetTopic.isValid()) {
            kDebug() << "set topic: " << targetTopic << " on dropped item: " << item.url();
            NepomukUtils::moveToTopic(item, targetTopic);
        } else {
            kDebug() << "remove all topics from item:" << item.url();
            NepomukUtils::removeAllTopics(item);
        }

    }
    return true;
}

bool TopicsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role!=Qt::EditRole || !index.isValid()) {
        return TodoProxyModelBase::setData(index, value, role);
    }

    Zanshin::ItemType type = static_cast<Zanshin::ItemType>(index.data(Zanshin::ItemTypeRole).toInt());
    if (index.column()==0 && type==Zanshin::Topic) {
        NepomukUtils::renameTopic(index.data(Zanshin::UriRole).toUrl(), value.toString());
        return true;
    }

    return TodoProxyModelBase::setData(index, value, role);
}
