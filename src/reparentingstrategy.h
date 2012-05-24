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


#ifndef REPARENTINGSTRATEGY_H
#define REPARENTINGSTRATEGY_H
#include <QList>
#include <QModelIndex>
#include <Akonadi/EntityTreeModel>


typedef qint64 Id;
typedef QList<qint64> IdList;

class ReparentingStrategy
{
public:
    /// Get the id for an object
    virtual Id getId(const QModelIndex &/*sourceChildIndex*/) = 0;
    /// Get parents
    virtual IdList onSourceInsertRow(const QModelIndex &/*sourceChildIndex*/);
    //Called whenever the item has changed, to reevaluate the parents
    virtual IdList onSourceDataChanged(const QModelIndex &/*changed*/);
};

class TestReparentingStrategy : public ReparentingStrategy
{
public:

    enum Roles {
        First = Akonadi::EntityTreeModel::TerminalUserRole,
        IdRole,
        ParentRole, //For items and topics
//         NameRole
    };

    explicit TestReparentingStrategy();

//     void addParent(const qint64 &identifier, const qint64 &parentIdentifier, const QString &name);
//     void setParent(const QModelIndex &item, const qint64 &parentIdentifier);
//     void removeParent(const qint64 &identifier);
//     void onNodeRemoval(const qint64 &changed) { qDebug() << "removed node: " << changed; };

    virtual Id getId(const QModelIndex& );
    virtual QList<qint64> onSourceInsertRow(const QModelIndex &sourceChildIndex);
    virtual QList<qint64> onSourceDataChanged(const QModelIndex &changed);

};

#endif // REPARENTINGSTRATEGY_H
