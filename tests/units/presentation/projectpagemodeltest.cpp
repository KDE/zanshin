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

#include <QtTest>

#include <mockitopp/mockitopp.hpp>

#include "domain/noterepository.h"
#include "domain/projectqueries.h"
#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

#include "presentation/projectpagemodel.h"

#include "testlib/fakejob.h"

using namespace mockitopp;

class ProjectPageModelTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldListProjectInCentralListModel()
    {
        // GIVEN

        // One project
        auto project = Domain::Project::Ptr::create();

        // One note and one task
        auto rootTask = Domain::Task::Ptr::create();
        rootTask->setTitle("rootTask");
        auto rootNote = Domain::Note::Ptr::create();
        rootNote->setTitle("rootNote");
        auto artifactProvider = Domain::QueryResultProvider<Domain::Artifact::Ptr>::Ptr::create();
        auto artifactResult = Domain::QueryResult<Domain::Artifact::Ptr>::create(artifactProvider);
        artifactProvider->append(rootTask);
        artifactProvider->append(rootNote);

        // One task under the root task
        auto childTask = Domain::Task::Ptr::create();
        childTask->setTitle("childTask");
        childTask->setDone(true);
        auto taskProvider = Domain::QueryResultProvider<Domain::Task::Ptr>::Ptr::create();
        auto taskResult = Domain::QueryResult<Domain::Task::Ptr>::create(taskProvider);
        taskProvider->append(childTask);

        mock_object<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findTopLevelArtifacts).when(project).thenReturn(artifactResult);

        mock_object<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(rootTask).thenReturn(taskResult);
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(childTask).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());

        mock_object<Domain::TaskRepository> taskRepositoryMock;
        mock_object<Domain::NoteRepository> noteRepositoryMock;

        Presentation::ProjectPageModel page(project,
                                            &projectQueriesMock.getInstance(),
                                            &taskQueriesMock.getInstance(),
                                            &taskRepositoryMock.getInstance(),
                                            &noteRepositoryMock.getInstance());

        // WHEN
        QAbstractItemModel *model = page.centralListModel();

        // THEN
        const QModelIndex rootTaskIndex = model->index(0, 0);
        const QModelIndex rootNoteIndex = model->index(1, 0);
        const QModelIndex childTaskIndex = model->index(0, 0, rootTaskIndex);

        QCOMPARE(page.project(), project);

        QCOMPARE(model->rowCount(), 2);
        QCOMPARE(model->rowCount(rootTaskIndex), 1);
        QCOMPARE(model->rowCount(rootNoteIndex), 0);
        QCOMPARE(model->rowCount(childTaskIndex), 0);

        const Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable
                                         | Qt::ItemIsEnabled
                                         | Qt::ItemIsEditable
                                         | Qt::ItemIsDragEnabled;
        QCOMPARE(model->flags(rootTaskIndex), defaultFlags | Qt::ItemIsUserCheckable);
        QCOMPARE(model->flags(rootNoteIndex), defaultFlags);
        QCOMPARE(model->flags(childTaskIndex), defaultFlags | Qt::ItemIsUserCheckable);

        QCOMPARE(model->data(rootTaskIndex).toString(), rootTask->title());
        QCOMPARE(model->data(rootNoteIndex).toString(), rootNote->title());
        QCOMPARE(model->data(childTaskIndex).toString(), childTask->title());

        QCOMPARE(model->data(rootTaskIndex, Qt::EditRole).toString(), rootTask->title());
        QCOMPARE(model->data(rootNoteIndex, Qt::EditRole).toString(), rootNote->title());
        QCOMPARE(model->data(childTaskIndex, Qt::EditRole).toString(), childTask->title());

        QVERIFY(model->data(rootTaskIndex, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(rootNoteIndex, Qt::CheckStateRole).isValid());
        QVERIFY(model->data(childTaskIndex, Qt::CheckStateRole).isValid());

        QCOMPARE(model->data(rootTaskIndex, Qt::CheckStateRole).toBool(), rootTask->isDone());
        QCOMPARE(model->data(childTaskIndex, Qt::CheckStateRole).toBool(), childTask->isDone());

        // WHEN
        taskRepositoryMock(&Domain::TaskRepository::save).when(rootTask).thenReturn(new FakeJob(this));
        taskRepositoryMock(&Domain::TaskRepository::save).when(childTask).thenReturn(new FakeJob(this));
        noteRepositoryMock(&Domain::NoteRepository::save).when(rootNote).thenReturn(new FakeJob(this));

        QVERIFY(model->setData(rootTaskIndex, "newRootTask"));
        QVERIFY(model->setData(rootNoteIndex, "newRootNote"));
        QVERIFY(model->setData(childTaskIndex, "newChildTask"));

        QVERIFY(model->setData(rootTaskIndex, Qt::Checked, Qt::CheckStateRole));
        QVERIFY(!model->setData(rootNoteIndex, Qt::Checked, Qt::CheckStateRole));
        QVERIFY(model->setData(childTaskIndex, Qt::Unchecked, Qt::CheckStateRole));

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::save).when(rootTask).exactly(2));
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::save).when(childTask).exactly(2));
        QVERIFY(noteRepositoryMock(&Domain::NoteRepository::save).when(rootNote).exactly(1));

        QCOMPARE(rootTask->title(), QString("newRootTask"));
        QCOMPARE(rootNote->title(), QString("newRootNote"));
        QCOMPARE(childTask->title(), QString("newChildTask"));

        QCOMPARE(rootTask->isDone(), true);
        QCOMPARE(childTask->isDone(), false);

        // WHEN
        QMimeData *data = model->mimeData(QModelIndexList() << childTaskIndex);

        // THEN
        QVERIFY(data->hasFormat("application/x-zanshin-object"));
        QCOMPARE(data->property("object").value<Domain::Artifact::Ptr>(),
                 Domain::Artifact::Ptr(childTask));

        // WHEN
        data = model->mimeData(QModelIndexList() << rootNoteIndex);

        // THEN
        QVERIFY(data->hasFormat("application/x-zanshin-object"));
        QCOMPARE(data->property("object").value<Domain::Artifact::Ptr>(),
                 Domain::Artifact::Ptr(rootNote));
    }

    void shouldDeleteItems()
    {
        // GIVEN

        // One project
        auto project = Domain::Project::Ptr::create();

        // Two tasks
        auto task1 = Domain::Task::Ptr::create();
        auto task2 = Domain::Task::Ptr::create();
        auto artifactProvider = Domain::QueryResultProvider<Domain::Artifact::Ptr>::Ptr::create();
        auto artifactResult = Domain::QueryResult<Domain::Artifact::Ptr>::create(artifactProvider);
        artifactProvider->append(task1);
        artifactProvider->append(task2);

        mock_object<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findTopLevelArtifacts).when(project).thenReturn(artifactResult);

        mock_object<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task1).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task2).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());

        mock_object<Domain::NoteRepository> noteRepositoryMock;

        mock_object<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::remove).when(task2).thenReturn(new FakeJob(this));

        Presentation::ProjectPageModel page(project,
                                            &projectQueriesMock.getInstance(),
                                            &taskQueriesMock.getInstance(),
                                            &taskRepositoryMock.getInstance(),
                                            &noteRepositoryMock.getInstance());

        // WHEN
        const QModelIndex index = page.centralListModel()->index(1, 0);
        page.removeItem(index);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::remove).when(task2).exactly(1));
    }

    // Clearly this one will go away when we'll get more support of notes
    void shouldNotTryToDeleteNotes()
    {
        // GIVEN

        // One project
        auto project = Domain::Project::Ptr::create();

        // One task, one note
        auto task1 = Domain::Task::Ptr::create();
        auto note2 = Domain::Note::Ptr::create();
        auto artifactProvider = Domain::QueryResultProvider<Domain::Artifact::Ptr>::Ptr::create();
        auto artifactResult = Domain::QueryResult<Domain::Artifact::Ptr>::create(artifactProvider);
        artifactProvider->append(task1);
        artifactProvider->append(note2);

        mock_object<Domain::ProjectQueries> projectQueriesMock;
        projectQueriesMock(&Domain::ProjectQueries::findTopLevelArtifacts).when(project).thenReturn(artifactResult);

        mock_object<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task1).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());

        mock_object<Domain::NoteRepository> noteRepositoryMock;
        mock_object<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::remove).when(Domain::Task::Ptr()).thenReturn(new FakeJob(this));

        Presentation::ProjectPageModel page(project,
                                            &projectQueriesMock.getInstance(),
                                            &taskQueriesMock.getInstance(),
                                            &taskRepositoryMock.getInstance(),
                                            &noteRepositoryMock.getInstance());

        // WHEN
        const QModelIndex index = page.centralListModel()->index(1, 0);
        page.removeItem(index);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::remove).when(Domain::Task::Ptr()).exactly(0));
    }
};

QTEST_MAIN(ProjectPageModelTest)

#include "projectpagemodeltest.moc"
