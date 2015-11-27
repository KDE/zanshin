/* This file is part of Zanshin

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


#include "projectpagemodel.h"

#include <QMimeData>

#include "presentation/querytreemodel.h"

using namespace Presentation;

ProjectPageModel::ProjectPageModel(const Domain::Project::Ptr &project,
                                   const Domain::ProjectQueries::Ptr &projectQueries,
                                   const Domain::TaskQueries::Ptr &taskQueries,
                                   const Domain::TaskRepository::Ptr &taskRepository,
                                   QObject *parent)
    : PageModel(parent),
      m_projectQueries(projectQueries),
      m_project(project),
      m_taskQueries(taskQueries),
      m_taskRepository(taskRepository)
{
}

Domain::Project::Ptr ProjectPageModel::project() const
{
    return m_project;
}

Domain::Artifact::Ptr ProjectPageModel::addItem(const QString &title, const QModelIndex &parentIndex)
{
    const auto parentData = parentIndex.data(QueryTreeModel<Domain::Task::Ptr>::ObjectRole);
    const auto parentArtifact = parentData.value<Domain::Artifact::Ptr>();
    const auto parentTask = parentArtifact.objectCast<Domain::Task>();

    auto task = Domain::Task::Ptr::create();
    task->setTitle(title);

    const auto job = parentTask ? m_taskRepository->createChild(task, parentTask)
                   : m_taskRepository->createInProject(task, m_project);
    installHandler(job, tr("Cannot add task %1 in project %2").arg(title).arg(m_project->name()));

    return task;
}

void ProjectPageModel::removeItem(const QModelIndex &index)
{
    QVariant data = index.data(QueryTreeModel<Domain::Artifact::Ptr>::ObjectRole);
    auto artifact = data.value<Domain::Artifact::Ptr>();
    auto task = artifact.objectCast<Domain::Task>();
    Q_ASSERT(task);
    const auto job = m_taskRepository->remove(task);
    installHandler(job, tr("Cannot remove task %1 from project %2").arg(task->title()).arg(m_project->name()));
}

void ProjectPageModel::promoteItem(const QModelIndex &index)
{
    QVariant data = index.data(QueryTreeModel<Domain::Task::Ptr>::ObjectRole);
    auto artifact = data.value<Domain::Artifact::Ptr>();
    auto task = artifact.objectCast<Domain::Task>();
    Q_ASSERT(task);
    const auto job = m_taskRepository->promoteToProject(task);
    installHandler(job, tr("Cannot promote task %1 to be a project").arg(task->title()));
}

QAbstractItemModel *ProjectPageModel::createCentralListModel()
{
    auto query = [this](const Domain::Task::Ptr &task) -> Domain::QueryResultInterface<Domain::Task::Ptr>::Ptr {
        if (!task)
            return m_projectQueries->findTopLevel(m_project);
        else
            return m_taskQueries->findChildren(task);
    };

    auto flags = [](const Domain::Task::Ptr &) {
        return Qt::ItemIsSelectable
             | Qt::ItemIsEnabled
             | Qt::ItemIsEditable
             | Qt::ItemIsDragEnabled
             | Qt::ItemIsUserCheckable
             | Qt::ItemIsDropEnabled;
    };

    auto data = [](const Domain::Task::Ptr &task, int role) -> QVariant {
        if (role != Qt::DisplayRole
         && role != Qt::EditRole
         && role != Qt::CheckStateRole) {
            return QVariant();
        }

        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return task->title();
        } else {
            return task->isDone() ? Qt::Checked : Qt::Unchecked;
        }
    };

    auto setData = [this](const Domain::Task::Ptr &task, const QVariant &value, int role) {
        if (role != Qt::EditRole && role != Qt::CheckStateRole) {
            return false;
        }

        const auto currentTitle = task->title();
        if (role == Qt::EditRole)
            task->setTitle(value.toString());
        else
            task->setDone(value.toInt() == Qt::Checked);

        const auto job = m_taskRepository->update(task);
        installHandler(job, tr("Cannot modify task %1 in project %2").arg(currentTitle).arg(m_project->name()));
        return true;
    };

    auto drop = [this](const QMimeData *mimeData, Qt::DropAction, const Domain::Task::Ptr &parentTask) {
        if (!parentTask)
            return false;

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
            const auto job = m_taskRepository->associate(parentTask, childTask);
            installHandler(job, tr("Cannot move task %1 as a sub-task of %2").arg(childTask->title()).arg(parentTask->title()));
        }

        return true;
    };

    auto drag = [](const Domain::Task::List &tasks) -> QMimeData* {
        if (tasks.isEmpty())
            return Q_NULLPTR;

        auto draggedArtifacts = Domain::Artifact::List();
        foreach (const Domain::Task::Ptr &task, tasks) {
            draggedArtifacts.append(task.objectCast<Domain::Artifact>());
        }

        auto data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(draggedArtifacts));
        return data;
    };

    return new QueryTreeModel<Domain::Task::Ptr>(query, flags, data, setData, drop, drag, this);
}
