/* This file is part of Zanshin Todo.
 * 
 * Copyright 2012 Christian Mollekopf <chrigi_1@fastmail.fm>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#ifndef NEPOMUKADAPTER_H
#define NEPOMUKADAPTER_H

#include <QObject>
#include <QUrl>
#include <QAbstractItemModel>
#include <akonadi/item.h>
#include <akonadi/entitytreemodel.h>

class QUrl;
namespace Nepomuk {
namespace Types {

class Property;
}

namespace Query {
class Result;
class Query;
}
class Resource;
}

class TopicsModel;

class StructureAdapter: public QObject
{
    Q_OBJECT
public:
    explicit StructureAdapter(QObject* parent = 0);
    virtual void setType(const QUrl &) {};
    
    void setModel(TopicsModel *model);
//     QAbstractItemModel *model() { return m_model; }
    
    virtual QStringList onSourceInsertRow(const QModelIndex &sourceChildIndex) { return QStringList();};
    virtual void onSourceDataChanged(const QModelIndex &changed) {};
    
signals:
    void parentAdded(const QString &identifier, const QString &parentIdentifier, const QString &name);
    void parentChanged(const QString &identifier, const QString &parentIdentifier, const QString &name);
    void parentRemoved(const QString &identifier);
    void itemsAdded(const QString &parentIdentifier, const QModelIndexList &);
    void itemsRemovedFromParent(const QString &parentIdentifier, const QModelIndexList &);
protected:
    TopicsModel *m_model;
};

class TestStructureAdapter : public StructureAdapter
{    
    Q_OBJECT
public:
    
    enum Roles {
        First = Akonadi::EntityTreeModel::TerminalUserRole,
        TopicRole,
        TopicParentRole, //For items and topics
        TopicNameRole
    };
    
    
    explicit TestStructureAdapter(QObject* parent = 0);
    
    void addParent(const QString &identifier, const QString &parentIdentifier, const QString &name);
    void removeParent(const QString &identifier);
//     void addItem(const QString &parentIdentifier, const Akonadi::Item::List &);
    
    virtual QStringList onSourceInsertRow(const QModelIndex &sourceChildIndex);
    virtual void onSourceDataChanged(const QModelIndex &changed);
    
};

class NepomukAdapter : public StructureAdapter
{
    Q_OBJECT
public:
    explicit NepomukAdapter(QObject* parent = 0);
    
    //Set the basic query
    virtual void setType(const QUrl &);
    
private slots:
    void checkResults(const QList<Nepomuk::Query::Result> &);
    void removeResult(const QList<QUrl> &);
    void queryFinished();
    
    void itemsWithTopicAdded(const QList<Nepomuk::Query::Result> &results);
    void itemsFromTopicRemoved(const QList<QUrl> &items);
    void propertyChanged(const Nepomuk::Resource &res, const Nepomuk::Types::Property &property, const QVariant &value);
    
private:
    void addParent (const Nepomuk::Resource& topic);
    
    QMap<QUrl, QObject*> m_guardMap;
    QUrl m_type;
    
};

#endif // NEPOMUKADAPTER_H
