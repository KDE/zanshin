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

#include <KLocalizedString>

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
                                   QObject *parent)
    : PageModel(parent),
      m_context(context),
      m_contextQueries(contextQueries),
      m_contextRepository(contextRepository),
      m_taskQueries(taskQueries),
      m_taskRepository(taskRepository)
{

}

Domain::Context::Ptr ContextPageModel::context() const
{
    return m_context;
}

Domain::Task::Ptr ContextPageModel::addItem(const QString &title, const QModelIndex &parentIndex)
{
    const auto parentData = parentIndex.data(QueryTreeModelBase::ObjectRole);
    const auto parentTask = parentData.value<Domain::Task::Ptr>();

    auto task = Domain::Task::Ptr::create();
    task->setTitle(title);

    const auto job = parentTask ? m_taskRepository->createChild(task, parentTask)
                   : m_taskRepository->createInContext(task, m_context);
    installHandler(job, i18n("Cannot add task %1 in context %2", title, m_context->name()));

    return task;
}

void ContextPageModel::removeItem(const QModelIndex &index)
{
    QVariant data = index.data(QueryTreeModelBase::ObjectRole);
    auto task = data.value<Domain::Task::Ptr>();
    const auto job = index.parent().isValid() ? m_taskRepository->dissociate(task)
                   : m_contextRepository->dissociate(m_context, task);
    installHandler(job, i18n("Cannot remove task %1 from context %2", task->title(), m_context->name()));
}

void ContextPageModel::promoteItem(const QModelIndex &index)
{
    QVariant data = index.data(QueryTreeModelBase::ObjectRole);
    auto task = data.value<Domain::Task::Ptr>();
    Q_ASSERT(task);
    const auto job = m_taskRepository->promoteToProject(task);
    installHandler(job, i18n("Cannot promote task %1 to be a project", task->title()));
}

QAbstractItemModel *ContextPageModel::createCentralListModel()
{
    auto query = [this] (const Domain::Task::Ptr &task) -> Domain::QueryResultInterface<Domain::Task::Ptr>::Ptr {
        if (!task)
            return m_contextQueries->findTopLevelTasks(m_context);
        else
            return m_taskQueries->findChildren(task);
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

    auto data = [](const Domain::Task::Ptr &task, int role, const TaskExtraDataPtr &info) -> QVariant {
        return defaultTaskData(task, role, info);
    };

    auto fetchAdditionalInfo = [this](const QModelIndex &index, const Domain::Task::Ptr &task) {
        return fetchTaskExtraData(m_taskQueries, TaskExtraPart::Project, index, task);
    };

    auto setData = [this] (const Domain::Task::Ptr &task, const QVariant &value, int role) {
        if (role != Qt::EditRole && role != Qt::CheckStateRole)
            return false;

        const auto currentTitle = task->title();
        if (role == Qt::EditRole)
            task->setTitle(value.toString());
        else
            task->setDone(value.toInt() == Qt::Checked);

        const auto job = m_taskRepository->update(task);
        installHandler(job, i18n("Cannot modify task %1 in context %2", currentTitle, m_context->name()));
        return true;
    };

    auto drop = [this] (const QMimeData *mimeData, Qt::DropAction, const Domain::Task::Ptr &parentTask) {
        if (!mimeData->hasFormat(QStringLiteral("application/x-zanshin-object")))
            return false;

        auto droppedTasks = mimeData->property("objects").value<Domain::Task::List>();
        if (droppedTasks.isEmpty())
            return false;

        using namespace std::placeholders;
        auto associate = std::function<KJob*(Domain::Task::Ptr)>();
        auto dissociate = std::function<KJob*(Domain::Task::Ptr)>();
        auto parentTitle = QString();

        if (parentTask) {
            associate = std::bind(&Domain::TaskRepository::associate, m_taskRepository, parentTask, _1);
            dissociate = [] (Domain::Task::Ptr) -> KJob* { return nullptr; };
            parentTitle = parentTask->title();
        } else {
            associate = std::bind(&Domain::ContextRepository::associate, m_contextRepository, m_context, _1);
            dissociate = std::bind(&Domain::TaskRepository::dissociate, m_taskRepository, _1);
            parentTitle = m_context->name();
        }

        foreach(const Domain::Task::Ptr &childTask, droppedTasks) {
            auto job = associate(childTask);
            installHandler(job, i18n("Cannot move task %1 as sub-task of %2", childTask->title(), parentTitle));
            job = dissociate(childTask);
            if (job)
                installHandler(job, i18n("Cannot dissociate task %1 from its parent", childTask->title()));
        }

        return true;
    };

    auto drag = [] (const Domain::Task::List &tasks) -> QMimeData* {
        if (tasks.isEmpty())
            return nullptr;

        auto data = new QMimeData();
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(tasks));
        return data;
    };

    return new QueryTreeModel<Domain::Task::Ptr, TaskExtraDataPtr>(query, flags, data, setData, drop, drag, fetchAdditionalInfo, this);
}
