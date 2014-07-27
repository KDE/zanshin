/* This file is part of Zanshin

   Copyright 2014 Mario Bensi <mbensi@ipsquad.net>

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


#ifndef PRESENTATION_TASKTREEMODEL_H
#define PRESENTATION_TASKTREEMODEL_H

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

class Node;

class TaskTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    typedef Domain::QueryResult<Domain::Task::Ptr> TaskList;

    explicit TaskTreeModel(const std::function<TaskList::Ptr()> &rootQuery,
                           Domain::TaskQueries *queries,
                           Domain::TaskRepository *repository,
                           QObject *parent = 0);
    ~TaskTreeModel();

    Qt::ItemFlags flags(const QModelIndex &index) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

private:
    friend class Node;
    Domain::Task::Ptr taskForIndex(const QModelIndex &index) const;
    bool isModelIndexValid(const QModelIndex &index) const;

    TaskList::Ptr m_taskList;
    Domain::TaskRepository *m_repository;
    Domain::TaskQueries *m_queries;
    QList<Node*> m_rootNodes;
};

}

#endif // PRESENTATION_TASKTREEMODEL_H
