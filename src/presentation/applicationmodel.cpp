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

#include "domain/contextqueries.h"
#include "domain/contextrepository.h"
#include "domain/datasourcequeries.h"
#include "domain/projectqueries.h"
#include "domain/projectrepository.h"
#include "domain/noterepository.h"
#include "domain/tagqueries.h"
#include "domain/tagrepository.h"
#include "domain/taskqueries.h"
#include "domain/taskrepository.h"
#include "domain/datasourcerepository.h"

#include "presentation/artifacteditormodel.h"
#include "presentation/availabletaskpagesmodel.h"
#include "presentation/availablesourcesmodel.h"
#include "presentation/datasourcelistmodel.h"
#include "presentation/errorhandler.h"

using namespace Presentation;

ApplicationModel::ApplicationModel(const Domain::ProjectQueries::Ptr &projectQueries,
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
      m_availableSources(Q_NULLPTR),
      m_availablePages(Q_NULLPTR),
      m_currentPage(Q_NULLPTR),
      m_editor(Q_NULLPTR),
      m_dataSourcesModel(Q_NULLPTR),
      m_projectQueries(projectQueries),
      m_projectRepository(projectRepository),
      m_contextQueries(contextQueries),
      m_contextRepository(contextRepository),
      m_sourceQueries(sourceQueries),
      m_sourceRepository(sourceRepository),
      m_taskQueries(taskQueries),
      m_taskRepository(taskRepository),
      m_noteRepository(noteRepository),
      m_tagQueries(tagQueries),
      m_tagRepository(tagRepository),
      m_errorHandler(Q_NULLPTR)
{
    MetaTypes::registerAll();
}

QAbstractItemModel *ApplicationModel::dataSourcesModel()
{
    if (!m_dataSourcesModel) {
        m_dataSourcesModel = new DataSourceListModel([this] { return dataSources(); }, this);
    }

    return m_dataSourcesModel;
}

Domain::QueryResult<Domain::DataSource::Ptr>::Ptr ApplicationModel::dataSources()
{
    if (!m_dataSources) {
        m_dataSources = createDataSourceQueryResult();
    }

    return m_dataSources;
}

Domain::DataSource::Ptr ApplicationModel::defaultDataSource()
{
    QList<Domain::DataSource::Ptr> sources = dataSources()->data();

    if (sources.isEmpty())
        return Domain::DataSource::Ptr();

    auto source = std::find_if(sources.begin(), sources.end(),
                               [this] (const Domain::DataSource::Ptr &source) {
                                   return isDefaultSource(source);
                               });

    if (source != sources.end())
        return *source;
    else
        return sources.first();
}

QObject *ApplicationModel::availableSources()
{
    if (!m_availableSources) {
        auto model = new AvailableSourcesModel(m_sourceQueries,
                                               m_sourceRepository,
                                               this);
        model->setErrorHandler(errorHandler());
        m_availableSources = model;
    }
    return m_availableSources;
}

QObject *ApplicationModel::availablePages()
{
    if (!m_availablePages) {
        auto model = createAvailablePagesModel();
        model->setErrorHandler(errorHandler());
        m_availablePages = model;
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
        auto model = new ArtifactEditorModel(m_taskRepository, m_noteRepository, this);
        model->setErrorHandler(errorHandler());
        m_editor = model;
    }

    return m_editor;
}

ErrorHandler *ApplicationModel::errorHandler() const
{
    return m_errorHandler;
}

void ApplicationModel::setCurrentPage(QObject *page)
{
    if (page == m_currentPage)
        return;

    m_currentPage = page;
    emit currentPageChanged(page);
}

void ApplicationModel::setErrorHandler(ErrorHandler *errorHandler)
{
    m_errorHandler = errorHandler;
    if (m_availableSources)
        static_cast<AvailableSourcesModel*>(m_availableSources)->setErrorHandler(errorHandler);
    if (m_availablePages)
        static_cast<AvailablePagesModelInterface*>(m_availablePages)->setErrorHandler(errorHandler);
    if (m_editor)
        static_cast<ArtifactEditorModel*>(m_editor)->setErrorHandler(errorHandler);
}
