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

#include "domain/contextqueries.h"
#include "domain/contextrepository.h"
#include "domain/projectqueries.h"
#include "domain/projectrepository.h"
#include "domain/tag.h"
#include "domain/tagqueries.h"
#include "domain/tagrepository.h"
#include "domain/taskrepository.h"

#include "presentation/contextpagemodel.h"
#include "presentation/inboxpagemodel.h"
#include "presentation/metatypes.h"
#include "presentation/projectpagemodel.h"
#include "presentation/querytreemodel.h"
#include "presentation/tagpagemodel.h"

#include "utils/jobhandler.h"

using namespace Presentation;

AvailablePagesModel::AvailablePagesModel(const Domain::ArtifactQueries::Ptr &artifactQueries,
                                         const Domain::ProjectQueries::Ptr &projectQueries,
                                         const Domain::ProjectRepository::Ptr &projectRepository,
                                         const Domain::ContextQueries::Ptr &contextQueries,
                                         const Domain::ContextRepository::Ptr &contextRepository,
                                         const Domain::TaskQueries::Ptr &taskQueries,
                                         const Domain::TaskRepository::Ptr &taskRepository,
                                         const Domain::NoteRepository::Ptr &noteRepository,
                                         const Domain::TagQueries::Ptr &tagQueries,
                                         const Domain::TagRepository::Ptr &tagRepository,
                                         QObject *parent)
    : QObject(parent),
      m_pageListModel(0),
      m_artifactQueries(artifactQueries),
      m_projectQueries(projectQueries),
      m_projectRepository(projectRepository),
      m_contextQueries(contextQueries),
      m_contextRepository(contextRepository),
      m_taskQueries(taskQueries),
      m_taskRepository(taskRepository),
      m_noteRepository(noteRepository),
      m_tagQueries(tagQueries),
      m_tagRepository(tagRepository)
{
}

QAbstractItemModel *AvailablePagesModel::pageListModel()
{
    if (!m_pageListModel)
        m_pageListModel = createPageListModel();
    return m_pageListModel;
}

QObject *AvailablePagesModel::createPageForIndex(const QModelIndex &index)
{
    QObjectPtr object = index.data(QueryTreeModelBase::ObjectRole).value<QObjectPtr>();

    if (object == m_inboxObject) {
        return new InboxPageModel(m_artifactQueries,
                                  m_taskQueries, m_taskRepository,
                                  m_noteRepository,
                                  this);

    } else if (auto project = object.objectCast<Domain::Project>()) {
        return new ProjectPageModel(project,
                                    m_projectQueries,
                                    m_taskQueries, m_taskRepository,
                                    m_noteRepository,
                                    this);
    } else if (auto context = object.objectCast<Domain::Context>()) {
        return new ContextPageModel(context,
                                    m_contextQueries,
                                    m_taskQueries,
                                    m_taskRepository,
                                    m_noteRepository,
                                    this);
    } else if (auto tag = object.objectCast<Domain::Tag>()) {
        return new TagPageModel(tag,
                                m_tagQueries,
                                m_taskQueries,
                                m_taskRepository,
                                m_noteRepository,
                                this);
    }

    return 0;
}

void AvailablePagesModel::addProject(const QString &name, const Domain::DataSource::Ptr &source)
{
    auto project = Domain::Project::Ptr::create();
    project->setName(name);
    m_projectRepository->create(project, source);
}

void AvailablePagesModel::addContext(const QString &name)
{
    auto context = Domain::Context::Ptr::create();
    context->setName(name);
    m_contextRepository->create(context);
}

void AvailablePagesModel::addTag(const QString &name)
{
    auto tag = Domain::Tag::Ptr::create();
    tag->setName(name);
    m_tagRepository->create(tag);
}

void AvailablePagesModel::removeItem(const QModelIndex &index)
{
    QObjectPtr object = index.data(QueryTreeModelBase::ObjectRole).value<QObjectPtr>();
    if (auto project = object.objectCast<Domain::Project>()) {
        m_projectRepository->remove(project);
    } else if (auto context = object.objectCast<Domain::Context>()) {
        m_contextRepository->remove(context);
    } else if (auto tag = object.objectCast<Domain::Tag>()) {
        m_tagRepository->remove(tag);
    } else {
        Q_ASSERT(false);
    }
}

QAbstractItemModel *AvailablePagesModel::createPageListModel()
{
    m_inboxObject = QObjectPtr::create();
    m_inboxObject->setProperty("name", tr("Inbox"));
    m_projectsObject = QObjectPtr::create();
    m_projectsObject->setProperty("name", tr("Projects"));
    m_contextsObject = QObjectPtr::create();
    m_contextsObject->setProperty("name", tr("Contexts"));
    m_tagsObject = QObjectPtr::create();
    m_tagsObject->setProperty("name", tr("Tags"));

    m_rootsProvider = Domain::QueryResultProvider<QObjectPtr>::Ptr::create();
    m_rootsProvider->append(m_inboxObject);
    m_rootsProvider->append(m_projectsObject);
    m_rootsProvider->append(m_contextsObject);
    m_rootsProvider->append(m_tagsObject);

    auto query = [this](const QObjectPtr &object) -> Domain::QueryResultInterface<QObjectPtr>::Ptr {
        if (!object)
            return Domain::QueryResult<QObjectPtr>::create(m_rootsProvider);
        else if (object == m_projectsObject)
            return Domain::QueryResult<Domain::Project::Ptr, QObjectPtr>::copy(m_projectQueries->findAll());
        else if (object == m_contextsObject)
            return Domain::QueryResult<Domain::Context::Ptr, QObjectPtr>::copy(m_contextQueries->findAll());
        else if (object == m_tagsObject)
            return Domain::QueryResult<Domain::Tag::Ptr, QObjectPtr>::copy(m_tagQueries->findAll());
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
             : object.objectCast<Domain::Tag>() ? defaultFlags
             : object == m_inboxObject ? immutableNodeFlags
             : structureNodeFlags;
    };

    auto data = [this](const QObjectPtr &object, int role) -> QVariant {
        if (role != Qt::DisplayRole
         && role != Qt::EditRole
         && role != Qt::DecorationRole
         && role != QueryTreeModelBase::IconNameRole) {
            return QVariant();
        }

        if (role == Qt::EditRole
         && (object == m_inboxObject
          || object == m_projectsObject
          || object == m_contextsObject
          || object == m_tagsObject)) {
            return QVariant();
        }

        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return object->property("name").toString();
        } else if (role == Qt::DecorationRole || role == QueryTreeModelBase::IconNameRole) {
            const QString iconName = object == m_inboxObject ? "mail-folder-inbox"
                                   : (object == m_projectsObject) ? "folder"
                                   : (object == m_contextsObject) ? "folder"
                                   : (object == m_tagsObject)     ? "folder"
                                   : "view-pim-tasks";

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
         || object == m_projectsObject
         || object == m_contextsObject
         || object == m_tagsObject) {
            return false;
        }

        if (auto project = object.objectCast<Domain::Project>()) {
            project->setName(value.toString());
            m_projectRepository->update(project);
        } else if (auto context = object.objectCast<Domain::Context>()) {
            context->setName(value.toString());
            m_contextRepository->update(context);
        } else if (object.objectCast<Domain::Tag>()) {
            return false; // Tag renaming is NOT allowed
        } else {
            Q_ASSERT(false);
        }

        return true;
    };

    auto drop = [this](const QMimeData *mimeData, Qt::DropAction, const QObjectPtr &object) {
        if (!mimeData->hasFormat("application/x-zanshin-object"))
            return false;

        auto droppedArtifacts = mimeData->property("objects").value<Domain::Artifact::List>();
        if (droppedArtifacts.isEmpty())
            return false;

        if (auto project = object.objectCast<Domain::Project>()) {
            foreach (const auto &droppedArtifact, droppedArtifacts) {
                m_projectRepository->associate(project, droppedArtifact);
            }
            return true;
        } else if (auto context = object.objectCast<Domain::Context>()) {
            if (std::any_of(droppedArtifacts.begin(), droppedArtifacts.end(),
                            [](const Domain::Artifact::Ptr &droppedArtifact) {
                                return !droppedArtifact.objectCast<Domain::Task>();
                            })) {
                return false;
            }
            foreach (const auto &droppedArtifact, droppedArtifacts) {
                auto task = droppedArtifact.staticCast<Domain::Task>();
                m_contextRepository->associate(context, task);
            }
            return true;
        } else if (auto tag = object.objectCast<Domain::Tag>()) {
            foreach (const auto &droppedArtifact, droppedArtifacts) {
                m_tagRepository->associate(tag, droppedArtifact);
            }
            return true;
        } else if (object == m_inboxObject) {
            foreach (const auto &droppedArtifact, droppedArtifacts) {
                auto job = m_projectRepository->dissociate(droppedArtifact);
                if (auto task = droppedArtifact.objectCast<Domain::Task>()) {
                    Utils::JobHandler::install(job, [this, task] {
                        m_taskRepository->dissociate(task);
                    });
                }
            }
            return true;
        }

        return false;
    };

    auto drag = [](const QObjectPtrList &) -> QMimeData* {
        return 0;
    };

    return new QueryTreeModel<QObjectPtr>(query, flags, data, setData, drop, drag, this);
}
