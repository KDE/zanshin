/*
 * SPDX-FileCopyrightText: 2015 Theo Vaucher <theo.vaucher@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "workdaypagemodel.h"

#include <QMimeData>

#include <KLocalizedString>

#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

#include "presentation/querytreemodel.h"

#include "utils/datetime.h"

using namespace Presentation;

WorkdayPageModel::WorkdayPageModel(const Domain::TaskQueries::Ptr &taskQueries,
                                   const Domain::TaskRepository::Ptr &taskRepository,
                                   QObject *parent)
    : PageModel(parent),
      m_taskQueries(taskQueries),
      m_taskRepository(taskRepository)
{
}

Domain::Task::Ptr WorkdayPageModel::addItem(const QString &title, const QModelIndex &parentIndex)
{
    const auto parentData = parentIndex.data(QueryTreeModelBase::ObjectRole);
    const auto parentTask = parentData.value<Domain::Task::Ptr>();

    auto task = Domain::Task::Ptr::create();
    task->setTitle(title);
    if (!parentTask)
        task->setStartDate(Utils::DateTime::currentDate());
    const auto job = parentTask ? m_taskRepository->createChild(task, parentTask)
                   : m_taskRepository->create(task);
    installHandler(job, i18n("Cannot add task %1 in Workday", title));

    return task;
}

void WorkdayPageModel::removeItem(const QModelIndex &index)
{
    QVariant data = index.data(QueryTreeModelBase::ObjectRole);
    auto task = data.value<Domain::Task::Ptr>();
    if (task) {
        const auto job = m_taskRepository->remove(task);
        installHandler(job, i18n("Cannot remove task %1 from Workday", task->title()));
    }
}

void WorkdayPageModel::promoteItem(const QModelIndex &index)
{
    QVariant data = index.data(QueryTreeModelBase::ObjectRole);
    auto task = data.value<Domain::Task::Ptr>();
    Q_ASSERT(task);
    const auto job = m_taskRepository->promoteToProject(task);
    installHandler(job, i18n("Cannot promote task %1 to be a project", task->title()));
}

QAbstractItemModel *WorkdayPageModel::createCentralListModel()
{
    auto query = [this](const Domain::Task::Ptr &task) -> Domain::QueryResultInterface<Domain::Task::Ptr>::Ptr {
        if (!task)
            return m_taskQueries->findWorkdayTopLevel();
        else
            return m_taskQueries->findChildren(task);
    };

    auto flags = [](const Domain::Task::Ptr &task) {
        const Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable
                                         | Qt::ItemIsEnabled
                                         | Qt::ItemIsEditable
                                         | Qt::ItemIsDragEnabled;

        return task ? (defaultFlags | Qt::ItemIsUserCheckable | Qt::ItemIsDropEnabled) : defaultFlags;
    };

    auto data = [](const Domain::Task::Ptr &task, int role, const TaskExtraDataPtr &info) {
        return defaultTaskData(task, role, info);
    };

    auto fetchAdditionalInfo = [this](const QModelIndex &index, const Domain::Task::Ptr &task) {
        return fetchTaskExtraData(m_taskQueries, TaskExtraPart::Project, index, task);
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
        installHandler(job, i18n("Cannot modify task %1 in Workday", currentTitle));
        return true;
    };

    auto drop = [this](const QMimeData *mimeData, Qt::DropAction, const Domain::Task::Ptr &parentTask) {
        if (!mimeData->hasFormat(QStringLiteral("application/x-zanshin-object")))
            return false;

        auto droppedTasks = mimeData->property("objects").value<Domain::Task::List>();
        if (droppedTasks.isEmpty())
            return false;

        foreach(const auto &childTask, droppedTasks) {
            if (parentTask) {
                const auto job = m_taskRepository->associate(parentTask, childTask);
                installHandler(job, i18n("Cannot move task %1 as sub-task of %2", childTask->title(), parentTask->title()));
            } else {
                childTask->setStartDate(Utils::DateTime::currentDate());
                // TODO something like m_taskRepository->update(childTask) is missing here
                // It was removed in commit c97a99bf because it led to a LLCONFLICT in akonadi (due to dissociate below).
                // The removal broke tests-features-workday-workdaydraganddropfeature (date not changed).

                auto job = m_taskRepository->dissociate(childTask);
                installHandler(job, i18n("Cannot deparent task %1 from its parent", childTask->title()));
            }
        }

        return true;
    };

    auto drag = [](const Domain::Task::List &tasks) -> QMimeData* {
        if (tasks.isEmpty())
            return nullptr;

        auto data = new QMimeData;
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(tasks));
        return data;
    };

    return new QueryTreeModel<Domain::Task::Ptr, TaskExtraDataPtr>(query, flags, data, setData, drop, drag, fetchAdditionalInfo, this);
}

#include "moc_workdaypagemodel.cpp"
