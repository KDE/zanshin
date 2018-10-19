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

#include <KLocalizedString>

#include "presentation/querytreemodel.h"

using namespace Presentation;

ProjectPageModel::ProjectPageModel(const Domain::Project::Ptr &project,
                                   const Domain::ProjectQueries::Ptr &projectQueries,
                                   const Domain::ProjectRepository::Ptr &projectRepository,
                                   const Domain::TaskQueries::Ptr &taskQueries,
                                   const Domain::TaskRepository::Ptr &taskRepository,
                                   QObject *parent)
    : PageModel(parent),
      m_projectQueries(projectQueries),
      m_projectRepository(projectRepository),
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
    const auto parentData = parentIndex.data(QueryTreeModelBase::ObjectRole);
    const auto parentArtifact = parentData.value<Domain::Artifact::Ptr>();
    const auto parentTask = parentArtifact.objectCast<Domain::Task>();

    auto task = Domain::Task::Ptr::create();
    task->setTitle(title);

    const auto job = parentTask ? m_taskRepository->createChild(task, parentTask)
                   : m_taskRepository->createInProject(task, m_project);
    installHandler(job, i18n("Cannot add task %1 in project %2", title, m_project->name()));

    return task;
}

void ProjectPageModel::removeItem(const QModelIndex &index)
{
    QVariant data = index.data(QueryTreeModelBase::ObjectRole);
    auto artifact = data.value<Domain::Artifact::Ptr>();
    auto task = artifact.objectCast<Domain::Task>();
    Q_ASSERT(task);
    const auto job = m_taskRepository->remove(task);
    installHandler(job, i18n("Cannot remove task %1 from project %2", task->title(), m_project->name()));
}

void ProjectPageModel::promoteItem(const QModelIndex &index)
{
    QVariant data = index.data(QueryTreeModelBase::ObjectRole);
    auto artifact = data.value<Domain::Artifact::Ptr>();
    auto task = artifact.objectCast<Domain::Task>();
    Q_ASSERT(task);
    const auto job = m_taskRepository->promoteToProject(task);
    installHandler(job, i18n("Cannot promote task %1 to be a project", task->title()));
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

    auto data = [](const Domain::Task::Ptr &task, int role, int) -> QVariant {
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
        installHandler(job, i18n("Cannot modify task %1 in project %2", currentTitle, m_project->name()));
        return true;
    };

    auto drop = [this](const QMimeData *mimeData, Qt::DropAction, const Domain::Task::Ptr &parentTask) {
        if (!mimeData->hasFormat(QStringLiteral("application/x-zanshin-object")))
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

        using namespace std::placeholders;
        auto associate = std::function<KJob*(Domain::Task::Ptr)>();
        auto parentTitle = QString();

        if (parentTask) {
            associate = std::bind(&Domain::TaskRepository::associate, m_taskRepository, parentTask, _1);
            parentTitle = parentTask->title();
        } else {
            associate = std::bind(&Domain::ProjectRepository::associate, m_projectRepository, m_project, _1);
            parentTitle = m_project->name();
        }

        foreach(const Domain::Artifact::Ptr &droppedArtifact, droppedArtifacts) {
            auto childTask = droppedArtifact.objectCast<Domain::Task>();
            const auto job = associate(childTask);
            installHandler(job, i18n("Cannot move task %1 as a sub-task of %2", childTask->title(), parentTitle));
        }

        return true;
    };

    auto drag = [](const Domain::Task::List &tasks) -> QMimeData* {
        if (tasks.isEmpty())
            return Q_NULLPTR;

        auto draggedArtifacts = Domain::Artifact::List();
        draggedArtifacts.reserve(tasks.size());
        foreach (const Domain::Task::Ptr &task, tasks) {
            draggedArtifacts.append(task.objectCast<Domain::Artifact>());
        }

        auto data = new QMimeData;
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(draggedArtifacts));
        return data;
    };

    return new QueryTreeModel<Domain::Task::Ptr>(query, flags, data, setData, drop, drag, nullptr, this);
}
