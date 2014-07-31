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
#include "domain/noterepository.h"
#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

#include "presentation/datasourcelistmodel.h"
#include "presentation/querytreemodel.h"
#include "presentation/inboxpagemodel.h"

#include "utils/dependencymanager.h"

using namespace Presentation;

ApplicationModel::ApplicationModel(QObject *parent)
    : QObject(parent),
      m_currentPage(0),
      m_artifactQueries(Utils::DependencyManager::globalInstance().create<Domain::ArtifactQueries>()),
      m_sourceQueries(Utils::DependencyManager::globalInstance().create<Domain::DataSourceQueries>()),
      m_taskQueries(Utils::DependencyManager::globalInstance().create<Domain::TaskQueries>()),
      m_taskRepository(Utils::DependencyManager::globalInstance().create<Domain::TaskRepository>()),
      m_taskSourcesModel(0),
      m_noteRepository(Utils::DependencyManager::globalInstance().create<Domain::NoteRepository>()),
      m_noteSourcesModel(0),
      m_ownInterface(true)
{
    qRegisterMetaType<QAbstractItemModel*>();
    qRegisterMetaType<Domain::DataSource::Ptr>();
}

ApplicationModel::ApplicationModel(Domain::ArtifactQueries *artifactQueries,
                       Domain::DataSourceQueries *sourceQueries,
                       Domain::TaskQueries *taskQueries,
                       Domain::TaskRepository *taskRepository,
                       Domain::NoteRepository *noteRepository,
                       QObject *parent)
    : QObject(parent),
      m_currentPage(0),
      m_artifactQueries(artifactQueries),
      m_sourceQueries(sourceQueries),
      m_taskQueries(taskQueries),
      m_taskRepository(taskRepository),
      m_taskSourcesModel(0),
      m_noteRepository(noteRepository),
      m_noteSourcesModel(0),
      m_ownInterface(false)
{
    qRegisterMetaType<QAbstractItemModel*>();
    qRegisterMetaType<Domain::DataSource::Ptr>();
}

ApplicationModel::~ApplicationModel()
{
    if (m_ownInterface) {
        delete m_artifactQueries;
        delete m_sourceQueries;
        delete m_taskQueries;
        delete m_taskRepository;
        delete m_noteRepository;
    }
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

QObject *ApplicationModel::currentPage()
{
    if (!m_currentPage) {
        m_currentPage = new InboxPageModel(m_artifactQueries, m_taskQueries,
                                           m_taskRepository, m_noteRepository, this);
    }

    return m_currentPage;
}

void ApplicationModel::setDefaultNoteDataSource(Domain::DataSource::Ptr source)
{
    m_noteRepository->setDefaultSource(source);
}

void ApplicationModel::setDefaultTaskDataSource(Domain::DataSource::Ptr source)
{
    m_taskRepository->setDefaultSource(source);
}
