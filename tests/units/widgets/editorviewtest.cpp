/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_gui_zanshin.h>

#include <QAbstractButton>
#include <QLabel>
#include <QListView>
#include <QPlainTextEdit>
#include <QStandardItemModel>
#include <QToolButton>

#include <KLocalizedString>

#include "domain/task.h"

#include "widgets/editorview.h"

#include "kdateedit.h"

class EditorModelStub : public QObject
{
    Q_OBJECT
public:
    EditorModelStub()
    {
        setProperty("editingInProgress", false);
        setProperty("attachmentModel", QVariant::fromValue(&attachmentModel));
    }

    void setPropertyAndSignal(const QByteArray &name, const QVariant &value)
    {
        if (property(name) == value)
            return;
        if (property("editingInProgress").toBool())
            return;

        setProperty(name, value);
        if (name == "task")
            Q_EMIT taskChanged(value.value<Domain::Task::Ptr>());
        else if (name == "text")
            Q_EMIT textChanged(value.toString());
        else if (name == "title")
            Q_EMIT titleChanged(value.toString());
        else if (name == "done")
            Q_EMIT doneChanged(value.toBool());
        else if (name == "startDate")
            Q_EMIT startDateChanged(value.toDate());
        else if (name == "dueDate")
            Q_EMIT dueDateChanged(value.toDate());
        else if (name == "recurrence")
            Q_EMIT recurrenceChanged(value.value<Domain::Task::Recurrence>());
        else
            qFatal("Unsupported property %s", name.constData());
    }

public Q_SLOTS:
    void setTask(const Domain::Task::Ptr &task) { setPropertyAndSignal("task", QVariant::fromValue(task)); }
    void setTitle(const QString &title) { setPropertyAndSignal("title", title); }
    void setText(const QString &text) { setPropertyAndSignal("text", text); }
    void setDone(bool done) { setPropertyAndSignal("done", done); }
    void setStartDate(const QDate &start) { setPropertyAndSignal("startDate", start); }
    void setDueDate(const QDate &due) { setPropertyAndSignal("dueDate", due); }
    void setRecurrence(Domain::Task::Recurrence recurrence) { setPropertyAndSignal("recurrence", QVariant::fromValue(recurrence)); }
    void makeTaskAvailable() { setTask(Domain::Task::Ptr(new Domain::Task)); }

    void addAttachment(const QString &fileName)
    {
        auto item = new QStandardItem(fileName);
        attachmentModel.appendRow(QList<QStandardItem*>() << item);
    }

    void removeAttachment(const QModelIndex &index)
    {
        if (index.isValid())
            attachmentModel.removeRows(index.row(), 1, QModelIndex());
    }

signals:
    void taskChanged(const Domain::Task::Ptr &task);
    void textChanged(const QString &text);
    void titleChanged(const QString &title);
    void doneChanged(bool done);
    void startDateChanged(const QDate &date);
    void dueDateChanged(const QDate &due);
    void recurrenceChanged(Domain::Task::Recurrence recurrence);

public:
    QStandardItemModel attachmentModel;
};

class EditorViewTest : public QObject
{
    Q_OBJECT
public:
    explicit EditorViewTest(QObject *parent = nullptr)
        : QObject(parent)
    {
        qputenv("ZANSHIN_UNIT_TEST_RUN", "1");
    }

private Q_SLOTS:
    void shouldHaveDefaultState()
    {
        Widgets::EditorView editor;

        QVERIFY(!editor.isEnabled());

        auto textEdit = editor.findChild<QPlainTextEdit*>(QStringLiteral("textEdit"));
        QVERIFY(textEdit);
        QVERIFY(textEdit->isVisibleTo(&editor));

        auto startDateEdit = editor.findChild<KPIM::KDateEdit*>(QStringLiteral("startDateEdit"));
        QVERIFY(startDateEdit);
        QVERIFY(startDateEdit->isVisibleTo(&editor));

        auto dueDateEdit = editor.findChild<KPIM::KDateEdit*>(QStringLiteral("dueDateEdit"));
        QVERIFY(dueDateEdit);
        QVERIFY(dueDateEdit->isVisibleTo(&editor));

        auto recurrenceCombo = editor.findChild<QComboBox*>(QStringLiteral("recurrenceCombo"));
        QVERIFY(recurrenceCombo);
        QVERIFY(recurrenceCombo->isVisibleTo(&editor));

        auto doneButton = editor.findChild<QAbstractButton*>(QStringLiteral("doneButton"));
        QVERIFY(doneButton);
        QVERIFY(doneButton->isVisibleTo(&editor));

        auto attachmentList = editor.findChild<QListView*>(QStringLiteral("attachmentList"));
        QVERIFY(attachmentList);
        QVERIFY(attachmentList->isVisibleTo(&editor));

        auto addAttachmentButton = editor.findChild<QToolButton*>(QStringLiteral("addAttachmentButton"));
        QVERIFY(addAttachmentButton);
        QVERIFY(addAttachmentButton->isVisibleTo(&editor));

        auto removeAttachmentButton = editor.findChild<QToolButton*>(QStringLiteral("removeAttachmentButton"));
        QVERIFY(removeAttachmentButton);
        QVERIFY(removeAttachmentButton->isVisibleTo(&editor));
    }

    void shouldNotCrashForNullModel()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.setTitle(QStringLiteral("Foo"));
        model.setText(QStringLiteral("Bar"));
        editor.setModel(&model);

        auto textEdit = editor.findChild<QPlainTextEdit*>(QStringLiteral("textEdit"));
        QVERIFY(textEdit);

        auto startDateEdit = editor.findChild<KPIM::KDateEdit*>(QStringLiteral("startDateEdit"));
        QVERIFY(startDateEdit);

        auto dueDateEdit = editor.findChild<KPIM::KDateEdit*>(QStringLiteral("dueDateEdit"));
        QVERIFY(dueDateEdit);

        auto recurrenceCombo = editor.findChild<QComboBox*>(QStringLiteral("recurrenceCombo"));
        QVERIFY(recurrenceCombo);

        auto doneButton = editor.findChild<QAbstractButton*>(QStringLiteral("doneButton"));
        QVERIFY(doneButton);

        auto attachmentList = editor.findChild<QListView*>(QStringLiteral("attachmentList"));
        QVERIFY(attachmentList);
        QCOMPARE(attachmentList->model(), &model.attachmentModel);

        auto addAttachmentButton = editor.findChild<QToolButton*>(QStringLiteral("addAttachmentButton"));
        QVERIFY(addAttachmentButton);

        auto removeAttachmentButton = editor.findChild<QToolButton*>(QStringLiteral("removeAttachmentButton"));
        QVERIFY(removeAttachmentButton);

        // WHEN
        editor.setModel(nullptr);

        // THEN
        QVERIFY(!editor.isEnabled());
        QVERIFY(textEdit->toPlainText().isEmpty());
        QVERIFY(!startDateEdit->isVisibleTo(&editor));
        QVERIFY(!dueDateEdit->isVisibleTo(&editor));
        QVERIFY(!recurrenceCombo->isVisibleTo(&editor));
        QVERIFY(!doneButton->isVisibleTo(&editor));
        QVERIFY(!attachmentList->isVisibleTo(&editor));
        QVERIFY(attachmentList->model() == nullptr);
        QVERIFY(!addAttachmentButton->isVisibleTo(&editor));
        QVERIFY(!removeAttachmentButton->isVisibleTo(&editor));
    }

    void shouldBeEnabledOnlyWhenAnTaskIsAvailable()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;

        // WHEN
        editor.setModel(&model);

        // THEN
        QVERIFY(!editor.isEnabled());

        // WHEN
        // like model.makeTaskAvailable() does:
        Domain::Task::Ptr task(new Domain::Task);
        model.setPropertyAndSignal("task", QVariant::fromValue(task));

        // THEN
        QVERIFY(editor.isEnabled());

        // WHEN
        model.setPropertyAndSignal("task", QVariant::fromValue(Domain::Task::Ptr()));

        // THEN
        QVERIFY(!editor.isEnabled());



        // GIVEN
        EditorModelStub model2;
        model2.setPropertyAndSignal("task", QVariant::fromValue(task));

        // WHEN
        editor.setModel(&model2);

        // THEN
        QVERIFY(editor.isEnabled());
    }

    void shouldDisplayModelProperties()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
        model.setProperty("title", "My title");
        model.setProperty("text", "\nMy text");
        model.setProperty("startDate", QDate::currentDate());
        model.setProperty("dueDate", QDate::currentDate().addDays(2));
        model.setProperty("recurrence", QVariant::fromValue(Domain::Task::RecursWeekly));
        model.setProperty("done", true);

        auto textEdit = editor.findChild<QPlainTextEdit*>(QStringLiteral("textEdit"));
        auto startDateEdit = editor.findChild<KPIM::KDateEdit*>(QStringLiteral("startDateEdit"));
        auto dueDateEdit = editor.findChild<KPIM::KDateEdit*>(QStringLiteral("dueDateEdit"));
        auto recurrenceCombo = editor.findChild<QComboBox*>(QStringLiteral("recurrenceCombo"));
        auto doneButton = editor.findChild<QAbstractButton*>(QStringLiteral("doneButton"));

        // WHEN
        editor.setModel(&model);

        // THEN
        QCOMPARE(textEdit->toPlainText(), QString(model.property("title").toString()
                                                + '\n'
                                                + model.property("text").toString()));
        QCOMPARE(startDateEdit->date(), model.property("startDate").toDate());
        QCOMPARE(dueDateEdit->date(), model.property("dueDate").toDate());
        QCOMPARE(recurrenceCombo->currentData().value<Domain::Task::Recurrence>(), model.property("recurrence").value<Domain::Task::Recurrence>());
        QCOMPARE(doneButton->isChecked(), model.property("done").toBool());
    }

    void shouldNotReactToChangesWhileEditing()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
        model.setProperty("title", "My title");
        model.setProperty("text", "\nMy text");
        model.setProperty("startDate", QDate::currentDate());
        model.setProperty("dueDate", QDate::currentDate().addDays(2));
        model.setProperty("recurrence", QVariant::fromValue(Domain::Task::RecursWeekly));
        model.setProperty("done", true);
        editor.setModel(&model);

        auto textEdit = editor.findChild<QPlainTextEdit*>(QStringLiteral("textEdit"));
        auto startDateEdit = editor.findChild<KPIM::KDateEdit*>(QStringLiteral("startDateEdit"));
        auto dueDateEdit = editor.findChild<KPIM::KDateEdit*>(QStringLiteral("dueDateEdit"));
        auto recurrenceCombo = editor.findChild<QComboBox*>(QStringLiteral("recurrenceCombo"));
        auto doneButton = editor.findChild<QAbstractButton*>(QStringLiteral("doneButton"));
        editor.setModel(&model);

        // WHEN
        editor.show();
        QVERIFY(QTest::qWaitForWindowExposed(&editor));
        editor.activateWindow();
        textEdit->setFocus();
        model.setTitle("New title");
        model.setText("New text");
        startDateEdit->setFocus();
        model.setStartDate(QDate::currentDate().addDays(1));
        dueDateEdit->setFocus();
        model.setDueDate(QDate::currentDate().addDays(3));
        recurrenceCombo->setFocus();
        model.setRecurrence(Domain::Task::RecursDaily);
        doneButton->setFocus();
        model.setDone(false);

        // THEN (nothing changed)
        QCOMPARE(textEdit->toPlainText(), QStringLiteral("My title\n\nMy text"));
        QCOMPARE(startDateEdit->date(), QDate::currentDate());
        QCOMPARE(dueDateEdit->date(), QDate::currentDate().addDays(2));
        QCOMPARE(recurrenceCombo->currentData().value<Domain::Task::Recurrence>(), Domain::Task::RecursWeekly);
        QVERIFY(doneButton->isChecked());
    }

    void shouldReactToTitleChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
        model.setProperty("title", "My title");
        model.setProperty("text", "\nMy text");
        model.setProperty("startDate", QDate::currentDate());
        model.setProperty("dueDate", QDate::currentDate().addDays(2));
        model.setProperty("done", true);
        editor.setModel(&model);

        auto textEdit = editor.findChild<QPlainTextEdit*>(QStringLiteral("textEdit"));

        // WHEN
        model.setPropertyAndSignal("title", "New title");

        // THEN
        QCOMPARE(textEdit->toPlainText(), QString(model.property("title").toString()
                                                + '\n'
                                                + model.property("text").toString()));
    }

    void shouldReactToTextChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
        model.setProperty("title", "My title");
        model.setProperty("text", "\nMy text");
        model.setProperty("startDate", QDate::currentDate());
        model.setProperty("dueDate", QDate::currentDate().addDays(2));
        model.setProperty("done", true);
        editor.setModel(&model);

        auto textEdit = editor.findChild<QPlainTextEdit*>(QStringLiteral("textEdit"));

        // WHEN
        model.setPropertyAndSignal("text", "\nNew text");

        // THEN
        QCOMPARE(textEdit->toPlainText(), QString(model.property("title").toString()
                                                + '\n'
                                                + model.property("text").toString()));
    }

    void shouldApplyTextEditChanges_data()
    {
        QTest::addColumn<QString>("plainText");
        QTest::addColumn<QString>("expectedTitle");
        QTest::addColumn<QString>("expectedText");

        QTest::newRow("nominal case") << "Title\n\nText" << "Title" << "\nText";
        QTest::newRow("single line") << "Title" << "Title" << "";
    }

    void shouldApplyTextEditChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
        editor.setModel(&model);

        auto textEdit = editor.findChild<QPlainTextEdit*>(QStringLiteral("textEdit"));

        // WHEN
        QFETCH(QString, plainText);
        textEdit->setPlainText(plainText);

        // THEN
        QFETCH(QString, expectedTitle);
        QCOMPARE(model.property("title").toString(), expectedTitle);
        QFETCH(QString, expectedText);
        QCOMPARE(model.property("text").toString(), expectedText);
    }

    void shouldReactToDoneChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
        model.setProperty("title", "My title");
        model.setProperty("text", "\nMy text");
        model.setProperty("startDate", QDate::currentDate());
        model.setProperty("dueDate", QDate::currentDate().addDays(2));
        model.setProperty("done", false);
        editor.setModel(&model);

        auto doneButton = editor.findChild<QAbstractButton*>(QStringLiteral("doneButton"));
        QVERIFY(!doneButton->isChecked());

        // WHEN
        model.setPropertyAndSignal("done", true);

        // THEN
        QVERIFY(doneButton->isChecked());
    }

    void shouldApplyDoneButtonChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
        editor.setModel(&model);

        auto doneButton = editor.findChild<QAbstractButton*>(QStringLiteral("doneButton"));

        // WHEN
        doneButton->setChecked(true);

        // THEN
        QCOMPARE(model.property("done").toBool(), true);
    }

    void shouldReactToStartDateChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
        model.setProperty("title", "My title");
        model.setProperty("text", "\nMy text");
        model.setProperty("startDate", QDate::currentDate());
        model.setProperty("dueDate", QDate::currentDate().addDays(2));
        model.setProperty("done", false);
        editor.setModel(&model);

        auto startDateEdit = editor.findChild<KPIM::KDateEdit*>(QStringLiteral("startDateEdit"));

        // WHEN
        model.setPropertyAndSignal("startDate", QDate::currentDate().addDays(-2));

        // THEN
        QCOMPARE(startDateEdit->date(), model.property("startDate").toDate());
    }

    void shouldApplyStartDateEditChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
        editor.setModel(&model);

        auto startDateEdit = editor.findChild<KPIM::KDateEdit*>(QStringLiteral("startDateEdit"));
        auto today = QDate::currentDate();

        // WHEN
        startDateEdit->setEditText(today.toString(QStringLiteral("dd/MM/yyyy")));
        QTest::keyClick(startDateEdit, Qt::Key_Enter);

        // THEN
        const QDate newStartDate = model.property("startDate").toDate();
        QCOMPARE(newStartDate, today);
    }

    void shouldReactToDueDateChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.setProperty("title", "My title");
        model.setProperty("text", "\nMy text");
        model.setProperty("startDate", QDate::currentDate());
        model.setProperty("dueDate", QDate::currentDate().addDays(2));
        model.setProperty("done", false);
        editor.setModel(&model);

        auto dueDateEdit = editor.findChild<KPIM::KDateEdit*>(QStringLiteral("dueDateEdit"));

        // WHEN
        model.setPropertyAndSignal("dueDate", QDate::currentDate().addDays(-2));

        // THEN
        QCOMPARE(dueDateEdit->date(), model.property("dueDate").toDate());
    }

    void shouldApplyDueDateEditChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
        editor.setModel(&model);

        auto dueDateEdit = editor.findChild<KPIM::KDateEdit*>(QStringLiteral("dueDateEdit"));
        auto today = QDate::currentDate();

        // WHEN
        QVERIFY(dueDateEdit->isEnabled());
        dueDateEdit->setEditText(today.toString(QStringLiteral("dd/MM/yyyy")));
        QTest::keyClick(dueDateEdit, Qt::Key_Enter);

        // THEN
        QCOMPARE(model.property("dueDate").toDate(), today);
    }

    void shouldApplyStartTodayChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
        editor.setModel(&model);

        QAbstractButton *startTodayButton = editor.findChild<QAbstractButton *>(QStringLiteral("startTodayButton"));
        QVERIFY(startTodayButton);
        auto startDateEdit = editor.findChild<KPIM::KDateEdit*>(QStringLiteral("startDateEdit"));
        auto today = QDate::currentDate();

        // WHEN
        QVERIFY(startTodayButton->isEnabled());
        startTodayButton->click();

        // THEN
        QCOMPARE(startDateEdit->currentText(), today.toString(QStringLiteral("dd/MM/yyyy")));
        QCOMPARE(model.property("startDate").toDate(), today);
    }

    void shouldReactToRecurrenceChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
        model.setProperty("title", "My title");
        model.setProperty("text", "\nMy text");
        model.setProperty("startDate", QDate::currentDate());
        model.setProperty("dueDate", QDate::currentDate().addDays(2));
        model.setProperty("recurrence", QVariant::fromValue(Domain::Task::RecursWeekly));
        model.setProperty("done", false);
        editor.setModel(&model);

        auto recurrenceCombo = editor.findChild<QComboBox*>(QStringLiteral("recurrenceCombo"));

        // WHEN
        model.setPropertyAndSignal("recurrence", Domain::Task::RecursMonthly);

        // THEN
        QCOMPARE(recurrenceCombo->currentData().value<Domain::Task::Recurrence>(), model.property("recurrence").value<Domain::Task::Recurrence>());
    }

    void shouldApplyRecurrenceComboChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
        editor.setModel(&model);

        auto recurrenceCombo = editor.findChild<QComboBox*>(QStringLiteral("recurrenceCombo"));

        // WHEN
        recurrenceCombo->setCurrentIndex(2); // Weekly

        // THEN
        QCOMPARE(model.property("recurrence").value<Domain::Task::Recurrence>(), Domain::Task::RecursWeekly);
    }

    void shouldAddAttachments()
    {
        // GIVEN
        Widgets::EditorView editor;
        editor.setRequestFileNameFunction([](QWidget*) { return "/tmp/foobar"; });
        EditorModelStub model;
        model.makeTaskAvailable();
        editor.setModel(&model);

        auto addAttachmentButton = editor.findChild<QToolButton*>(QStringLiteral("addAttachmentButton"));

        // WHEN
        addAttachmentButton->click();

        // THEN
        QCOMPARE(model.attachmentModel.rowCount(), 1);
        QCOMPARE(model.attachmentModel.data(model.attachmentModel.index(0, 0)).toString(),
                 QStringLiteral("/tmp/foobar"));
    }

    void shouldRemoveAttachments()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
        model.addAttachment("/tmp/foo");
        model.addAttachment("/tmp/bar");
        editor.setModel(&model);

        auto attachmentList = editor.findChild<QListView*>(QStringLiteral("attachmentList"));
        auto removeAttachmentButton = editor.findChild<QToolButton*>(QStringLiteral("removeAttachmentButton"));

        // THEN
        QVERIFY(!removeAttachmentButton->isEnabled());

        // WHEN
        attachmentList->selectionModel()->select(model.attachmentModel.index(0, 0), QItemSelectionModel::ClearAndSelect);

        // THEN
        QVERIFY(removeAttachmentButton->isEnabled());

        // WHEN
        removeAttachmentButton->click();

        // THEN
        QCOMPARE(model.attachmentModel.rowCount(), 1);
        QCOMPARE(model.attachmentModel.data(model.attachmentModel.index(0, 0)).toString(),
                 QStringLiteral("/tmp/bar"));
    }
};

ZANSHIN_TEST_MAIN(EditorViewTest)

#include "editorviewtest.moc"
