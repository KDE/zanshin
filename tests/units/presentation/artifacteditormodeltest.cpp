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

#include "utils/mockobject.h"

#include <QSignalSpy>

#include "testlib/fakejob.h"

#include "domain/task.h"
#include "domain/taskrepository.h"
#include "domain/note.h"
#include "domain/noterepository.h"

#include "presentation/artifacteditormodel.h"
#include "presentation/errorhandler.h"

using namespace mockitopp;

class FakeErrorHandler : public Presentation::ErrorHandler
{
public:
    void doDisplayMessage(const QString &message)
    {
        m_message = message;
    }

    QString m_message;
};

class ArtifactEditorModelTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldHaveEmptyDefaultState()
    {
        // GIVEN
        auto taskRepository = Domain::TaskRepository::Ptr();
        auto noteRepository = Domain::NoteRepository::Ptr();
        Presentation::ArtifactEditorModel model(taskRepository,
                                                noteRepository);

        // WHEN
        // Nothing

        // THEN
        QVERIFY(model.artifact().isNull());
        QVERIFY(!model.hasTaskProperties());
        QVERIFY(model.text().isEmpty());
        QVERIFY(model.title().isEmpty());
        QVERIFY(!model.isDone());
        QVERIFY(model.startDate().isNull());
        QVERIFY(model.dueDate().isNull());
        QVERIFY(model.delegateText().isNull());
    }

    void shouldHaveTaskProperties()
    {
        // GIVEN
        auto taskRepository = Domain::TaskRepository::Ptr();
        auto noteRepository = Domain::NoteRepository::Ptr();
        Presentation::ArtifactEditorModel model(taskRepository,
                                                noteRepository);
        QSignalSpy textSpy(&model, SIGNAL(textChanged(QString)));
        QSignalSpy titleSpy(&model, SIGNAL(titleChanged(QString)));
        QSignalSpy doneSpy(&model, SIGNAL(doneChanged(bool)));
        QSignalSpy startSpy(&model, SIGNAL(startDateChanged(QDateTime)));
        QSignalSpy dueSpy(&model, SIGNAL(dueDateChanged(QDateTime)));
        QSignalSpy delegateSpy(&model, SIGNAL(delegateTextChanged(QString)));

        auto task = Domain::Task::Ptr::create();
        task->setText("description");
        task->setTitle("title");
        task->setDone(true);
        task->setStartDate(QDateTime::currentDateTime());
        task->setDueDate(QDateTime::currentDateTime().addDays(2));
        task->setDelegate(Domain::Task::Delegate("John Doe", "john@doe.com"));

        // WHEN
        model.setArtifact(task);
        // To make sure we don't signal too much
        model.setText(task->text());
        model.setTitle(task->title());
        model.setDone(task->isDone());
        model.setStartDate(task->startDate());
        model.setDueDate(task->dueDate());

        // THEN
        QVERIFY(model.hasTaskProperties());

        QCOMPARE(textSpy.size(), 1);
        QCOMPARE(textSpy.takeFirst().takeFirst().toString(), task->text());
        QCOMPARE(model.property("text").toString(), task->text());

        QCOMPARE(titleSpy.size(), 1);
        QCOMPARE(titleSpy.takeFirst().takeFirst().toString(), task->title());
        QCOMPARE(model.property("title").toString(), task->title());

        QCOMPARE(doneSpy.size(), 1);
        QCOMPARE(doneSpy.takeFirst().takeFirst().toBool(), task->isDone());
        QCOMPARE(model.property("done").toBool(), task->isDone());

        QCOMPARE(startSpy.size(), 1);
        QCOMPARE(startSpy.takeFirst().takeFirst().toDateTime(), task->startDate());
        QCOMPARE(model.property("startDate").toDateTime(), task->startDate());

        QCOMPARE(dueSpy.size(), 1);
        QCOMPARE(dueSpy.takeFirst().takeFirst().toDateTime(), task->dueDate());
        QCOMPARE(model.property("dueDate").toDateTime(), task->dueDate());

        QCOMPARE(delegateSpy.size(), 1);
        QCOMPARE(delegateSpy.takeFirst().takeFirst().toString(), task->delegate().display());
        QCOMPARE(model.property("delegateText").toString(), task->delegate().display());
    }

    void shouldHaveNoteProperties()
    {
        // GIVEN
        auto taskRepository = Domain::TaskRepository::Ptr();
        auto noteRepository = Domain::NoteRepository::Ptr();
        Presentation::ArtifactEditorModel model(taskRepository,
                                                noteRepository);
        QSignalSpy textSpy(&model, SIGNAL(textChanged(QString)));
        QSignalSpy titleSpy(&model, SIGNAL(titleChanged(QString)));
        QSignalSpy doneSpy(&model, SIGNAL(doneChanged(bool)));
        QSignalSpy startSpy(&model, SIGNAL(startDateChanged(QDateTime)));
        QSignalSpy dueSpy(&model, SIGNAL(dueDateChanged(QDateTime)));
        QSignalSpy delegateSpy(&model, SIGNAL(delegateTextChanged(QString)));

        auto note = Domain::Note::Ptr::create();
        note->setText("description");
        note->setTitle("title");

        // WHEN
        model.setArtifact(note);
        // To make sure we don't signal too much
        model.setText(note->text());
        model.setTitle(note->title());

        // THEN
        QVERIFY(!model.hasTaskProperties());

        QCOMPARE(textSpy.size(), 1);
        QCOMPARE(textSpy.takeFirst().takeFirst().toString(), note->text());
        QCOMPARE(model.property("text").toString(), note->text());

        QCOMPARE(titleSpy.size(), 1);
        QCOMPARE(titleSpy.takeFirst().takeFirst().toString(), note->title());
        QCOMPARE(model.property("title").toString(), note->title());

        QCOMPARE(doneSpy.size(), 1);
        QCOMPARE(doneSpy.takeFirst().takeFirst().toBool(), false);
        QCOMPARE(model.property("done").toBool(), false);

        QCOMPARE(startSpy.size(), 1);
        QVERIFY(startSpy.takeFirst().takeFirst().toDateTime().isNull());
        QVERIFY(model.property("startDate").toDateTime().isNull());

        QCOMPARE(dueSpy.size(), 1);
        QVERIFY(dueSpy.takeFirst().takeFirst().toDateTime().isNull());
        QVERIFY(model.property("dueDate").toDateTime().isNull());

        QCOMPARE(delegateSpy.size(), 1);
        QVERIFY(delegateSpy.takeFirst().takeFirst().toString().isEmpty());
        QVERIFY(model.property("delegateText").toString().isEmpty());
    }

    void shouldReactToArtifactPropertyChanges_data()
    {
        QTest::addColumn<Domain::Artifact::Ptr>("artifact");
        QTest::addColumn<QByteArray>("propertyName");
        QTest::addColumn<QVariant>("propertyValue");
        QTest::addColumn<QByteArray>("signal");

        QTest::newRow("note text") << Domain::Artifact::Ptr(Domain::Note::Ptr::create())
                                   << QByteArray("text")
                                   << QVariant("new text")
                                   << QByteArray(SIGNAL(textChanged(QString)));

        QTest::newRow("note title") << Domain::Artifact::Ptr(Domain::Note::Ptr::create())
                                    << QByteArray("title")
                                    << QVariant("new title")
                                    << QByteArray(SIGNAL(titleChanged(QString)));

        QTest::newRow("task text") << Domain::Artifact::Ptr(Domain::Task::Ptr::create())
                                   << QByteArray("text")
                                   << QVariant("new text")
                                   << QByteArray(SIGNAL(textChanged(QString)));

        QTest::newRow("task title") << Domain::Artifact::Ptr(Domain::Task::Ptr::create())
                                    << QByteArray("title")
                                    << QVariant("new title")
                                    << QByteArray(SIGNAL(titleChanged(QString)));

        QTest::newRow("task done") << Domain::Artifact::Ptr(Domain::Task::Ptr::create())
                                   << QByteArray("done")
                                   << QVariant(true)
                                   << QByteArray(SIGNAL(doneChanged(bool)));

        QTest::newRow("task start") << Domain::Artifact::Ptr(Domain::Task::Ptr::create())
                                    << QByteArray("startDate")
                                    << QVariant(QDateTime::currentDateTime())
                                    << QByteArray(SIGNAL(startDateChanged(QDateTime)));

        QTest::newRow("task due") << Domain::Artifact::Ptr(Domain::Task::Ptr::create())
                                  << QByteArray("dueDate")
                                  << QVariant(QDateTime::currentDateTime().addDays(2))
                                  << QByteArray(SIGNAL(dueDateChanged(QDateTime)));
    }

    void shouldReactToArtifactPropertyChanges()
    {
        // GIVEN
        QFETCH(Domain::Artifact::Ptr, artifact);
        QFETCH(QByteArray, propertyName);
        QFETCH(QVariant, propertyValue);
        QFETCH(QByteArray, signal);

        auto taskRepository = Domain::TaskRepository::Ptr();
        auto noteRepository = Domain::NoteRepository::Ptr();
        Presentation::ArtifactEditorModel model(taskRepository,
                                                noteRepository);
        model.setArtifact(artifact);
        QSignalSpy spy(&model, signal.constData());

        // WHEN
        artifact->setProperty(propertyName, propertyValue);

        // THEN
        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().takeFirst(), propertyValue);
        QCOMPARE(model.property(propertyName), propertyValue);
    }

    void shouldReactToTaskDelegateChanges()
    {
        // GIVEN
        auto task = Domain::Task::Ptr::create();
        auto taskRepository = Domain::TaskRepository::Ptr();
        auto noteRepository = Domain::NoteRepository::Ptr();
        Presentation::ArtifactEditorModel model(taskRepository,
                                                noteRepository);
        model.setArtifact(task);
        QSignalSpy spy(&model, SIGNAL(delegateTextChanged(QString)));

        // WHEN
        task->setDelegate(Domain::Task::Delegate("John Doe", "john@doe.com"));

        // THEN
        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().takeFirst().toString(), task->delegate().display());
        QCOMPARE(model.property("delegateText").toString(), task->delegate().display());
    }

    void shouldApplyChangesBackToArtifactAfterADelay_data()
    {
        shouldReactToArtifactPropertyChanges_data();
    }

    void shouldApplyChangesBackToArtifactAfterADelay()
    {
        // GIVEN
        QFETCH(Domain::Artifact::Ptr, artifact);
        QFETCH(QByteArray, propertyName);
        QFETCH(QVariant, propertyValue);
        QFETCH(QByteArray, signal);

        auto task = artifact.objectCast<Domain::Task>();
        auto note = artifact.objectCast<Domain::Note>();

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::update).when(task).thenReturn(new FakeJob(this));
        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;
        noteRepositoryMock(&Domain::NoteRepository::save).when(note).thenReturn(new FakeJob(this));

        Presentation::ArtifactEditorModel model(taskRepositoryMock.getInstance(),
                                                noteRepositoryMock.getInstance());
        model.setArtifact(artifact);
        QSignalSpy spy(&model, signal.constData());

        // WHEN
        model.setProperty(propertyName, propertyValue);

        // THEN
        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().takeFirst(), propertyValue);
        QCOMPARE(model.property(propertyName), propertyValue);
        QVERIFY(artifact->property(propertyName) != propertyValue);
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::update).when(task).exactly(0));
        QVERIFY(noteRepositoryMock(&Domain::NoteRepository::save).when(note).exactly(0));

        // WHEN (apply after delay)
        QTest::qWait(model.autoSaveDelay() + 50);

        // THEN
        QCOMPARE(artifact->property(propertyName), propertyValue);
        if (task) {
            QVERIFY(taskRepositoryMock(&Domain::TaskRepository::update).when(task).exactly(1));
        } else {
            QVERIFY(noteRepositoryMock(&Domain::NoteRepository::save).when(note).exactly(1));
        }
    }

    void shouldApplyChangesImmediatelyIfANewArtifactIsSet_data()
    {
        shouldReactToArtifactPropertyChanges_data();
    }

    void shouldApplyChangesImmediatelyIfANewArtifactIsSet()
    {
        // GIVEN
        QFETCH(Domain::Artifact::Ptr, artifact);
        QFETCH(QByteArray, propertyName);
        QFETCH(QVariant, propertyValue);
        QFETCH(QByteArray, signal);

        auto task = artifact.objectCast<Domain::Task>();
        auto note = artifact.objectCast<Domain::Note>();

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::update).when(task).thenReturn(new FakeJob(this));
        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;
        noteRepositoryMock(&Domain::NoteRepository::save).when(note).thenReturn(new FakeJob(this));

        Presentation::ArtifactEditorModel model(taskRepositoryMock.getInstance(),
                                                noteRepositoryMock.getInstance());
        model.setArtifact(artifact);
        QSignalSpy spy(&model, signal.constData());

        // WHEN
        model.setProperty(propertyName, propertyValue);

        // THEN
        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().takeFirst(), propertyValue);
        QCOMPARE(model.property(propertyName), propertyValue);
        QVERIFY(artifact->property(propertyName) != propertyValue);
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::update).when(task).exactly(0));
        QVERIFY(noteRepositoryMock(&Domain::NoteRepository::save).when(note).exactly(0));

        // WHEN (apply immediately)
        model.setArtifact(Domain::Task::Ptr::create());

        // THEN
        QCOMPARE(artifact->property(propertyName), propertyValue);
        if (task) {
            QVERIFY(taskRepositoryMock(&Domain::TaskRepository::update).when(task).exactly(1));
        } else {
            QVERIFY(noteRepositoryMock(&Domain::NoteRepository::save).when(note).exactly(1));
        }

        // WHEN (nothing else happens after a delay)
        QTest::qWait(model.autoSaveDelay() + 50);

        // THEN
        QCOMPARE(artifact->property(propertyName), propertyValue);
        if (task) {
            QVERIFY(taskRepositoryMock(&Domain::TaskRepository::update).when(task).exactly(1));
        } else {
            QVERIFY(noteRepositoryMock(&Domain::NoteRepository::save).when(note).exactly(1));
        }
    }

    void shouldApplyChangesImmediatelyIfDeleted_data()
    {
        shouldReactToArtifactPropertyChanges_data();
    }

    void shouldApplyChangesImmediatelyIfDeleted()
    {
        // GIVEN
        QFETCH(Domain::Artifact::Ptr, artifact);
        QFETCH(QByteArray, propertyName);
        QFETCH(QVariant, propertyValue);
        QFETCH(QByteArray, signal);

        auto task = artifact.objectCast<Domain::Task>();
        auto note = artifact.objectCast<Domain::Note>();

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::update).when(task).thenReturn(new FakeJob(this));
        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;
        noteRepositoryMock(&Domain::NoteRepository::save).when(note).thenReturn(new FakeJob(this));

        auto model = new Presentation::ArtifactEditorModel(taskRepositoryMock.getInstance(),
                                                           noteRepositoryMock.getInstance());
        model->setArtifact(artifact);
        QSignalSpy spy(model, signal.constData());

        // WHEN
        model->setProperty(propertyName, propertyValue);

        // THEN
        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().takeFirst(), propertyValue);
        QCOMPARE(model->property(propertyName), propertyValue);
        QVERIFY(artifact->property(propertyName) != propertyValue);
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::update).when(task).exactly(0));
        QVERIFY(noteRepositoryMock(&Domain::NoteRepository::save).when(note).exactly(0));

        // WHEN (apply immediately)
        delete model;

        // THEN
        QCOMPARE(artifact->property(propertyName), propertyValue);
        if (task) {
            QVERIFY(taskRepositoryMock(&Domain::TaskRepository::update).when(task).exactly(1));
        } else {
            QVERIFY(noteRepositoryMock(&Domain::NoteRepository::save).when(note).exactly(1));
        }
    }

    void shouldLaunchDelegation()
    {
        // GIVEN
        auto task = Domain::Task::Ptr::create();
        auto expectedDelegate = Domain::Task::Delegate("John Doe", "john@doe.com");

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::delegate).when(task, expectedDelegate).thenReturn(new FakeJob(this));

        Presentation::ArtifactEditorModel model(taskRepositoryMock.getInstance(),
                                                Domain::NoteRepository::Ptr());
        model.setArtifact(task);

        // WHEN
        model.delegate("John Doe", "john@doe.com");

        // THEN
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::delegate).when(task, expectedDelegate).exactly(1));
        QVERIFY(!task->delegate().isValid());
    }

    void shouldGetAnErrorMessageWhenUpdateTaskFailed()
    {
        // GIVEN
        auto task = Domain::Task::Ptr::create();
        task->setTitle("Task 1");
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        taskRepositoryMock(&Domain::TaskRepository::update).when(task).thenReturn(job);
        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;

        auto model = new Presentation::ArtifactEditorModel(taskRepositoryMock.getInstance(),
                                                           noteRepositoryMock.getInstance());
        FakeErrorHandler errorHandler;
        model->setErrorHandler(&errorHandler);
        model->setArtifact(task);

        // WHEN
        model->setProperty("title", "Foo");
        delete model;

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot modify task Task 1: Foo"));
    }

    void shouldGetAnErrorMessageWhenUpdateNoteFailed()
    {
        // GIVEN
        auto note = Domain::Note::Ptr::create();
        note->setTitle("Note 1");
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, "Foo");
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;
        Utils::MockObject<Domain::NoteRepository> noteRepositoryMock;
        noteRepositoryMock(&Domain::NoteRepository::save).when(note).thenReturn(job);

        auto model = new Presentation::ArtifactEditorModel(taskRepositoryMock.getInstance(),
                                                           noteRepositoryMock.getInstance());
        FakeErrorHandler errorHandler;
        model->setErrorHandler(&errorHandler);
        model->setArtifact(note);

        // WHEN
        model->setProperty("title", "Foo");
        delete model;

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QString("Cannot modify note Note 1: Foo"));
    }
};

QTEST_MAIN(ArtifactEditorModelTest)

#include "artifacteditormodeltest.moc"
