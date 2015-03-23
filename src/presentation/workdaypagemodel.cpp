/* This file is part of Zanshin

   Copyright 2015 Theo Vaucher <theo.vaucher@gmail.com>

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


#include "workdaypagemodel.h"

#include <QMimeData>

#include "domain/noterepository.h"
#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

#include "presentation/querytreemodel.h"

#include "utils/datetime.h"

using namespace Presentation;

WorkdayPageModel::WorkdayPageModel(const Domain::TaskQueries::Ptr &taskQueries,
                                   const Domain::TaskRepository::Ptr &taskRepository,
                                   const Domain::NoteRepository::Ptr &noteRepository,
                                   QObject *parent)
    : PageModel(taskQueries,
                taskRepository,
                noteRepository,
                parent)
{
}

Domain::Task::Ptr WorkdayPageModel::addTask(const QString &title)
{
    auto task = Domain::Task::Ptr::create();
    task->setTitle(title);
    task->setStartDate(Utils::DateTime::currentDateTime());
    const auto job = taskRepository()->create(task);
    installHandler(job, tr("Cannot add task %1 in Workday").arg(title));

    return task;
}

void WorkdayPageModel::removeItem(const QModelIndex &index)
{
    QVariant data = index.data(QueryTreeModel<Domain::Artifact::Ptr>::ObjectRole);
    auto artifact = data.value<Domain::Artifact::Ptr>();
    auto task = artifact.objectCast<Domain::Task>();
    if (task) {
        const auto job = taskRepository()->remove(task);
        installHandler(job, tr("Cannot remove task %1 from Workday").arg(task->title()));
    }
}

QAbstractItemModel *WorkdayPageModel::createCentralListModel()
{
    auto query = [this](const Domain::Artifact::Ptr &artifact) -> Domain::QueryResultInterface<Domain::Artifact::Ptr>::Ptr {
        if (!artifact)
            return Domain::QueryResult<Domain::Task::Ptr, Domain::Artifact::Ptr>::copy(taskQueries()->findWorkdayTopLevel());
        else if (auto task = artifact.dynamicCast<Domain::Task>())
            return Domain::QueryResult<Domain::Task::Ptr, Domain::Artifact::Ptr>::copy(taskQueries()->findChildren(task));
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
        Q_UNUSED(artifact);
        Q_UNUSED(value);
        Q_UNUSED(role);
        qFatal("Not implemented yet");
        return false;
    };

    auto drop = [this](const QMimeData *mimeData, Qt::DropAction, const Domain::Artifact::Ptr &artifact) {
        Q_UNUSED(mimeData);
        Q_UNUSED(artifact);
        qFatal("Not implemented yet");
        return false;
    };

    auto drag = [](const Domain::Artifact::List &artifacts) -> QMimeData* {
        Q_UNUSED(artifacts);
        qFatal("Not implemented yet");
        return Q_NULLPTR;
    };

    return new QueryTreeModel<Domain::Artifact::Ptr>(query, flags, data, setData, drop, drag, this);
}
