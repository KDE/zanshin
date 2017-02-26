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

#include <testlib/qtest_zanshin.h>

#include "utils/dependencymanager.h"
#include "utils/jobhandler.h"
#include "utils/mockobject.h"

#include "presentation/taskapplicationmodel.h"
#include "presentation/runningtaskmodel.h"

#include "testlib/fakejob.h"

using namespace mockitopp;
using namespace mockitopp::matcher;


class TaskApplicationModelTest : public QObject
{
    Q_OBJECT
public:
    explicit TaskApplicationModelTest(QObject *parent = Q_NULLPTR)
        : QObject(parent)
    {
        Utils::DependencyManager::globalInstance().add<Presentation::RunningTaskModel>(
                    [] (Utils::DependencyManager *) {
            return new Presentation::RunningTaskModel(Domain::TaskQueries::Ptr(),
                                                      Domain::TaskRepository::Ptr());
        });
    }

private slots:
    void shouldProvideRunningTaskModel()
    {
        // GIVEN
        Presentation::TaskApplicationModel app;

        // WHEN
        QObject *model = app.runningTaskModel();

        // THEN
        QVERIFY(qobject_cast<Presentation::RunningTaskModel*>(model));
    }

};

ZANSHIN_TEST_MAIN(TaskApplicationModelTest)

#include "taskapplicationmodeltest.moc"
