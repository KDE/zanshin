/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
   Copyright 2014 Franck Arrecot <franck.arrecot@gmail.com>

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

#include "utils/mockobject.h"

#include "domain/noterepository.h"
#include "domain/tagqueries.h"
#include "domain/tagrepository.h"
#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

#include "presentation/tagpagemodel.h"

#include "testlib/fakejob.h"

using namespace mockitopp;
using namespace mockitopp::matcher;

class FakeErrorHandler : public Presentation::ErrorHandler
{
public:
    void doDisplayMessage(const QString &message)
    {
        m_message = message;
    }

    QString m_message;
};

class TagPageModelTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldListTagArtifactsInCentralListModel()
    {
        // GIVEN

        // One Tag
        auto tag = Domain::Tag::Ptr::create();

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

        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findTopLevelArtifacts).when(tag).thenReturn(artifactResult);

        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(rootTask).thenReturn(taskResult);
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(childTask).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;

        Presentation::TagPageModel page(tag,
                                        tagQueriesMock.getInstance(),
                                        tagRepositoryMock.getInstance(),
                                        taskQueriesMock.getInstance(),
                                        taskRepositoryMock.getInstance(),
                                        noteRepositoryMock.getInstance());

        // WHEN
        QAbstractItemModel *model = page.centralListModel();

        // THEN
        const QModelIndex rootTaskIndex = model->index(0, 0);
        const QModelIndex rootNoteIndex = model->index(1, 0);
        const QModelIndex childTaskIndex = model->index(0, 0, rootTaskIndex);

        QCOMPARE(page.tag(), tag);

        QCOMPARE(model->rowCount(), 2);
        QCOMPARE(model->rowCount(rootTaskIndex), 1);
        QCOMPARE(model->rowCount(rootNoteIndex), 0);
        QCOMPARE(model->rowCount(childTaskIndex), 0);

        const Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable
                                         | Qt::ItemIsEnabled
                                         | Qt::ItemIsEditable
                                         | Qt::ItemIsDragEnabled;
        QCOMPARE(model->flags(rootTaskIndex), defaultFlags | Qt::ItemIsUserCheckable | Qt::ItemIsDropEnabled);
        QCOMPARE(model->flags(rootNoteIndex), defaultFlags);
        QCOMPARE(model->flags(childTaskIndex), defaultFlags | Qt::ItemIsUserCheckable | Qt::ItemIsDropEnabled);

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
        taskRepositoryMock(&Domain::TaskRepository::update).when(rootTask).thenReturn(new FakeJob(this));
        taskRepositoryMock(&Domain::TaskRepository::update).when(childTask).thenReturn(new FakeJob(this));
        noteRepositoryMock(&Domain::NoteRepository::save).when(rootNote).thenReturn(new FakeJob(this));

        QVERIFY(model->setData(rootTaskIndex, "newRootTask"));
        QVERIFY(model->setData(rootNoteIndex, "newRootNote"));
        QVERIFY(model->setData(childTaskIndex, "newChildTask"));

        QVERIFY(model->setData(rootTaskIndex, Qt::Checked, Qt::CheckStateRole));
        QVERIFY(!model->setData(rootNoteIndex, Qt::Checked, Qt::CheckStateRole));
        QVERIFY(model->setData(childTaskIndex, Qt::Unchecked, Qt::CheckStateRole));

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::update).when(rootTask).exactly(2));
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::update).when(childTask).exactly(2));
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
        QCOMPARE(data->property("objects").value<Domain::Artifact::List>(),
                 Domain::Artifact::List() << childTask);

        // WHEN
        data = model->mimeData(QModelIndexList() << rootNoteIndex);

        // THEN
        QVERIFY(data->hasFormat("application/x-zanshin-object"));
        QCOMPARE(data->property("objects").value<Domain::Artifact::List>(),
                 Domain::Artifact::List() << rootNote);


        // WHEN
        auto childTask2 = Domain::Task::Ptr::create();
        taskRepositoryMock(&Domain::TaskRepository::associate).when(rootTask, childTask2).thenReturn(new FakeJob(this));
        data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << childTask2));
        model->dropMimeData(data, Qt::MoveAction, -1, -1, rootTaskIndex);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::associate).when(rootTask, childTask2).exactly(1));


        // WHEN
        data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << rootNote));
        bool result = model->dropMimeData(data, Qt::MoveAction, -1, -1, childTaskIndex);

        // THEN
        QVERIFY(!result);

        // WHEN
        auto childTask3 = Domain::Task::Ptr::create();
        auto childTask4 = Domain::Task::Ptr::create();
        taskRepositoryMock(&Domain::TaskRepository::associate).when(rootTask, childTask3).thenReturn(new FakeJob(this));
        taskRepositoryMock(&Domain::TaskRepository::associate).when(rootTask, childTask4).thenReturn(new FakeJob(this));
        data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Artifact::List() << childTask3 << childTask4));
        model->dropMimeData(data, Qt::MoveAction, -1, -1, rootTaskIndex);

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::associate).when(rootTask, childTask3).exactly(1));
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::associate).when(rootTask, childTask4).exactly(1));
    }

    void shouldAddTasks()
    {
        // GIVEN

        // One Tag
        auto tag = Domain::Tag::Ptr::create();

        // ... in fact we won't list any model
        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;

        // Nor create notes...
        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;
        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;

        // We'll gladly create a task though
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::createInTag).when(any<Domain::Task::Ptr>(),
                                                                          any<Domain::Tag::Ptr>())
                                                                    .thenReturn(new FakeJob(this));

        Presentation::TagPageModel page(tag,
                                        tagQueriesMock.getInstance(),
                                        tagRepositoryMock.getInstance(),
                                        taskQueriesMock.getInstance(),
                                        taskRepositoryMock.getInstance(),
                                        noteRepositoryMock.getInstance());

        // WHEN
        page.addTask("New task");

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::createInTag).when(any<Domain::Task::Ptr>(),
                                                                                  any<Domain::Tag::Ptr>())
                                                                            .exactly(1));
    }

    void shouldRemoveItem()
    {
        // GIVEN

        // One domain tag
        auto tag = Domain::Tag::Ptr::create();

        // Two tasks
        Domain::Artifact::Ptr task1(new Domain::Task);
        Domain::Artifact::Ptr task2(new Domain::Task);
        Domain::Artifact::Ptr note(new Domain::Note);

        auto artifactProvider = Domain::QueryResultProvider<Domain::Artifact::Ptr>::Ptr::create();
        auto artifactResult = Domain::QueryResult<Domain::Artifact::Ptr>::create(artifactProvider);
        artifactProvider->append(task1);
        artifactProvider->append(task2);
        artifactProvider->append(note);

        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findTopLevelArtifacts).when(tag).thenReturn(artifactResult);

        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;
        tagRepositoryMock(&Domain::TagRepository::dissociate).when(tag, task2).thenReturn(new FakeJob(this));
        tagRepositoryMock(&Domain::TagRepository::dissociate).when(tag, note).thenReturn(new FakeJob(this));

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task1.dynamicCast<Domain::Task>()).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(task2.dynamicCast<Domain::Task>()).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;

        Presentation::TagPageModel page(tag,
                                        tagQueriesMock.getInstance(),
                                        tagRepositoryMock.getInstance(),
                                        taskQueriesMock.getInstance(),
                                        taskRepositoryMock.getInstance(),
                                        noteRepositoryMock.getInstance());

        // WHEN
        const QModelIndex indexTask2 = page.centralListModel()->index(1, 0);
        page.removeItem(indexTask2);

        // THEN
        QVERIFY(tagRepositoryMock(&Domain::TagRepository::dissociate).when(tag, task2).exactly(1));

        // WHEN
        const QModelIndex indexNote = page.centralListModel()->index(2, 0);
        page.removeItem(indexNote);

        // THEN
        QVERIFY(tagRepositoryMock(&Domain::TagRepository::dissociate).when(tag, note).exactly(1));
    }

    void shouldGetAnErrorMessageWhenAddTaskFailed()
    {
        // GIVEN

        // One Tag
        auto tag = Domain::Tag::Ptr::create();
        tag->setName("Tag1");

        // ... in fact we won't list any model
        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;

        // Nor create notes...
        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;
        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;

        // We'll gladly create a task though
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        taskRepositoryMock(&Domain::TaskRepository::createInTag).when(any<Domain::Task::Ptr>(),
                                                                          any<Domain::Tag::Ptr>())
                                                                    .thenReturn(job);

        Presentation::TagPageModel page(tag,
                                        tagQueriesMock.getInstance(),
                                        tagRepositoryMock.getInstance(),
                                        taskQueriesMock.getInstance(),
                                        taskRepositoryMock.getInstance(),
                                        noteRepositoryMock.getInstance());
        FakeErrorHandler errorHandler;
        page.setErrorHandler(&errorHandler);

        // WHEN
        page.addTask("New task");

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot add task New task in tag Tag1: Foo"));
    }

    void shouldGetAnErrorMessageWhenUpdateTaskFailed()
    {
        // GIVEN

        // One Tag
        auto tag = Domain::Tag::Ptr::create();
        tag->setName("Tag1");

        // One note and one task
        auto rootTask = Domain::Task::Ptr::create();
        rootTask->setTitle("rootTask");
        auto artifactProvider = Domain::QueryResultProvider<Domain::Artifact::Ptr>::Ptr::create();
        auto artifactResult = Domain::QueryResult<Domain::Artifact::Ptr>::create(artifactProvider);
        artifactProvider->append(rootTask);

        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findTopLevelArtifacts).when(tag).thenReturn(artifactResult);

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        taskQueriesMock(&Domain::TaskQueries::findChildren).when(rootTask).thenReturn(Domain::QueryResult<Domain::Task::Ptr>::Ptr());

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;
        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;

        Presentation::TagPageModel page(tag,
                                        tagQueriesMock.getInstance(),
                                        tagRepositoryMock.getInstance(),
                                        taskQueriesMock.getInstance(),
                                        taskRepositoryMock.getInstance(),
                                        noteRepositoryMock.getInstance());

        QAbstractItemModel *model = page.centralListModel();
        const QModelIndex rootTaskIndex = model->index(0, 0);
        FakeErrorHandler errorHandler;
        page.setErrorHandler(&errorHandler);

        // WHEN
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        taskRepositoryMock(&Domain::TaskRepository::update).when(rootTask).thenReturn(job);

        QVERIFY(model->setData(rootTaskIndex, "newRootTask"));

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot modify task rootTask in tag Tag1: Foo"));
    }

    void shouldGetAnErrorMessageWhenUpdateNoteFailed()
    {
        // GIVEN

        // One Tag
        auto tag = Domain::Tag::Ptr::create();
        tag->setName("Tag1");

        // One note and one task
        auto rootNote = Domain::Note::Ptr::create();
        rootNote->setTitle("rootNote");
        auto artifactProvider = Domain::QueryResultProvider<Domain::Artifact::Ptr>::Ptr::create();
        auto artifactResult = Domain::QueryResult<Domain::Artifact::Ptr>::create(artifactProvider);
        artifactProvider->append(rootNote);

        Utils::MockObject<Domain::TagQueries> tagQueriesMock;
        tagQueriesMock(&Domain::TagQueries::findTopLevelArtifacts).when(tag).thenReturn(artifactResult);

        Utils::MockObject<Domain::TaskQueries> taskQueriesMock;
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;
        Utils::MockObject<Domain::TagRepository> tagRepositoryMock;

        Presentation::TagPageModel page(tag,
                                        tagQueriesMock.getInstance(),
                                        tagRepositoryMock.getInstance(),
                                        taskQueriesMock.getInstance(),
                                        taskRepositoryMock.getInstance(),
                                        noteRepositoryMock.getInstance());

        QAbstractItemModel *model = page.centralListModel();
        const QModelIndex rootNoteIndex = model->index(0, 0);
        FakeErrorHandler errorHandler;
        page.setErrorHandler(&errorHandler);

        // WHEN
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        noteRepositoryMock(&Domain::NoteRepository::save).when(rootNote).thenReturn(job);

        QVERIFY(model->setData(rootNoteIndex, "newRootNote"));

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot modify note rootNote in tag Tag1: Foo"));
    }
};

QTEST_MAIN(TagPageModelTest)

#include "tagpagemodeltest.moc"
