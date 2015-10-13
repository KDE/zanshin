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

#include "domain/noterepository.h"
#include "domain/projectqueries.h"
#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

#include "presentation/querytreemodel.h"

using namespace Presentation;

ProjectPageModel::ProjectPageModel(const Domain::Project::Ptr &project,
                                   const Domain::ProjectQueries::Ptr &projectQueries,
                                   const Domain::TaskQueries::Ptr &taskQueries,
                                   const Domain::TaskRepository::Ptr &taskRepository,
                                   const Domain::NoteRepository::Ptr &noteRepository,
                                   QObject *parent)
    : PageModel(parent),
      m_projectQueries(projectQueries),
      m_project(project),
      m_taskQueries(taskQueries),
      m_taskRepository(taskRepository),
      m_noteRepository(noteRepository)
{
}

Domain::Project::Ptr ProjectPageModel::project() const
{
    return m_project;
}

Domain::Artifact::Ptr ProjectPageModel::addItem(const QString &title)
{
    auto task = Domain::Task::Ptr::create();
    task->setTitle(title);
    const auto job = m_taskRepository->createInProject(task, m_project);
    installHandler(job, tr("Cannot add task %1 in project %2").arg(title).arg(m_project->name()));

    return task;
}

void ProjectPageModel::removeItem(const QModelIndex &index)
{
    QVariant data = index.data(QueryTreeModel<Domain::Artifact::Ptr>::ObjectRole);
    auto artifact = data.value<Domain::Artifact::Ptr>();
    auto task = artifact.objectCast<Domain::Task>();
    if (task) {
        const auto job = m_taskRepository->remove(task);
        installHandler(job, tr("Cannot remove task %1 from project %2").arg(task->title()).arg(m_project->name()));
    }
}

QAbstractItemModel *ProjectPageModel::createCentralListModel()
{
    auto query = [this](const Domain::Artifact::Ptr &artifact) -> Domain::QueryResultInterface<Domain::Artifact::Ptr>::Ptr {
        if (!artifact)
            return m_projectQueries->findTopLevelArtifacts(m_project);
        else if (auto task = artifact.dynamicCast<Domain::Task>())
            return Domain::QueryResult<Domain::Task::Ptr, Domain::Artifact::Ptr>::copy(m_taskQueries->findChildren(task));
        else
            return Domain::QueryResult<Domain::Artifact::Ptr>::Ptr();
    };

    auto flags = [](const Domain::Artifact::Ptr &artifact) {
        const Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable
                                         | Qt::ItemIsEnabled
                                         | Qt::ItemIsEditable
                                         | Qt::ItemIsDragEnabled;

        return artifact.dynamicCast<Domain::Task>() ? (defaultFlags | Qt::ItemIsUserCheckable | Qt::ItemIsDropEnabled) : defaultFlags;
    };

    auto data = [](const Domain::Artifact::Ptr &artifact, int role) -> QVariant {
        if (role != Qt::DisplayRole
         && role != Qt::EditRole
         && role != Qt::CheckStateRole) {
            return QVariant();
        }

        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return artifact->title();
        } else if (auto task = artifact.dynamicCast<Domain::Task>()) {
            return task->isDone() ? Qt::Checked : Qt::Unchecked;
        } else {
            return QVariant();
        }
    };

    auto setData = [this](const Domain::Artifact::Ptr &artifact, const QVariant &value, int role) {
        if (role != Qt::EditRole && role != Qt::CheckStateRole) {
            return false;
        }

        if (auto task = artifact.dynamicCast<Domain::Task>()) {
            const auto currentTitle = task->title();
            if (role == Qt::EditRole)
                task->setTitle(value.toString());
            else
                task->setDone(value.toInt() == Qt::Checked);

            const auto job = m_taskRepository->update(task);
            installHandler(job, tr("Cannot modify task %1 in project %2").arg(currentTitle).arg(m_project->name()));
            return true;

        } else if (auto note = artifact.dynamicCast<Domain::Note>()) {
            if (role != Qt::EditRole)
                return false;

            const auto currentTitle = note->title();
            note->setTitle(value.toString());
            const auto job = m_noteRepository->update(note);
            installHandler(job, tr("Cannot modify note %1 in project %2").arg(currentTitle).arg(m_project->name()));
            return true;

        }

        return false;
    };

    auto drop = [this](const QMimeData *mimeData, Qt::DropAction, const Domain::Artifact::Ptr &artifact) {
        auto parentTask = artifact.objectCast<Domain::Task>();
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

    auto drag = [](const Domain::Artifact::List &artifacts) -> QMimeData* {
        if (artifacts.isEmpty())
            return Q_NULLPTR;

        auto data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(artifacts));
        return data;
    };

    return new QueryTreeModel<Domain::Artifact::Ptr>(query, flags, data, setData, drop, drag, this);
}
