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

#include "domain/artifactqueries.h"
#include "domain/datasourcequeries.h"
#include "domain/noterepository.h"
#include "domain/taskqueries.h"
#include "domain/taskrepository.h"
#include "presentation/inboxpagemodel.h"

#include "testlib/fakejob.h"

using namespace mockitopp;
using namespace mockitopp::matcher;

class InboxPageModelTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldRetrieveDefaultTaskCollectionFromRepository()
    {
        // GIVEN

        // A data source
        auto expectedSource = Domain::DataSource::Ptr::create();

        // A source list containing the source we expect as default
        auto provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(expectedSource);
        provider->append(Domain::DataSource::Ptr::create());
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(provider);

        // Queries mock returning the list of data sources
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(sourceResult);

        // Dummy note repository
        mock_object<Domain::NoteRepository> noteRepositoryMock;

        // Repository mock returning the data source as default
        mock_object<Domain::TaskRepository> taskRepositoryMock;
        foreach (const Domain::DataSource::Ptr &source, provider->data()) {
            taskRepositoryMock(&Domain::TaskRepository::isDefaultSource).when(source).thenReturn(source == expectedSource);
        }

        Presentation::InboxPageModel inbox(0,
                                       &sourceQueriesMock.getInstance(),
                                       0,
                                       &taskRepositoryMock.getInstance(),
                                       &noteRepositoryMock.getInstance());

        // WHEN
        auto source = inbox.defaultTaskDataSource();

        // THEN
        QCOMPARE(source, expectedSource);
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::isDefaultSource).when(expectedSource).exactly(1));
    }

    void shouldGiveFirstTaskCollectionAsDefaultIfNoneMatched()
    {
        // GIVEN

        // A list of irrelevant sources
        auto provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(Domain::DataSource::Ptr::create());
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(provider);

        // Queries mock returning the list of data sources
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(sourceResult);

        // Dummy note repository
        mock_object<Domain::NoteRepository> noteRepositoryMock;

        // Repository mock returning the data source as default
        mock_object<Domain::TaskRepository> taskRepositoryMock;
        foreach (const Domain::DataSource::Ptr &source, provider->data()) {
            taskRepositoryMock(&Domain::TaskRepository::isDefaultSource).when(source).thenReturn(false);
        }

        Presentation::InboxPageModel inbox(0,
                                       &sourceQueriesMock.getInstance(),
                                       0,
                                       &taskRepositoryMock.getInstance(),
                                       &noteRepositoryMock.getInstance());

        // WHEN
        auto source = inbox.defaultTaskDataSource();

        // THEN
        QCOMPARE(source, provider->data().first());
    }

    void shouldProvideNullPointerIfNoTaskSourceIsAvailable()
    {
        // GIVEN

        // An empty source list
        auto provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(provider);

        // Queries mock returning the list of data sources
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(sourceResult);

        // Dummy note repository
        mock_object<Domain::NoteRepository> noteRepositoryMock;

        // Repository mock returning the data source as default
        mock_object<Domain::TaskRepository> taskRepositoryMock;

        Presentation::InboxPageModel inbox(0,
                                       &sourceQueriesMock.getInstance(),
                                       0,
                                       &taskRepositoryMock.getInstance(),
                                       &noteRepositoryMock.getInstance());

        // WHEN
        auto source = inbox.defaultTaskDataSource();

        // THEN
        QVERIFY(source.isNull());
    }

    void shouldForwardDefaultTaskCollectionToRepository()
    {
        // GIVEN

        // A data source
        auto source = Domain::DataSource::Ptr::create();

        // A dummy source list
        auto provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(provider);

        // Queries mock returning the list of data sources
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(sourceResult);

        // Dummy note repository
        mock_object<Domain::NoteRepository> noteRepositoryMock;

        // Repository mock setting the default data source
        mock_object<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::setDefaultSource).when(source).thenReturn();

        Presentation::InboxPageModel inbox(0,
                                       &sourceQueriesMock.getInstance(),
                                       0,
                                       &taskRepositoryMock.getInstance(),
                                       &noteRepositoryMock.getInstance());

        // WHEN
        inbox.setDefaultTaskDataSource(source);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::setDefaultSource).when(source).exactly(1));
    }

    void shouldRetrieveDefaultNoteCollectionFromRepository()
    {
        // GIVEN

        // A data source
        auto expectedSource = Domain::DataSource::Ptr::create();

        // A source list containing the source we expect as default
        auto provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(expectedSource);
        provider->append(Domain::DataSource::Ptr::create());
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(provider);

        // Queries mock returning the list of data sources
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(sourceResult);
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());

        // Dummy task repository
        mock_object<Domain::TaskRepository> taskRepositoryMock;

        // Repository mock returning the data source as default
        mock_object<Domain::NoteRepository> noteRepositoryMock;
        foreach (const Domain::DataSource::Ptr &source, provider->data()) {
            noteRepositoryMock(&Domain::NoteRepository::isDefaultSource).when(source).thenReturn(source == expectedSource);
        }

        Presentation::InboxPageModel inbox(0,
                                       &sourceQueriesMock.getInstance(),
                                       0,
                                       &taskRepositoryMock.getInstance(),
                                       &noteRepositoryMock.getInstance());

        // WHEN
        auto source = inbox.defaultNoteDataSource();

        // THEN
        QCOMPARE(source, expectedSource);
        QVERIFY(noteRepositoryMock(&Domain::NoteRepository::isDefaultSource).when(expectedSource).exactly(1));
    }

    void shouldGiveFirstNoteCollectionAsDefaultIfNoneMatched()
    {
        // GIVEN

        // A list of irrelevant sources
        auto provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(Domain::DataSource::Ptr::create());
        provider->append(Domain::DataSource::Ptr::create());
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(provider);

        // Queries mock returning the list of data sources
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(sourceResult);
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());

        // Dummy task repository
        mock_object<Domain::TaskRepository> taskRepositoryMock;

        // Repository mock returning the data source as default
        mock_object<Domain::NoteRepository> noteRepositoryMock;
        foreach (const Domain::DataSource::Ptr &source, provider->data()) {
            noteRepositoryMock(&Domain::NoteRepository::isDefaultSource).when(source).thenReturn(false);
        }

        Presentation::InboxPageModel inbox(0,
                                       &sourceQueriesMock.getInstance(),
                                       0,
                                       &taskRepositoryMock.getInstance(),
                                       &noteRepositoryMock.getInstance());

        // WHEN
        auto source = inbox.defaultNoteDataSource();

        // THEN
        QCOMPARE(source, provider->data().first());
    }

    void shouldProvideNullPointerIfNoNoteSourceIsAvailable()
    {
        // GIVEN

        // An empty source list
        auto provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(provider);

        // Queries mock returning the list of data sources
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(sourceResult);
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());

        // Dummy task repository
        mock_object<Domain::TaskRepository> taskRepositoryMock;

        // Repository mock returning the data source as default
        mock_object<Domain::NoteRepository> noteRepositoryMock;

        Presentation::InboxPageModel inbox(0,
                                       &sourceQueriesMock.getInstance(),
                                       0,
                                       &taskRepositoryMock.getInstance(),
                                       &noteRepositoryMock.getInstance());

        // WHEN
        auto source = inbox.defaultNoteDataSource();

        // THEN
        QVERIFY(source.isNull());
    }

    void shouldForwardDefaultNoteCollectionToRepository()
    {
        // GIVEN

        // A data source
        auto source = Domain::DataSource::Ptr::create();

        // A dummy source list
        auto provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(provider);

        // Queries mock returning the list of data sources
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(sourceResult);
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());

        // Dummy task repository
        mock_object<Domain::TaskRepository> taskRepositoryMock;

        // Repository mock setting the default data source
        mock_object<Domain::NoteRepository> noteRepositoryMock;
        noteRepositoryMock(&Domain::NoteRepository::setDefaultSource).when(source).thenReturn();

        Presentation::InboxPageModel inbox(0,
                                       &sourceQueriesMock.getInstance(),
                                       0,
                                       &taskRepositoryMock.getInstance(),
                                       &noteRepositoryMock.getInstance());

        // WHEN
        inbox.setDefaultNoteDataSource(source);

        // THEN
        QVERIFY(noteRepositoryMock(&Domain::NoteRepository::setDefaultSource).when(source).exactly(1));
    }

    void shouldListInboxInCentralListModel()
    {
        // GIVEN

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

        // We won't need source queries in that test
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());

        mock_object<Domain::ArtifactQueries> artifactQueriesMock;
        artifactQueriesMock(&Domain::ArtifactQueries::findInboxTopLevel).when().thenReturn(artifactResult);

        mock_object<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(rootTask).thenReturn(taskResult);
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(childTask).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());

        mock_object<Domain::TaskRepository> taskRepositoryMock;
        mock_object<Domain::NoteRepository> noteRepositoryMock;

        Presentation::InboxPageModel inbox(&artifactQueriesMock.getInstance(),
                                       &sourceQueriesMock.getInstance(),
                                       &taskQueriesMock.getInstance(),
                                       &taskRepositoryMock.getInstance(),
                                       &noteRepositoryMock.getInstance());

        // WHEN
        QAbstractItemModel *model = inbox.centralListModel();

        // THEN
        const QModelIndex rootTaskIndex = model->index(0, 0);
        const QModelIndex rootNoteIndex = model->index(1, 0);
        const QModelIndex childTaskIndex = model->index(0, 0, rootTaskIndex);

        QCOMPARE(model->rowCount(), 2);
        QCOMPARE(model->rowCount(rootTaskIndex), 1);
        QCOMPARE(model->rowCount(rootNoteIndex), 0);
        QCOMPARE(model->rowCount(childTaskIndex), 0);

        const Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable
                                         | Qt::ItemIsEnabled
                                         | Qt::ItemIsEditable;
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
    }

    void shouldAddTasks()
    {
        // GIVEN

        // We won't need source queries in that test...
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());

        // ... in fact we won't list any model
        mock_object<Domain::ArtifactQueries> artifactQueriesMock;
        mock_object<Domain::TaskQueries> taskQueriesMock;

        // Nor create notes...
        mock_object<Domain::NoteRepository> noteRepositoryMock;

        // We'll gladly create a task though
        mock_object<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::save).when(any<Domain::Task::Ptr>()).thenReturn(new FakeJob(this));

        Presentation::InboxPageModel inbox(&artifactQueriesMock.getInstance(),
                                           &sourceQueriesMock.getInstance(),
                                           &taskQueriesMock.getInstance(),
                                           &taskRepositoryMock.getInstance(),
                                           &noteRepositoryMock.getInstance());

        // WHEN
        inbox.addTask("New task");

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::save).when(any<Domain::Task::Ptr>()).exactly(1));
    }

    void shouldDeleteItems()
    {
        // GIVEN

        // Two tasks
        auto task1 = Domain::Task::Ptr::create();
        auto task2 = Domain::Task::Ptr::create();
        auto artifactProvider = Domain::QueryResultProvider<Domain::Artifact::Ptr>::Ptr::create();
        auto artifactResult = Domain::QueryResult<Domain::Artifact::Ptr>::create(artifactProvider);
        artifactProvider->append(task1);
        artifactProvider->append(task2);

        // We won't need source queries in that test
        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findNotes).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());
        sourceQueriesMock(&Domain::DataSourceQueries::findTasks).when().thenReturn(Domain::QueryResult<Domain::DataSource::Ptr>::Ptr());

        mock_object<Domain::ArtifactQueries> artifactQueriesMock;
        artifactQueriesMock(&Domain::ArtifactQueries::findInboxTopLevel).when().thenReturn(artifactResult);

        mock_object<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task1).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task2).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());

        mock_object<Domain::NoteRepository> noteRepositoryMock;

        mock_object<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::remove).when(task2).thenReturn(new FakeJob(this));

        Presentation::InboxPageModel inbox(&artifactQueriesMock.getInstance(),
                                           &sourceQueriesMock.getInstance(),
                                           &taskQueriesMock.getInstance(),
                                           &taskRepositoryMock.getInstance(),
                                           &noteRepositoryMock.getInstance());

        // WHEN
        const QModelIndex index = inbox.centralListModel()->index(1, 0);
        inbox.removeItem(index);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::remove).when(task2).exactly(1));
    }
};

QTEST_MAIN(InboxPageModelTest)

#include "inboxpagemodeltest.moc"
