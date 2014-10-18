/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
   Copyright 2014 Rémi Benoit <r3m1.benoit@gmail.com>

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

#include "contextpagemodel.h"

#include <QMimeData>

#include "domain/task.h"
#include "domain/contextqueries.h"
#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

#include "presentation/querytreemodel.h"

using namespace Presentation;

ContextPageModel::ContextPageModel(const Domain::Context::Ptr &context,
                                   Domain::ContextQueries *contextQueries,
                                   Domain::TaskQueries *taskQueries,
                                   Domain::TaskRepository *taskRepository,
                                   Domain::NoteRepository *noteRepository,
                                   QObject *parent)
    :PageModel(taskQueries,
               taskRepository,
               noteRepository,
               parent),
      m_context(context),
      m_contextQueries(contextQueries)
{

}

Domain::Context::Ptr ContextPageModel::context() const
{
    return m_context;
}

void ContextPageModel::addTask(const QString &title)
{
    Q_UNUSED(title);
    qFatal("Not implemented yet");
}

void ContextPageModel::removeItem(const QModelIndex &index)
{
    Q_UNUSED(index);
    qFatal("Not implemented yet");
}

QAbstractItemModel *ContextPageModel::createCentralListModel()
{
    auto query = [this] (const Domain::Task::Ptr &task) -> Domain::QueryResultInterface<Domain::Task::Ptr>::Ptr {
        if (!task)
            return m_contextQueries->findTopLevelTasks(m_context);
        else
            return taskQueries()->findChildren(task);
    };

    auto flags = [] (const Domain::Task::Ptr &task) {
        Q_UNUSED(task);
        return Qt::ItemIsSelectable
             | Qt::ItemIsEnabled
             | Qt::ItemIsEditable
             | Qt::ItemIsDragEnabled
             | Qt::ItemIsUserCheckable
             | Qt::ItemIsDropEnabled;
    };

    auto data = [] (const Domain::Task::Ptr &task, int role) -> QVariant {
        if (role != Qt::DisplayRole
         && role != Qt::EditRole
         && role != Qt::CheckStateRole) {
            return QVariant();
        }

        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return task->title();
        } else if (role == Qt::CheckStateRole){
            return task->isDone() ? Qt::Checked : Qt::Unchecked;
        } else {
            return QVariant();
        }
    };

    auto setData = [this] (const Domain::Task::Ptr &task, const QVariant &value, int role) {
        if (role != Qt::EditRole && role != Qt::CheckStateRole)
            return false;

        if (role == Qt::EditRole)
            task->setTitle(value.toString());
        else
            task->setDone(value.toInt() == Qt::Checked);

        taskRepository()->update(task);
        return true;
    };

    auto drop = [this] (const QMimeData *mimeData, Qt::DropAction, const Domain::Task::Ptr &parentTask) {
        Q_UNUSED(mimeData);
        Q_UNUSED(parentTask);
        qFatal("Drop Not implemented yet");
        return true;
    };

    auto drag = [] (const Domain::Task::List &tasks) -> QMimeData* {
        Q_UNUSED(tasks);
        qFatal("Drag Not implemented yet");
        return new QMimeData;
    };

    return new QueryTreeModel<Domain::Task::Ptr>(query, flags, data, setData, drop, drag, this);
}
