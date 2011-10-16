/*

    Copyright <year>  <name of author> <e-mail>

    This library is free software;
    you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation;
    either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
            successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY;
    without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "topicsmodel.h"
#include "todonode.h"
#include "globaldefs.h"
#include <KIcon>
#include <KLocalizedString>
#include <Nepomuk/Query/Query>
#include <nepomuk/resourcetypeterm.h>
#include <Nepomuk/Vocabulary/PIMO>
#include <Nepomuk/Query/QueryServiceClient>
#include <Nepomuk/Query/Result>
#include <Nepomuk/Resource>
#include "todonodemanager.h"
#include <tagmanager.h>
#include <QMimeData>
#include <notetakermodel.h>
#include <abstractpimitem.h>
#include <nepomukpropertyproxy.h>
#include <queries.h>
#include <pimitem.h>

TopicsModel::TopicsModel(QObject* parent)
: TodoProxyModelBase(MultiMapping, parent), m_rootNode(0)
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

    Nepomuk::Query::Query query;
    /*switch (type) {
        case Topics:*/
            query.setTerm(Nepomuk::Query::ResourceTypeTerm(Nepomuk::Types::Class(Nepomuk::Vocabulary::PIMO::Topic())));
            //break;
        /*case Projects:
        default:
            Q_ASSERT(0);
    }*/

    Nepomuk::Query::QueryServiceClient *queryServiceClient = new Nepomuk::Query::QueryServiceClient(this);
    connect(queryServiceClient, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)), this, SLOT(checkResults(QList<Nepomuk::Query::Result>)));
    connect(queryServiceClient, SIGNAL(finishedListing()), this, SLOT(queryFinished()));
    //connect(queryServiceClient, SIGNAL(finishedListing()), queryServiceClient, SLOT(deleteLater()));
    if ( !queryServiceClient->query(query) ) {
        kWarning() << "error";
    }
}


void TopicsModel::checkResults(QList< Nepomuk::Query::Result > results)
{
    //kDebug() <<  results.size() << results.first().resource().resourceUri() << results.first().resource().label() << results.first().resource().types() << results.first().resource().className();
    foreach (const Nepomuk::Query::Result &result, results) {
        Nepomuk::Resource res = Nepomuk::Resource(result.resource().resourceUri());
        kDebug() << res.resourceUri() << res.label() << res.types() << res.className();
        if (res.types().contains(Nepomuk::Vocabulary::PIMO::Topic()) || res.types().contains(Nepomuk::Vocabulary::PIMO::Project())) {
            addTopic(res);
        }
    }
}

void TopicsModel::queryFinished()
{
    kWarning();
    //emit ready();
}

void TopicsModel::addTopic (const Nepomuk::Resource& topic)
{
    Nepomuk::Query::QueryServiceClient *queryServiceClient = new Nepomuk::Query::QueryServiceClient(this);
    connect(queryServiceClient, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)), this, SLOT(itemsWithTopicAdded(QList<Nepomuk::Query::Result>)));
    connect(queryServiceClient, SIGNAL(finishedListing()), this, SLOT(queryFinished()));
    //connect(queryServiceClient, SIGNAL(finishedListing()), queryServiceClient, SLOT(deleteLater()));
    if ( !queryServiceClient->sparqlQuery(MindMirrorQueries::itemsWithTopicsQuery(QList <QUrl>() << topic.resourceUri())) ) {
        kWarning() << "error";
    }
    createNode(topic);
}

void TopicsModel::itemsWithTopicAdded(const QList<Nepomuk::Query::Result> &results)
{
    QUrl topic;
    foreach (const Nepomuk::Query::Result &result, results) {
        Nepomuk::Resource res = Nepomuk::Resource(result.resource().resourceUri());
        //kDebug() << res.resourceUri() << res.label() << res.types() << res.className();
        const Akonadi::Item item = PimItemUtils::getItemFromResource(res);
        
        TodoNode *parent = m_resourceMap[topic];
        Q_ASSERT(parent);
        const QModelIndexList &indexes = Akonadi::EntityTreeModel::modelIndexesForItem(this, item);
        if (indexes.isEmpty()) {
            kDebug() << "item not found" << item.url();
            return;
        }
        //Q_ASSERT(indexes.size() == 1); //assumption that every item is only once shown in the list
        addChildNode(indexes.first(), parent);
    }

}


void TopicsModel::createNode(const Nepomuk::Resource& res)
{
    //TODO: Order them along a tree
    TodoNode* parentNode = m_rootNode;
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

    beginInsertRows(m_manager->indexForNode(parentNode, 0), row, row);

    TodoNode *node = new TodoNode(parentNode);
    node->setData(res.label(), 0, Qt::DisplayRole);
    node->setData(res.label(), 0, Qt::EditRole);
    //node->setData(categoryPath, 0, Zanshin::CategoryPathRole);
    node->setData(KIcon("view-pim-notes"), 0, Qt::DecorationRole); //TODO get icon from reesource
    node->setRowData(Zanshin::Topic, Zanshin::ItemTypeRole);
    node->setRowData(res.resourceUri(), Zanshin::UriRole);

    m_resourceMap[res.resourceUri()] = node;
    m_manager->insertNode(node);

    endInsertRows();
}

void TopicsModel::removeNode(const Nepomuk::Resource& res)
{
    /*
    if (!m_resourceMap.contains(res.resourceUri())) {
        return;
    }

    TodoNode *node = m_resourceMap[res.resourceUri()];

    QList<TodoNode*> children = node->children();
    foreach (TodoNode* child, children) {
        QModelIndex childIndex = m_manager->indexForNode(child, 0);
        if (childIndex.data(Zanshin::ItemTypeRole).toInt() == Zanshin::Category) {
            CategoryManager::instance().removeCategory(childIndex.data(Zanshin::CategoryPathRole).toString());
        } else {
            QStringList categories = childIndex.data(Zanshin::CategoriesRole).toStringList();
            if (categories.empty()) {
                child->setParent(m_inboxNode);
            } else {
                beginRemoveRows(childIndex.parent(), childIndex.row(), childIndex.row());
                m_manager->removeNode(child);
                delete child;
                endRemoveRows();
            }
        }
    }

    QModelIndex index = m_manager->indexForNode(node, 0);
    beginRemoveRows(index.parent(), index.row(), index.row());
    m_manager->removeNode(node);
    m_resourceMap.remove(res.resourceUri());
    delete node;
    endRemoveRows();*/
}


void TopicsModel::onSourceInsertRows(const QModelIndex& sourceIndex, int begin, int end)
{
    kDebug() << begin << end;
    //TODO Use propertycache model to find hierarchy
    for (int i = begin; i <= end; i++) {
        QModelIndex sourceChildIndex = sourceModel()->index(i, 0, sourceIndex);

        if (!sourceChildIndex.isValid()) {
            continue;
        }
        
        //addChildNode(sourceChildIndex, m_rootNode);

        AbstractPimItem::ItemType type = (AbstractPimItem::ItemType) sourceChildIndex.data(NotetakerModel::ItemTypeRole).toInt();
        if (type & AbstractPimItem::All) {
            QVariantList resources = sourceModel()->data(sourceChildIndex, NepomukPropertyProxy::PropertyRole).toList();

            if (resources.isEmpty()) {
                addChildNode(sourceChildIndex, m_inboxNode);
            } else {
                //QList<TodoNode*> nodes = m_manager->nodesForSourceIndex(sourceChildIndex);
                /*QSet<QString> oldCategories;

                foreach (TodoNode *node, nodes) {

                    TodoNode *categoryNode = node->parent();
                    if (categoryNode
                     && categoryNode->data(0, Zanshin::ItemTypeRole).toInt()!=Zanshin::Inbox) {
                        QString category = categoryNode->data(0, Zanshin::CategoryPathRole).toString();
                        oldCategories << category;
                    }
                }

                QSet<QString> newCategories = QSet<QString>::fromList(categories);
                QSet<QString> interCategories = newCategories;
                interCategories.intersect(oldCategories);
                newCategories-= interCategories;*/

                foreach (const QVariant &res, resources) {
                    TodoNode *parent = m_resourceMap[res.toUrl()];
                    Q_ASSERT(parent);
                    addChildNode(sourceChildIndex, parent);
                }
            }
        }/* else if (type==Zanshin::Collection) {
            //onSourceInsertRows(sourceChildIndex, 0, sourceModel()->rowCount(sourceChildIndex)-1);
        }*/
    }
}

void TopicsModel::onSourceRemoveRows(const QModelIndex& sourceIndex, int begin, int end)
{

}

void TopicsModel::onSourceDataChanged(const QModelIndex& begin, const QModelIndex& end)
{   
    //return KIdentityProxyModelCopy::onSourceDataChanged
    kDebug() << begin.row() << end.row();
    for (int row=begin.row(); row<=end.row(); ++row) {
        QModelIndex sourceIndex = begin.sibling(row, 0);
        kDebug() << "----- index: " << sourceIndex;
        QList<TodoNode*> nodes = m_manager->nodesForSourceIndex(sourceIndex);

        if (!sourceIndex.data(NotetakerModel::ItemTypeRole).toInt()) {
            kDebug() << "invalid item";
            return;
        }

        QSet<QUrl> oldTopics;
        QHash<QUrl, TodoNode*> nodeMap;
        foreach (TodoNode *node, nodes) {
            QModelIndex begin = m_manager->indexForNode(node, 0);
            QModelIndex end = m_manager->indexForNode(node, qMax(begin.column(), end.column()));
            emit dataChanged(begin, end);

            TodoNode *categoryNode = node->parent();
            if (categoryNode
             && categoryNode->data(0, Zanshin::ItemTypeRole).toInt()!=Zanshin::Inbox) {
                QUrl res = categoryNode->data(0, Zanshin::UriRole).toUrl();
                oldTopics << res;
                nodeMap[res] = node;
            }
        }

        QSet<QUrl> newTopics;
        foreach (const QVariant &res, sourceIndex.data(NepomukPropertyProxy::PropertyRole).toList()) {
            newTopics << res.toUrl();
        }
            
        QSet<QUrl> inter = newTopics;
        inter.intersect(oldTopics);
        newTopics-= inter;
        oldTopics-= inter;

        foreach (const QUrl &old, oldTopics) {
            TodoNode *parentNode = m_resourceMap[old];
            TodoNode *node = nodeMap[old];

            int oldRow = parentNode->children().indexOf(node);
            beginRemoveRows(m_manager->indexForNode(parentNode, 0), oldRow, oldRow);
            m_manager->removeNode(node);
            delete node;
            endRemoveRows();
        }
        kDebug() << "removed from old topics: " << oldTopics;


        if (!oldTopics.isEmpty()) {
            //item has been removed from topics and has no new topic, so goes to inbox
            if (/*sourceIndex.data(NepomukPropertyProxy::PropertyRole).toList().isEmpty()*/newTopics.isEmpty()) { 
                addChildNode(sourceIndex, m_inboxNode);
                kDebug() << "back to inbox";
            }
        }

        if (!newTopics.isEmpty()) {
            TodoNode *node = 0;
            QList<TodoNode*> nodes = m_manager->nodesForSourceIndex(sourceIndex);
            foreach (TodoNode *n, nodes) {
                if (n->parent() == m_inboxNode) {
                    node = n;
                    break;
                }
            }
            if (node) {
                int oldRow = m_inboxNode->children().indexOf(node);
                beginRemoveRows(m_manager->indexForNode(m_inboxNode, 0), oldRow, oldRow);
                m_manager->removeNode(node);
                delete node;
                endRemoveRows();
            }

            foreach (const QUrl &newRes, newTopics) {
                TodoNode *parent = m_resourceMap[newRes];
                Q_ASSERT(parent);
                addChildNode(sourceIndex, parent);
            }
            kDebug() << "added to new topics: " << newTopics;
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

