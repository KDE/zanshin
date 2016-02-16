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

#include "utils/mockobject.h"

#include <QSignalSpy>

#include "testlib/fakejob.h"

#include "domain/task.h"
#include "domain/note.h"

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
        Presentation::ArtifactEditorModel model;

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
        QVERIFY(!model.hasSaveFunction());
        QVERIFY(!model.hasDelegateFunction());
    }

    void shouldHaveTaskProperties()
    {
        // GIVEN
        Presentation::ArtifactEditorModel model;
        QSignalSpy textSpy(&model, &Presentation::ArtifactEditorModel::textChanged);
        QSignalSpy titleSpy(&model, &Presentation::ArtifactEditorModel::titleChanged);
        QSignalSpy doneSpy(&model, &Presentation::ArtifactEditorModel::doneChanged);
        QSignalSpy startSpy(&model, &Presentation::ArtifactEditorModel::startDateChanged);
        QSignalSpy dueSpy(&model, &Presentation::ArtifactEditorModel::dueDateChanged);
        QSignalSpy delegateSpy(&model, &Presentation::ArtifactEditorModel::delegateTextChanged);

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
        Presentation::ArtifactEditorModel model;
        QSignalSpy textSpy(&model, &Presentation::ArtifactEditorModel::textChanged);
        QSignalSpy titleSpy(&model, &Presentation::ArtifactEditorModel::titleChanged);
        QSignalSpy doneSpy(&model, &Presentation::ArtifactEditorModel::doneChanged);
        QSignalSpy startSpy(&model, &Presentation::ArtifactEditorModel::startDateChanged);
        QSignalSpy dueSpy(&model, &Presentation::ArtifactEditorModel::dueDateChanged);
        QSignalSpy delegateSpy(&model, &Presentation::ArtifactEditorModel::delegateTextChanged);

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

        Presentation::ArtifactEditorModel model;
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
        Presentation::ArtifactEditorModel model;
        model.setArtifact(task);
        QSignalSpy spy(&model, &Presentation::ArtifactEditorModel::delegateTextChanged);

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

        auto savedArtifact = Domain::Artifact::Ptr();
        auto save = [this, &savedArtifact] (const Domain::Artifact::Ptr &artifact) {
            savedArtifact = artifact;
            return new FakeJob(this);
        };

        Presentation::ArtifactEditorModel model;
        model.setSaveFunction(save);
        model.setArtifact(artifact);
        QSignalSpy spy(&model, signal.constData());

        // WHEN
        model.setProperty(propertyName, propertyValue);

        // THEN
        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().takeFirst(), propertyValue);
        QCOMPARE(model.property(propertyName), propertyValue);
        QVERIFY(artifact->property(propertyName) != propertyValue);
        QVERIFY(!savedArtifact);

        // WHEN (apply after delay)
        QTest::qWait(model.autoSaveDelay() + 50);

        // THEN
        QCOMPARE(savedArtifact, artifact);
        QCOMPARE(artifact->property(propertyName), propertyValue);
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

        auto savedArtifact = Domain::Artifact::Ptr();
        auto save = [this, &savedArtifact] (const Domain::Artifact::Ptr &artifact) {
            savedArtifact = artifact;
            return new FakeJob(this);
        };

        Presentation::ArtifactEditorModel model;
        model.setSaveFunction(save);
        QVERIFY(model.hasSaveFunction());
        model.setArtifact(artifact);
        QSignalSpy spy(&model, signal.constData());

        // WHEN
        model.setProperty(propertyName, propertyValue);

        // THEN
        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().takeFirst(), propertyValue);
        QCOMPARE(model.property(propertyName), propertyValue);
        QVERIFY(artifact->property(propertyName) != propertyValue);
        QVERIFY(!savedArtifact);

        // WHEN (apply immediately)
        model.setArtifact(Domain::Task::Ptr::create());

        // THEN
        QCOMPARE(savedArtifact, artifact);
        QCOMPARE(artifact->property(propertyName), propertyValue);
        savedArtifact.clear();

        // WHEN (nothing else happens after a delay)
        QTest::qWait(model.autoSaveDelay() + 50);

        // THEN
        QVERIFY(!savedArtifact);
        QCOMPARE(artifact->property(propertyName), propertyValue);
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

        auto savedArtifact = Domain::Artifact::Ptr();
        auto save = [this, &savedArtifact] (const Domain::Artifact::Ptr &artifact) {
            savedArtifact = artifact;
            return new FakeJob(this);
        };

        auto model = new Presentation::ArtifactEditorModel;
        model->setSaveFunction(save);
        QVERIFY(model->hasSaveFunction());
        model->setArtifact(artifact);
        QSignalSpy spy(model, signal.constData());

        // WHEN
        model->setProperty(propertyName, propertyValue);

        // THEN
        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().takeFirst(), propertyValue);
        QCOMPARE(model->property(propertyName), propertyValue);
        QVERIFY(artifact->property(propertyName) != propertyValue);
        QVERIFY(!savedArtifact);

        // WHEN (apply immediately)
        delete model;

        // THEN
        QCOMPARE(savedArtifact, artifact);
        QCOMPARE(artifact->property(propertyName), propertyValue);
    }

    void shouldLaunchDelegation()
    {
        // GIVEN
        auto task = Domain::Task::Ptr::create();
        auto expectedDelegate = Domain::Task::Delegate("John Doe", "john@doe.com");

        auto delegatedTask = Domain::Task::Ptr();
        auto delegate = Domain::Task::Delegate();
        auto delegateFunction = [this, &delegatedTask, &delegate] (const Domain::Task::Ptr &task, const Domain::Task::Delegate &d) {
            delegatedTask = task;
            delegate = d;
            return new FakeJob(this);
        };

        Presentation::ArtifactEditorModel model;
        model.setDelegateFunction(delegateFunction);
        QVERIFY(model.hasDelegateFunction());
        model.setArtifact(task);

        // WHEN
        model.delegate("John Doe", "john@doe.com");

        // THEN
        QCOMPARE(delegatedTask, task);
        QCOMPARE(delegate, expectedDelegate);
        QVERIFY(!task->delegate().isValid());
    }

    void shouldGetAnErrorMessageWhenSaveFailed()
    {
        // GIVEN
        auto task = Domain::Task::Ptr::create();
        task->setTitle("Task 1");

        auto savedArtifact = Domain::Artifact::Ptr();
        auto save = [this, &savedArtifact] (const Domain::Artifact::Ptr &artifact) {
            savedArtifact = artifact;
            auto job = new FakeJob(this);
            job->setExpectedError(KJob::KilledJobError, "Foo");
            return job;
        };

        auto model = new Presentation::ArtifactEditorModel;
        model->setSaveFunction(save);
        QVERIFY(model->hasSaveFunction());
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
};

ZANSHIN_TEST_MAIN(ArtifactEditorModelTest)

#include "artifacteditormodeltest.moc"
