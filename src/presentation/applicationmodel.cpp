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


#include "applicationmodel.h"

#include "domain/artifactqueries.h"
#include "domain/contextqueries.h"
#include "domain/contextrepository.h"
#include "domain/projectqueries.h"
#include "domain/projectrepository.h"
#include "domain/noterepository.h"
#include "domain/tagqueries.h"
#include "domain/tagrepository.h"
#include "domain/taskqueries.h"
#include "domain/taskrepository.h"
#include "domain/datasourcerepository.h"

#include "presentation/artifacteditormodel.h"
#include "presentation/availablepagesmodel.h"
#include "presentation/availablesourcesmodel.h"
#include "presentation/datasourcelistmodel.h"

using namespace Presentation;

ApplicationModel::ApplicationModel(const Domain::ArtifactQueries::Ptr &artifactQueries,
                                   const Domain::ProjectQueries::Ptr &projectQueries,
                                   const Domain::ProjectRepository::Ptr &projectRepository,
                                   const Domain::ContextQueries::Ptr &contextQueries,
                                   const Domain::ContextRepository::Ptr &contextRepository,
                                   const Domain::DataSourceQueries::Ptr &sourceQueries,
                                   const Domain::DataSourceRepository::Ptr &sourceRepository,
                                   const Domain::TaskQueries::Ptr &taskQueries,
                                   const Domain::TaskRepository::Ptr &taskRepository,
                                   const Domain::NoteRepository::Ptr &noteRepository,
                                   const Domain::TagQueries::Ptr &tagQueries,
                                   const Domain::TagRepository::Ptr &tagRepository,
                                   QObject *parent)
    : QObject(parent),
      m_availableSources(0),
      m_availablePages(0),
      m_currentPage(0),
      m_editor(0),
      m_artifactQueries(artifactQueries),
      m_projectQueries(projectQueries),
      m_projectRepository(projectRepository),
      m_contextQueries(contextQueries),
      m_contextRepository(contextRepository),
      m_sourceQueries(sourceQueries),
      m_sourceRepository(sourceRepository),
      m_taskQueries(taskQueries),
      m_taskRepository(taskRepository),
      m_taskSourcesModel(0),
      m_noteRepository(noteRepository),
      m_noteSourcesModel(0),
      m_tagQueries(tagQueries),
      m_tagRepository(tagRepository)
{
    MetaTypes::registerAll();
}

QAbstractItemModel *ApplicationModel::noteSourcesModel()
{
    if (!m_noteSourcesModel) {
        m_noteSourcesModel = new DataSourceListModel([this] { return noteSources(); }, this);
    }

    return m_noteSourcesModel;
}

Domain::QueryResult<Domain::DataSource::Ptr>::Ptr ApplicationModel::noteSources()
{
    if (!m_noteSources) {
        m_noteSources = m_sourceQueries->findNotes();
    }

    return m_noteSources;
}

Domain::DataSource::Ptr ApplicationModel::defaultNoteDataSource()
{
    QList<Domain::DataSource::Ptr> sources = noteSources()->data();

    if (sources.isEmpty())
        return Domain::DataSource::Ptr();

    auto source = std::find_if(sources.begin(), sources.end(),
                               [this] (const Domain::DataSource::Ptr &source) {
                                   return m_noteRepository->isDefaultSource(source);
                               });

    if (source != sources.end())
        return *source;
    else
        return sources.first();
}

QAbstractItemModel *ApplicationModel::taskSourcesModel()
{
    if (!m_taskSourcesModel) {
        m_taskSourcesModel = new DataSourceListModel([this] { return taskSources(); }, this);
    }

    return m_taskSourcesModel;
}

Domain::QueryResult<Domain::DataSource::Ptr>::Ptr ApplicationModel::taskSources()
{
    if (!m_taskSources) {
        m_taskSources = m_sourceQueries->findTasks();
    }

    return m_taskSources;
}

Domain::DataSource::Ptr ApplicationModel::defaultTaskDataSource()
{
    QList<Domain::DataSource::Ptr> sources = taskSources()->data();

    if (sources.isEmpty())
        return Domain::DataSource::Ptr();

    auto source = std::find_if(sources.begin(), sources.end(),
                               [this] (const Domain::DataSource::Ptr &source) {
                                   return m_taskRepository->isDefaultSource(source);
                               });

    if (source != sources.end())
        return *source;
    else
        return sources.first();
}

QObject *ApplicationModel::availableSources()
{
    if (!m_availableSources) {
        m_availableSources = new AvailableSourcesModel(m_sourceQueries,
                                                       m_sourceRepository,
                                                       this);
    }
    return m_availableSources;
}

QObject *ApplicationModel::availablePages()
{
    if (!m_availablePages) {
        m_availablePages = new AvailablePagesModel(m_artifactQueries,
                                                   m_projectQueries,
                                                   m_projectRepository,
                                                   m_contextQueries,
                                                   m_contextRepository,
                                                   m_taskQueries,
                                                   m_taskRepository,
                                                   m_noteRepository,
                                                   m_tagQueries,
                                                   m_tagRepository,
                                                   this);
    }
    return m_availablePages;
}

QObject *ApplicationModel::currentPage()
{
    return m_currentPage;
}

QObject *ApplicationModel::editor()
{
    if (!m_editor) {
        m_editor = new ArtifactEditorModel(m_taskRepository, m_noteRepository, this);
    }

    return m_editor;
}

void ApplicationModel::setCurrentPage(QObject *page)
{
    if (page == m_currentPage)
        return;

    m_currentPage = page;
    emit currentPageChanged(page);
}

void ApplicationModel::setDefaultNoteDataSource(Domain::DataSource::Ptr source)
{
    m_noteRepository->setDefaultSource(source);
}

void ApplicationModel::setDefaultTaskDataSource(Domain::DataSource::Ptr source)
{
    m_taskRepository->setDefaultSource(source);
}
