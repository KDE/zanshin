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

#include <QtTest>

#include "presentation/availablenotepagesmodel.h"
#include "presentation/noteapplicationmodel.h"

class NoteApplicationModelTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldProvideAvailableNotePagesModel()
    {
        // GIVEN
        auto projectQueries = Domain::ProjectQueries::Ptr();
        auto projectRepository = Domain::ProjectRepository::Ptr();
        auto contextQueries = Domain::ContextQueries::Ptr();
        auto contextRepository = Domain::ContextRepository::Ptr();
        auto sourceQueries = Domain::DataSourceQueries::Ptr();
        auto sourceRepository = Domain::DataSourceRepository::Ptr();
        auto taskQueries = Domain::TaskQueries::Ptr();
        auto taskRepository = Domain::TaskRepository::Ptr();
        auto noteQueries = Domain::NoteQueries::Ptr();
        auto noteRepository = Domain::NoteRepository::Ptr();
        auto tagQueries = Domain::TagQueries::Ptr();
        auto tagRepository = Domain::TagRepository::Ptr();
        Presentation::NoteApplicationModel app(projectQueries,
                                               projectRepository,
                                               contextQueries,
                                               contextRepository,
                                               sourceQueries,
                                               sourceRepository,
                                               taskQueries,
                                               taskRepository,
                                               noteQueries,
                                               noteRepository,
                                               tagQueries,
                                               tagRepository);

        // WHEN
        QObject *available = app.availablePages();

        // THEN
        QVERIFY(qobject_cast<Presentation::AvailableNotePagesModel*>(available));
    }
};

QTEST_MAIN(NoteApplicationModelTest)

#include "noteapplicationmodeltest.moc"
