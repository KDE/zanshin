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


#include "pagemodel.h"

using namespace Presentation;

PageModel::PageModel(Domain::ArtifactQueries *artifactQueries,
                     Domain::TaskQueries *taskQueries,
                     Domain::TaskRepository *taskRepository,
                     Domain::NoteRepository *noteRepository,
                     QObject *parent)
    : QObject(parent),
      m_centralListModel(0),
      m_artifactQueries(artifactQueries),
      m_taskQueries(taskQueries),
      m_taskRepository(taskRepository),
      m_noteRepository(noteRepository)
{
}

QAbstractItemModel *PageModel::centralListModel()
{
    if (!m_centralListModel)
        m_centralListModel = createCentralListModel();
    return m_centralListModel;
}

Domain::ArtifactQueries *PageModel::artifactQueries() const
{
    return m_artifactQueries;
}

Domain::TaskQueries *PageModel::taskQueries() const
{
    return m_taskQueries;
}

Domain::TaskRepository *PageModel::taskRepository() const
{
    return m_taskRepository;
}

Domain::NoteRepository *PageModel::noteRepository() const
{
    return m_noteRepository;
}
