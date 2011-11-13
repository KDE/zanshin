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

#ifndef TOPICSMODEL_H
#define TOPICSMODEL_H
#include "todoproxymodelbase.h"
#include <akonadi/item.h>

namespace Akonadi
{
class Item;
}

namespace Nepomuk {
namespace Query {

class Result;
}

    class Resource;
}

class TopicsModel : public TodoProxyModelBase
{
     Q_OBJECT

public:
    TopicsModel(QObject *parent = 0);
    virtual ~TopicsModel();

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    //virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    virtual QStringList mimeTypes() const;
    virtual Qt::DropActions supportedDropActions() const;

    //virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

private slots:
    void onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end);
    void onSourceInsertRows(const QModelIndex &sourceIndex, int begin, int end);
    void onSourceRemoveRows(const QModelIndex &sourceIndex, int begin, int end);
    //void onTopicAdded();
    //void onTopicRemoved();
    
    void createNode(const Nepomuk::Resource &res);
    void removeNode(const Nepomuk::Resource &res);
    //void renameNode(const QString &oldCategoryPath, const QString &newCategoryPath);
    //void moveNode(const QString &oldCategoryPath, const QString &newCategoryPath);
    
    void checkResults(const QList<Nepomuk::Query::Result> &);
    void topicRemoved(const QList<QUrl> &);
    void itemsWithTopicAdded(const QList<Nepomuk::Query::Result> &);
    void itemsFromTopicRemoved(const QList<QUrl> &);

    void queryFinished();

private:
    virtual void init();
    virtual TodoNode *createInbox() const;
    void addTopic(const Nepomuk::Resource& topic);

    TodoNode *m_rootNode;
    QMap<QUrl, TodoNode*> m_resourceMap;
    QMap<QUrl, QObject*> m_guardMap;
    QHash<Akonadi::Item::Id, QList <QUrl> > m_itemTopics;
    
};

#endif // TOPICSMODEL_H
