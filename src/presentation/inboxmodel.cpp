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


#include "inboxmodel.h"

#include "domain/noterepository.h"
#include "domain/taskrepository.h"
#include "utils/dependencymanager.h"

using namespace Presentation;

InboxModel::InboxModel(Domain::DataSourceQueries *sourceQueries,
                       Domain::TaskRepository *taskRepository,
                       Domain::NoteRepository *noteRepository,
                       QObject *parent)
    : QObject(parent),
      m_sourceQueries(sourceQueries),
      m_taskRepository(taskRepository),
      m_taskSources(m_sourceQueries->findTasks()),
      m_noteRepository(noteRepository),
      m_noteSources(m_sourceQueries->findNotes())
{
    qRegisterMetaType<Domain::DataSource::Ptr>();
}

InboxModel::~InboxModel()
{
}

Domain::DataSource::Ptr InboxModel::defaultNoteDataSource() const
{
    QList<Domain::DataSource::Ptr> sources = m_noteSources->data();

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

Domain::DataSource::Ptr InboxModel::defaultTaskDataSource() const
{
    QList<Domain::DataSource::Ptr> sources = m_taskSources->data();

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

void InboxModel::setDefaultNoteDataSource(Domain::DataSource::Ptr source)
{
    m_noteRepository->setDefaultSource(source);
}

void InboxModel::setDefaultTaskDataSource(Domain::DataSource::Ptr source)
{
    m_taskRepository->setDefaultSource(source);
}
