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
        node->setRowData(Zanshin::CategoryRoot, Zanshin::ItemTypeRole);

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
            createNode(res);
        }
    }
}

void TopicsModel::queryFinished()
{
    kWarning();
    //emit ready();
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
    //node->setRowData(Zanshin::Category, Zanshin::ItemTypeRole);

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
    
    //TODO Use propertycache model to find hierarchy
    for (int i = begin; i <= end; i++) {
        QModelIndex sourceChildIndex = sourceModel()->index(i, 0, sourceIndex);

        if (!sourceChildIndex.isValid()) {
            continue;
        }
        
        addChildNode(sourceChildIndex, m_rootNode);

       /* Zanshin::ItemType type = (Zanshin::ItemType) sourceChildIndex.data(Zanshin::ItemTypeRole).toInt();
        if (type==Zanshin::StandardTodo) {
            QStringList categories = sourceModel()->data(sourceChildIndex, Zanshin::CategoriesRole).toStringList();

            if (categories.isEmpty()) {
                addChildNode(sourceChildIndex, m_inboxNode);

            } else {
                QList<TodoNode*> nodes = m_manager->nodesForSourceIndex(sourceChildIndex);
                QSet<QString> oldCategories;

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
                newCategories-= interCategories;

                foreach (const QString &category, newCategories) {
                    TodoNode *parent = m_categoryMap[category];
                    Q_ASSERT(parent);
                    addChildNode(sourceChildIndex, parent);
                }
            }
        } else if (type==Zanshin::Collection) {*/
            //onSourceInsertRows(sourceChildIndex, 0, sourceModel()->rowCount(sourceChildIndex)-1);
        //}
    }
}

void TopicsModel::onSourceRemoveRows(const QModelIndex& sourceIndex, int begin, int end)
{

}

void TopicsModel::onSourceDataChanged(const QModelIndex& begin, const QModelIndex& end)
{

}





