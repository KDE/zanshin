/* This file is part of Zanshin

   Copyright 2014 Mario Bensi <mbensi@ipsquad.net>
   Copyright 2014 Kevin Ottens <ervin@kde.org>

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


#ifndef PRESENTATION_QUERYTREEMODEL_H
#define PRESENTATION_QUERYTREEMODEL_H

#include <functional>

#include <QAbstractItemModel>

#include "domain/queryresult.h"
#include "domain/queryresultprovider.h"
#include "domain/task.h"

namespace Domain
{
    class TaskRepository;
    class TaskQueries;
}

namespace Presentation {

class NodeBase;

class QueryTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    typedef Domain::QueryResult<Domain::Task::Ptr> TaskList;
    typedef std::function<TaskList::Ptr(const Domain::Task::Ptr &)> QueryGenerator;
    typedef std::function<Qt::ItemFlags(const Domain::Task::Ptr &)> FlagsFunction;
    typedef std::function<QVariant(const Domain::Task::Ptr &, int)> DataFunction;
    typedef std::function<bool(const Domain::Task::Ptr &, const QVariant &, int)> SetDataFunction;

    explicit QueryTreeModel(const QueryGenerator &queryGenerator,
                           const FlagsFunction &flagsFunction,
                           const DataFunction &dataFunction,
                           const SetDataFunction &setDataFunction,
                           QObject *parent = 0);
    ~QueryTreeModel();

    Qt::ItemFlags flags(const QModelIndex &index) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

private:
    friend class NodeBase;
    NodeBase *nodeFromIndex(const QModelIndex &index) const;
    bool isModelIndexValid(const QModelIndex &index) const;

    NodeBase *m_rootNode;
};

}

#endif // PRESENTATION_QUERYTREEMODEL_H
