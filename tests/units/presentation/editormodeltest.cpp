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
#include <QTemporaryFile>

#include "testlib/fakejob.h"

#include "domain/task.h"

#include "presentation/editormodel.h"
#include "presentation/errorhandler.h"

using namespace mockitopp;

class FakeErrorHandler : public Presentation::ErrorHandler
{
public:
    void doDisplayMessage(const QString &message) override
    {
        m_message = message;
    }

    QString m_message;
};

class EditorModelTest : public QObject
{
    Q_OBJECT
public:
    explicit EditorModelTest(QObject *parent = nullptr)
        : QObject(parent)
    {
        qRegisterMetaType<Domain::Task::Recurrence>();
        Presentation::EditorModel::setAutoSaveDelay(50);
    }

private slots:
    void shouldHaveEmptyDefaultState()
    {
        // GIVEN
        Presentation::EditorModel model;

        // WHEN
        // Nothing

        // THEN
        QVERIFY(model.task().isNull());
        QVERIFY(model.text().isEmpty());
        QVERIFY(model.title().isEmpty());
        QVERIFY(!model.isDone());
        QVERIFY(model.startDate().isNull());
        QVERIFY(model.dueDate().isNull());
        QCOMPARE(model.recurrence(), Domain::Task::NoRecurrence);
        QVERIFY(model.attachmentModel() != nullptr);
        QVERIFY(!model.hasSaveFunction());
        auto am = model.attachmentModel();
        QCOMPARE(am->rowCount(), 0);
    }

    void shouldHaveTaskProperties()
    {
        // GIVEN
        Presentation::EditorModel model;
        QSignalSpy textSpy(&model, &Presentation::EditorModel::textChanged);
        QSignalSpy titleSpy(&model, &Presentation::EditorModel::titleChanged);
        QSignalSpy doneSpy(&model, &Presentation::EditorModel::doneChanged);
        QSignalSpy startSpy(&model, &Presentation::EditorModel::startDateChanged);
        QSignalSpy dueSpy(&model, &Presentation::EditorModel::dueDateChanged);
        QSignalSpy recurrenceSpy(&model, &Presentation::EditorModel::recurrenceChanged);
        QSignalSpy attachmentSpy(model.attachmentModel(), &QAbstractItemModel::modelReset);

        Domain::Task::Attachments attachments;

        Domain::Task::Attachment dataAttachment;
        dataAttachment.setData("foo");
        dataAttachment.setLabel("dataAttachment");
        dataAttachment.setMimeType("text/plain");
        dataAttachment.setIconName("text-plain");
        attachments.append(dataAttachment);

        Domain::Task::Attachment uriAttachment;
        uriAttachment.setUri(QUrl("https://www.kde.org"));
        uriAttachment.setLabel("uriAttachment");
        uriAttachment.setMimeType("text/html");
        uriAttachment.setIconName("text-html");
        attachments.append(uriAttachment);

        auto task = Domain::Task::Ptr::create();
        task->setText(QStringLiteral("description"));
        task->setTitle(QStringLiteral("title"));
        task->setDone(true);
        task->setStartDate(QDate::currentDate());
        task->setDueDate(QDate::currentDate().addDays(2));
        task->setRecurrence(Domain::Task::RecursDaily);
        task->setAttachments(attachments);

        // WHEN
        model.setTask(task);
        // To make sure we don't signal too much
        model.setText(task->text());
        model.setTitle(task->title());
        model.setDone(task->isDone());
        model.setStartDate(task->startDate());
        model.setDueDate(task->dueDate());
        model.setRecurrence(task->recurrence());

        // THEN
        QCOMPARE(textSpy.size(), 1);
        QCOMPARE(textSpy.takeFirst().at(0).toString(), task->text());
        QCOMPARE(model.property("text").toString(), task->text());

        QCOMPARE(titleSpy.size(), 1);
        QCOMPARE(titleSpy.takeFirst().at(0).toString(), task->title());
        QCOMPARE(model.property("title").toString(), task->title());

        QCOMPARE(doneSpy.size(), 1);
        QCOMPARE(doneSpy.takeFirst().at(0).toBool(), task->isDone());
        QCOMPARE(model.property("done").toBool(), task->isDone());

        QCOMPARE(startSpy.size(), 1);
        QCOMPARE(startSpy.takeFirst().at(0).toDate(), task->startDate());
        QCOMPARE(model.property("startDate").toDate(), task->startDate());

        QCOMPARE(dueSpy.size(), 1);
        QCOMPARE(dueSpy.takeFirst().at(0).toDate(), task->dueDate());
        QCOMPARE(model.property("dueDate").toDate(), task->dueDate());

        QCOMPARE(recurrenceSpy.size(), 1);
        QCOMPARE(recurrenceSpy.takeFirst().at(0).value<Domain::Task::Recurrence>(), task->recurrence());
        QCOMPARE(model.property("recurrence").value<Domain::Task::Recurrence>(), task->recurrence());

        QCOMPARE(attachmentSpy.size(), 1);
        auto am = model.attachmentModel();
        QCOMPARE(am->rowCount(), 2);
        QCOMPARE(am->data(am->index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("dataAttachment"));
        QCOMPARE(am->data(am->index(0, 0), Qt::DecorationRole).value<QIcon>(), QIcon::fromTheme("text-plain"));
        QCOMPARE(am->data(am->index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("uriAttachment"));
        QCOMPARE(am->data(am->index(1, 0), Qt::DecorationRole).value<QIcon>(), QIcon::fromTheme("text-html"));
    }

    void shouldReactToTaskPropertyChanges_data()
    {
        QTest::addColumn<Domain::Task::Ptr>("task");
        QTest::addColumn<QByteArray>("propertyName");
        QTest::addColumn<QVariant>("propertyValue");
        QTest::addColumn<QByteArray>("signal");

        QTest::newRow("task text") << Domain::Task::Ptr(Domain::Task::Ptr::create())
                                   << QByteArray("text")
                                   << QVariant("new text")
                                   << QByteArray(SIGNAL(textChanged(QString)));

        QTest::newRow("task title") << Domain::Task::Ptr(Domain::Task::Ptr::create())
                                    << QByteArray("title")
                                    << QVariant("new title")
                                    << QByteArray(SIGNAL(titleChanged(QString)));

        QTest::newRow("task done") << Domain::Task::Ptr(Domain::Task::Ptr::create())
                                   << QByteArray("done")
                                   << QVariant(true)
                                   << QByteArray(SIGNAL(doneChanged(bool)));

        QTest::newRow("task start") << Domain::Task::Ptr(Domain::Task::Ptr::create())
                                    << QByteArray("startDate")
                                    << QVariant(QDate::currentDate())
                                    << QByteArray(SIGNAL(startDateChanged(QDate)));

        QTest::newRow("task due") << Domain::Task::Ptr(Domain::Task::Ptr::create())
                                  << QByteArray("dueDate")
                                  << QVariant(QDate::currentDate().addDays(2))
                                  << QByteArray(SIGNAL(dueDateChanged(QDate)));

        QTest::newRow("task recurrence") << Domain::Task::Ptr(Domain::Task::Ptr::create())
                                  << QByteArray("recurrence")
                                  << QVariant::fromValue(Domain::Task::RecursDaily)
                                  << QByteArray(SIGNAL(recurrenceChanged(Domain::Task::Recurrence)));
    }

    void shouldReactToTaskPropertyChanges()
    {
        // GIVEN
        QFETCH(Domain::Task::Ptr, task);
        QFETCH(QByteArray, propertyName);
        QFETCH(QVariant, propertyValue);
        QFETCH(QByteArray, signal);

        Presentation::EditorModel model;
        model.setTask(task);
        QSignalSpy spy(&model, signal.constData());

        // WHEN
        task->setProperty(propertyName, propertyValue);

        // THEN
        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().at(0), propertyValue);
        QCOMPARE(model.property(propertyName), propertyValue);
    }

    void shouldNotReactToTaskPropertyChangesWhenEditing_data()
    {
        shouldReactToTaskPropertyChanges_data();
    }

    void shouldNotReactToTaskPropertyChangesWhenEditing()
    {
        // GIVEN
        QFETCH(Domain::Task::Ptr, task);
        QFETCH(QByteArray, propertyName);
        QFETCH(QVariant, propertyValue);
        QFETCH(QByteArray, signal);

        Presentation::EditorModel model;
        model.setTask(task);
        QSignalSpy spy(&model, signal.constData());

        // WHEN
        const auto oldPropertyValue = task->property(propertyName);
        model.setEditingInProgress(true);
        task->setProperty(propertyName, propertyValue);

        // THEN
        QVERIFY(spy.isEmpty());
        QCOMPARE(model.property(propertyName), oldPropertyValue);
    }

    void shouldApplyChangesBackToTaskAfterADelay_data()
    {
        shouldReactToTaskPropertyChanges_data();
    }

    void shouldApplyChangesBackToTaskAfterADelay()
    {
        // GIVEN
        QFETCH(Domain::Task::Ptr, task);
        QFETCH(QByteArray, propertyName);
        QFETCH(QVariant, propertyValue);
        QFETCH(QByteArray, signal);

        auto savedTask = Domain::Task::Ptr();
        auto save = [this, &savedTask] (const Domain::Task::Ptr &task) {
            savedTask = task;
            return new FakeJob(this);
        };

        Presentation::EditorModel model;
        model.setSaveFunction(save);
        model.setTask(task);
        QSignalSpy spy(&model, signal.constData());

        // WHEN
        model.setProperty(propertyName, propertyValue);

        // THEN
        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().at(0), propertyValue);
        QCOMPARE(model.property(propertyName), propertyValue);
        QVERIFY(task->property(propertyName) != propertyValue);
        QVERIFY(!savedTask);

        // WHEN (apply after delay)
        QTest::qWait(model.autoSaveDelay() + 50);

        // THEN
        QCOMPARE(savedTask, task);
        QCOMPARE(task->property(propertyName), propertyValue);
    }

    void shouldApplyChangesImmediatelyIfANewTaskIsSet_data()
    {
        shouldReactToTaskPropertyChanges_data();
    }

    void shouldApplyChangesImmediatelyIfANewTaskIsSet()
    {
        // GIVEN
        QFETCH(Domain::Task::Ptr, task);
        QFETCH(QByteArray, propertyName);
        QFETCH(QVariant, propertyValue);
        QFETCH(QByteArray, signal);

        auto savedTask = Domain::Task::Ptr();
        auto save = [this, &savedTask] (const Domain::Task::Ptr &task) {
            savedTask = task;
            return new FakeJob(this);
        };

        Presentation::EditorModel model;
        model.setSaveFunction(save);
        QVERIFY(model.hasSaveFunction());
        model.setTask(task);
        QSignalSpy spy(&model, signal.constData());

        // WHEN
        model.setProperty(propertyName, propertyValue);

        // THEN
        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().at(0), propertyValue);
        QCOMPARE(model.property(propertyName), propertyValue);
        QVERIFY(task->property(propertyName) != propertyValue);
        QVERIFY(!savedTask);

        // WHEN (apply immediately)
        model.setTask(Domain::Task::Ptr::create());

        // THEN
        QCOMPARE(savedTask, task);
        QCOMPARE(task->property(propertyName), propertyValue);
        savedTask.clear();

        // WHEN (nothing else happens after a delay)
        QTest::qWait(model.autoSaveDelay() + 50);

        // THEN
        QVERIFY(!savedTask);
        QCOMPARE(task->property(propertyName), propertyValue);
    }

    void shouldApplyChangesImmediatelyIfDeleted_data()
    {
        shouldReactToTaskPropertyChanges_data();
    }

    void shouldApplyChangesImmediatelyIfDeleted()
    {
        // GIVEN
        QFETCH(Domain::Task::Ptr, task);
        QFETCH(QByteArray, propertyName);
        QFETCH(QVariant, propertyValue);
        QFETCH(QByteArray, signal);

        auto savedTask = Domain::Task::Ptr();
        auto save = [this, &savedTask] (const Domain::Task::Ptr &task) {
            savedTask = task;
            return new FakeJob(this);
        };

        auto model = new Presentation::EditorModel;
        model->setSaveFunction(save);
        QVERIFY(model->hasSaveFunction());
        model->setTask(task);
        QSignalSpy spy(model, signal.constData());

        // WHEN
        model->setProperty(propertyName, propertyValue);

        // THEN
        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.takeFirst().at(0), propertyValue);
        QCOMPARE(model->property(propertyName), propertyValue);
        QVERIFY(task->property(propertyName) != propertyValue);
        QVERIFY(!savedTask);

        // WHEN (apply immediately)
        delete model;

        // THEN
        QCOMPARE(savedTask, task);
        QCOMPARE(task->property(propertyName), propertyValue);
    }

    void shouldGetAnErrorMessageWhenSaveFailed()
    {
        // GIVEN
        auto task = Domain::Task::Ptr::create();
        task->setTitle(QStringLiteral("Task 1"));

        auto savedTask = Domain::Task::Ptr();
        auto save = [this, &savedTask] (const Domain::Task::Ptr &task) {
            savedTask = task;
            auto job = new FakeJob(this);
            job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
            return job;
        };

        auto model = new Presentation::EditorModel;
        model->setSaveFunction(save);
        QVERIFY(model->hasSaveFunction());
        FakeErrorHandler errorHandler;
        model->setErrorHandler(&errorHandler);
        model->setTask(task);

        // WHEN
        model->setProperty("title", "Foo");
        delete model;

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot modify task Task 1: Foo"));
    }

    void shouldDisconnectFromPreviousTask_data()
    {
        shouldReactToTaskPropertyChanges_data();
    }

    void shouldDisconnectFromPreviousTask()
    {
        // GIVEN
        QFETCH(Domain::Task::Ptr, task);
        QFETCH(QByteArray, propertyName);
        QFETCH(QVariant, propertyValue);
        QFETCH(QByteArray, signal);

        Presentation::EditorModel model;
        model.setTask(task);
        QSignalSpy spy(&model, signal.constData());

        Domain::Task::Ptr newTask = Domain::Task::Ptr::create();

        // WHEN
        model.setTask(newTask);
        // modifying the *old* task should have no effect.
        task->setProperty(propertyName, propertyValue);

        // THEN
        QCOMPARE(spy.size(), 1); // emitted by setTask
        QVERIFY(model.property(propertyName) != task->property(propertyName));
    }

    void shouldAddAttachments()
    {
        // GIVEN
        QTemporaryFile temporaryFile(QDir::tempPath() + "/taskeditormodeltest_XXXXXX.txt");
        temporaryFile.open();
        temporaryFile.write("foo bar");
        temporaryFile.close();
        auto fileName = temporaryFile.fileName().mid(QDir::tempPath().size() + 1);

        auto task = Domain::Task::Ptr::create();

        auto savedTask = Domain::Task::Ptr();
        auto save = [this, &savedTask] (const Domain::Task::Ptr &task) {
            savedTask = task;
            return new FakeJob(this);
        };

        Presentation::EditorModel model;
        model.setSaveFunction(save);
        model.setTask(task);

        QSignalSpy spy(model.attachmentModel(), &QAbstractItemModel::modelReset);

        // WHEN
        model.addAttachment(temporaryFile.fileName());

        // THEN
        QCOMPARE(spy.size(), 1);
        QCOMPARE(model.attachmentModel()->rowCount(), 1);
        QVERIFY(!savedTask);

        // WHEN (nothing else happens after a delay)
        QTest::qWait(model.autoSaveDelay() + 50);

        // THEN
        QCOMPARE(savedTask, task);
        QCOMPARE(task->attachments().size(), 1);
        QCOMPARE(task->attachments().first().label(), fileName);
        QCOMPARE(task->attachments().first().mimeType(), QStringLiteral("text/plain"));
        QCOMPARE(task->attachments().first().iconName(), QStringLiteral("text-plain"));
        QCOMPARE(task->attachments().first().data(), QByteArrayLiteral("foo bar"));
    }

    void shouldRemoveAttachments()
    {
        // GIVEN
        auto task = Domain::Task::Ptr::create();
        task->setAttachments(Domain::Task::Attachments() << Domain::Task::Attachment("foo")
                                                         << Domain::Task::Attachment("bar"));

        auto savedTask = Domain::Task::Ptr();
        auto save = [this, &savedTask] (const Domain::Task::Ptr &task) {
            savedTask = task;
            return new FakeJob(this);
        };

        Presentation::EditorModel model;
        model.setSaveFunction(save);
        model.setTask(task);

        QSignalSpy spy(model.attachmentModel(), &QAbstractItemModel::modelReset);

        // WHEN
        model.removeAttachment(model.attachmentModel()->index(0, 0));

        // THEN
        QCOMPARE(spy.size(), 1);
        QCOMPARE(model.attachmentModel()->rowCount(), 1);
        QVERIFY(!savedTask);

        // WHEN (nothing else happens after a delay)
        QTest::qWait(model.autoSaveDelay() + 50);

        // THEN
        QCOMPARE(savedTask, task);
        QCOMPARE(task->attachments().size(), 1);
        QCOMPARE(task->attachments().first().data(), QByteArrayLiteral("bar"));
    }
};

ZANSHIN_TEST_MAIN(EditorModelTest)

#include "editormodeltest.moc"
