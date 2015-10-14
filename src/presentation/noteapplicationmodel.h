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


#ifndef PRESENTATION_NOTEAPPLICATIONMODEL_H
#define PRESENTATION_NOTEAPPLICATIONMODEL_H

#include "presentation/applicationmodel.h"

#include "domain/notequeries.h"

namespace Presentation {

class NoteApplicationModel : public ApplicationModel
{
    Q_OBJECT
public:
    explicit NoteApplicationModel(const Domain::ArtifactQueries::Ptr &artifactQueries,
                                  const Domain::ProjectQueries::Ptr &projectQueries,
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
                                  QObject *parent = Q_NULLPTR);

private:
    AvailablePagesModelInterface *createAvailablePagesModel() Q_DECL_OVERRIDE;

    Domain::NoteQueries::Ptr m_noteQueries;
};

}

#endif // PRESENTATION_NOTEAPPLICATIONMODEL_H
