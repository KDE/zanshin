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
#include "domain/contextrepository.h"
#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

#include "presentation/querytreemodel.h"

using namespace Presentation;

ContextPageModel::ContextPageModel(const Domain::Context::Ptr &context,
                                   const Domain::ContextQueries::Ptr &contextQueries,
                                   const Domain::ContextRepository::Ptr &contextRepository,
                                   const Domain::TaskQueries::Ptr &taskQueries,
                                   const Domain::TaskRepository::Ptr &taskRepository,
                                   const Domain::NoteRepository::Ptr &noteRepository,
                                   QObject *parent)
    :PageModel(taskQueries,
               taskRepository,
               noteRepository,
               parent),
      m_context(context),
      m_contextQueries(contextQueries),
      m_contextRepository(contextRepository)
{

}

Domain::Context::Ptr ContextPageModel::context() const
{
    return m_context;
}

Domain::Artifact::Ptr ContextPageModel::addItem(const QString &title)
{
    auto task = Domain::Task::Ptr::create();
    task->setTitle(title);
    const auto job = taskRepository()->createInContext(task, m_context);
    installHandler(job, tr("Cannot add task %1 in context %2").arg(title).arg(m_context->name()));

    return task;
}

void ContextPageModel::removeItem(const QModelIndex &index)
{
    QVariant data = index.data(QueryTreeModel<Domain::Task::Ptr>::ObjectRole);
    auto task = data.value<Domain::Task::Ptr>();
    const auto job = m_contextRepository->dissociate(m_context, task);
    installHandler(job, tr("Cannot remove task %1 from context %2").arg(task->title()).arg(m_context->name()));
}

QAbstractItemModel *ContextPageModel::createCentralListModel()
{
    auto query = [this] (const Domain::Task::Ptr &task) -> Domain::QueryResultInterface<Domain::Task::Ptr>::Ptr {
        if (!task)
            return m_contextQueries->findTopLevelTasks(m_context); //FIXME : for now returns all tasks associated, not only top level ones
        else
            return Domain::QueryResult<Domain::Task::Ptr>::Ptr();
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

        const auto currentTitle = task->title();
        if (role == Qt::EditRole)
            task->setTitle(value.toString());
        else
            task->setDone(value.toInt() == Qt::Checked);

        const auto job = taskRepository()->update(task);
        installHandler(job, tr("Cannot modify task %1 in context %2").arg(currentTitle).arg(m_context->name()));
        return true;
    };

    auto drop = [this] (const QMimeData *mimeData, Qt::DropAction, const Domain::Task::Ptr &parentTask) {
        if (!mimeData->hasFormat("application/x-zanshin-object"))
            return false;

        auto droppedArtifacts = mimeData->property("objects").value<Domain::Artifact::List>();
        if (droppedArtifacts.isEmpty())
            return false;

        if (std::any_of(droppedArtifacts.begin(), droppedArtifacts.end(),
                        [](const Domain::Artifact::Ptr &droppedArtifact) {
                            return !droppedArtifact.objectCast<Domain::Task>();
                        })) {
            return false;
        }

        foreach(const Domain::Artifact::Ptr &droppedArtifact, droppedArtifacts) {
            auto childTask = droppedArtifact.objectCast<Domain::Task>();
            const auto job = taskRepository()->associate(parentTask, childTask);
            installHandler(job, tr("Cannot move task %1 as sub-task of %2").arg(childTask->title()).arg(parentTask->title()));
        }

        return true;
    };

    auto drag = [] (const Domain::Task::List &tasks) -> QMimeData* {
        if (tasks.isEmpty())
            return Q_NULLPTR;

        auto draggedArtifacts = Domain::Artifact::List();
        foreach (const Domain::Task::Ptr &task, tasks) {
            draggedArtifacts.append(task.objectCast<Domain::Artifact>());
        }

        auto data = new QMimeData();
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(draggedArtifacts));
        return data;
    };

    return new QueryTreeModel<Domain::Task::Ptr>(query, flags, data, setData, drop, drag, this);
}
