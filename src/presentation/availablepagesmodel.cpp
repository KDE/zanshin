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


#include "availablepagesmodel.h"

#include <QIcon>
#include <QMimeData>

#include <KLocalizedString>

#include "domain/contextqueries.h"
#include "domain/contextrepository.h"
#include "domain/projectqueries.h"
#include "domain/projectrepository.h"
#include "domain/taskrepository.h"

#include "presentation/availablepagessortfilterproxymodel.h"
#include "presentation/contextpagemodel.h"
#include "presentation/metatypes.h"
#include "presentation/projectpagemodel.h"
#include "presentation/querytreemodel.h"
#include "presentation/taskinboxpagemodel.h"
#include "presentation/workdaypagemodel.h"

#include "utils/jobhandler.h"
#include "utils/datetime.h"

using namespace Presentation;

AvailablePagesModel::AvailablePagesModel(const Domain::DataSourceQueries::Ptr &dataSourceQueries,
                                                 const Domain::ProjectQueries::Ptr &projectQueries,
                                                 const Domain::ProjectRepository::Ptr &projectRepository,
                                                 const Domain::ContextQueries::Ptr &contextQueries,
                                                 const Domain::ContextRepository::Ptr &contextRepository,
                                                 const Domain::TaskQueries::Ptr &taskQueries,
                                                 const Domain::TaskRepository::Ptr &taskRepository,
                                                 QObject *parent)
    : QObject(parent),
      m_pageListModel(Q_NULLPTR),
      m_sortProxyModel(Q_NULLPTR),
      m_dataSourceQueries(dataSourceQueries),
      m_projectQueries(projectQueries),
      m_projectRepository(projectRepository),
      m_contextQueries(contextQueries),
      m_contextRepository(contextRepository),
      m_taskQueries(taskQueries),
      m_taskRepository(taskRepository)
{
}

QAbstractItemModel *AvailablePagesModel::pageListModel()
{
    if (!m_pageListModel)
        m_pageListModel = createPageListModel();

    if (!m_sortProxyModel) {
        m_sortProxyModel = new AvailablePagesSortFilterProxyModel(this);
        m_sortProxyModel->setSourceModel(m_pageListModel);
    }

    return m_sortProxyModel;
}

QObject *AvailablePagesModel::createPageForIndex(const QModelIndex &index)
{
    QObjectPtr object = index.data(QueryTreeModelBase::ObjectRole).value<QObjectPtr>();

    if (object == m_inboxObject) {
        auto inboxPageModel = new TaskInboxPageModel(m_taskQueries,
                                                     m_taskRepository,
                                                     this);
        inboxPageModel->setErrorHandler(errorHandler());
        return inboxPageModel;
    } else if (object == m_workdayObject) {
        auto workdayPageModel = new WorkdayPageModel(m_taskQueries,
                                                     m_taskRepository,
                                                     this);
        workdayPageModel->setErrorHandler(errorHandler());
        return workdayPageModel;
    } else if (auto project = object.objectCast<Domain::Project>()) {
        auto projectPageModel = new ProjectPageModel(project,
                                                     m_projectQueries,
                                                     m_projectRepository,
                                                     m_taskQueries,
                                                     m_taskRepository,
                                                     this);
        projectPageModel->setErrorHandler(errorHandler());
        return projectPageModel;
    } else if (auto context = object.objectCast<Domain::Context>()) {
        auto contextPageModel = new ContextPageModel(context,
                                                     m_contextQueries,
                                                     m_contextRepository,
                                                     m_taskQueries,
                                                     m_taskRepository,
                                                     this);
        contextPageModel->setErrorHandler(errorHandler());
        return contextPageModel;
    }

    return Q_NULLPTR;
}

void AvailablePagesModel::addProject(const QString &name, const Domain::DataSource::Ptr &source)
{
    auto project = Domain::Project::Ptr::create();
    project->setName(name);
    const auto job = m_projectRepository->create(project, source);
    installHandler(job, i18n("Cannot add project %1 in dataSource %2", name, source->name()));
}

void AvailablePagesModel::addContext(const QString &name)
{
    auto context = Domain::Context::Ptr::create();
    context->setName(name);
    const auto job = m_contextRepository->create(context);
    installHandler(job, i18n("Cannot add context %1", name));
}

void AvailablePagesModel::removeItem(const QModelIndex &index)
{
    QObjectPtr object = index.data(QueryTreeModelBase::ObjectRole).value<QObjectPtr>();
    if (auto project = object.objectCast<Domain::Project>()) {
        const auto job = m_projectRepository->remove(project);
        installHandler(job, i18n("Cannot remove project %1", project->name()));
    } else if (auto context = object.objectCast<Domain::Context>()) {
        const auto job = m_contextRepository->remove(context);
        installHandler(job, i18n("Cannot remove context %1", context->name()));
    } else {
        Q_ASSERT(false);
    }
}

QAbstractItemModel *AvailablePagesModel::createPageListModel()
{
    m_inboxObject = QObjectPtr::create();
    m_inboxObject->setProperty("name", i18n("Inbox"));
    m_workdayObject = QObjectPtr::create();
    m_workdayObject->setProperty("name", i18n("Workday"));
    m_projectsObject = QObjectPtr::create();
    m_projectsObject->setProperty("name", i18n("Projects"));
    m_contextsObject = QObjectPtr::create();
    m_contextsObject->setProperty("name", i18n("Contexts"));

    m_rootsProvider = Domain::QueryResultProvider<QObjectPtr>::Ptr::create();
    m_rootsProvider->append(m_inboxObject);
    m_rootsProvider->append(m_workdayObject);
    m_rootsProvider->append(m_projectsObject);
    m_rootsProvider->append(m_contextsObject);

    auto query = [this](const QObjectPtr &object) -> Domain::QueryResultInterface<QObjectPtr>::Ptr {
        if (!object)
            return Domain::QueryResult<QObjectPtr>::create(m_rootsProvider);
        else if (object == m_projectsObject)
            return Domain::QueryResult<Domain::DataSource::Ptr, QObjectPtr>::copy(m_dataSourceQueries->findAllSelected());
        else if (object == m_contextsObject)
            return Domain::QueryResult<Domain::Context::Ptr, QObjectPtr>::copy(m_contextQueries->findAll());
        else if (const auto source = object.objectCast<Domain::DataSource>())
            return Domain::QueryResult<Domain::Project::Ptr, QObjectPtr>::copy(m_dataSourceQueries->findProjects(source));
        else
            return Domain::QueryResult<QObjectPtr>::Ptr();
    };

    auto flags = [this](const QObjectPtr &object) {
        const Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable
                                         | Qt::ItemIsEnabled
                                         | Qt::ItemIsEditable
                                         | Qt::ItemIsDropEnabled;
        const Qt::ItemFlags immutableNodeFlags = Qt::ItemIsSelectable
                                               | Qt::ItemIsEnabled
                                               | Qt::ItemIsDropEnabled;
        const Qt::ItemFlags structureNodeFlags = Qt::NoItemFlags;

        return object.objectCast<Domain::Project>() ? defaultFlags
             : object.objectCast<Domain::Context>() ? defaultFlags
             : object == m_inboxObject ? immutableNodeFlags
             : object == m_workdayObject ? immutableNodeFlags
             : structureNodeFlags;
    };

    auto data = [this](const QObjectPtr &object, int role, int) -> QVariant {
        if (role != Qt::DisplayRole
         && role != Qt::EditRole
         && role != Qt::DecorationRole
         && role != QueryTreeModelBase::IconNameRole) {
            return QVariant();
        }

        if (role == Qt::EditRole
         && (object == m_inboxObject
          || object == m_workdayObject
          || object == m_projectsObject
          || object == m_contextsObject
          || object.objectCast<Domain::DataSource>())) {
            return QVariant();
        }

        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return object->property("name").toString();
        } else if (role == Qt::DecorationRole || role == QueryTreeModelBase::IconNameRole) {
            const QString iconName = object == m_inboxObject ? QStringLiteral("mail-folder-inbox")
                                   : (object == m_workdayObject)  ? QStringLiteral("go-jump-today")
                                   : (object == m_projectsObject) ? QStringLiteral("folder")
                                   : (object == m_contextsObject) ? QStringLiteral("folder")
                                   : object.objectCast<Domain::DataSource>() ? QStringLiteral("folder")
                                   : object.objectCast<Domain::Context>() ? QStringLiteral("view-pim-notes")
                                   : QStringLiteral("view-pim-tasks");

            if (role == Qt::DecorationRole)
                return QVariant::fromValue(QIcon::fromTheme(iconName));
            else
                return iconName;
        } else {
            return QVariant();
        }
    };

    auto setData = [this](const QObjectPtr &object, const QVariant &value, int role) {
        if (role != Qt::EditRole) {
            return false;
        }

        if (object == m_inboxObject
         || object == m_workdayObject
         || object == m_projectsObject
         || object == m_contextsObject
         || object.objectCast<Domain::DataSource>()) {
            return false;
        }

        if (auto project = object.objectCast<Domain::Project>()) {
            const auto currentName = project->name();
            project->setName(value.toString());
            const auto job = m_projectRepository->update(project);
            installHandler(job, i18n("Cannot modify project %1", currentName));
        } else if (auto context = object.objectCast<Domain::Context>()) {
            const auto currentName = context->name();
            context->setName(value.toString());
            const auto job = m_contextRepository->update(context);
            installHandler(job, i18n("Cannot modify context %1", currentName));
        } else {
            Q_ASSERT(false);
        }

        return true;
    };

    auto drop = [this](const QMimeData *mimeData, Qt::DropAction, const QObjectPtr &object) {
        if (!mimeData->hasFormat(QStringLiteral("application/x-zanshin-object")))
            return false;

        auto droppedTasks = mimeData->property("objects").value<Domain::Task::List>();
        if (droppedTasks.isEmpty())
            return false;

        if (auto project = object.objectCast<Domain::Project>()) {
            foreach (const auto &task, droppedTasks) {
                const auto job = m_projectRepository->associate(project, task);
                installHandler(job, i18n("Cannot add %1 to project %2", task->title(), project->name()));
            }
            return true;
        } else if (auto context = object.objectCast<Domain::Context>()) {
            foreach (const auto &task, droppedTasks) {
                const auto job = m_contextRepository->associate(context, task);
                installHandler(job, i18n("Cannot add %1 to context %2", task->title(), context->name()));
            }
            return true;
        } else if (object == m_inboxObject) {
            foreach (const auto &task, droppedTasks) {
                const auto job = m_projectRepository->dissociate(task);
                installHandler(job, i18n("Cannot move %1 to Inbox", task->title()));

                Utils::JobHandler::install(job, [this, task] {
                    const auto dissociateJob = m_taskRepository->dissociateAll(task);
                    installHandler(dissociateJob, i18n("Cannot move task %1 to Inbox", task->title()));
                });
            }
            return true;
        } else if (object == m_workdayObject) {
            foreach (const auto &task, droppedTasks) {
                task->setStartDate(Utils::DateTime::currentDate());
                const auto job = m_taskRepository->update(task);

                installHandler(job, i18n("Cannot update task %1 to Workday", task->title()));
            }
            return true;
        }

        return false;
    };

    auto drag = [](const QObjectPtrList &) -> QMimeData* {
        return Q_NULLPTR;
    };

    return new QueryTreeModel<QObjectPtr>(query, flags, data, setData, drop, drag, nullptr, this);
}
