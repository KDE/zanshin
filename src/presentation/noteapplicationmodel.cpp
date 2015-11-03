/* This file is part of Zanshin

   Copyright 2015 Kevin Ottens <ervin@kde.org>

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


#include "noteapplicationmodel.h"

#include "presentation/availablenotepagesmodel.h"

using namespace Presentation;

NoteApplicationModel::NoteApplicationModel(const Domain::ProjectQueries::Ptr &projectQueries,
                                           const Domain::ProjectRepository::Ptr &projectRepository,
                                           const Domain::ContextQueries::Ptr &contextQueries,
                                           const Domain::ContextRepository::Ptr &contextRepository,
                                           const Domain::DataSourceQueries::Ptr &sourceQueries,
                                           const Domain::DataSourceRepository::Ptr &sourceRepository,
                                           const Domain::TaskQueries::Ptr &taskQueries,
                                           const Domain::TaskRepository::Ptr &taskRepository,
                                           const Domain::NoteQueries::Ptr &noteQueries,
                                           const Domain::NoteRepository::Ptr &noteRepository,
                                           const Domain::TagQueries::Ptr &tagQueries,
                                           const Domain::TagRepository::Ptr &tagRepository,
                                           QObject *parent)
    : ApplicationModel(projectQueries,
                       projectRepository,
                       contextQueries,
                       contextRepository,
                       sourceQueries,
                       sourceRepository,
                       taskQueries,
                       taskRepository,
                       noteRepository,
                       tagQueries,
                       tagRepository,
                       parent),
      m_noteQueries(noteQueries)
{
}

void NoteApplicationModel::setDefaultDataSource(const Domain::DataSource::Ptr &source)
{
    m_noteRepository->setDefaultSource(source);
}

Domain::QueryResult<Domain::DataSource::Ptr>::Ptr NoteApplicationModel::createDataSourceQueryResult()
{
    return m_sourceQueries->findNotes();
}

bool NoteApplicationModel::isDefaultSource(const Domain::DataSource::Ptr &source)
{
    return m_noteRepository->isDefaultSource(source);
}

AvailablePagesModelInterface *NoteApplicationModel::createAvailablePagesModel()
{
    return new AvailableNotePagesModel(m_noteQueries,
                                       m_noteRepository,
                                       m_tagQueries,
                                       m_tagRepository,
                                       this);
}
