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

#include <QtTestGui>

#include <QAbstractButton>
#include <QPlainTextEdit>

#include "domain/note.h"
#include "domain/task.h"

#include "pimlibs/kdateedit.h"
#include "widgets/editorview.h"

class EditorModelStub : public QObject
{
    Q_OBJECT
public:
    void setPropertyAndSignal(const QByteArray &name, const QVariant &value)
    {
        if (property(name) == value)
            return;

        setProperty(name, value);
        if (name == "text")
            emit textChanged(value.toString());
        else if (name == "title")
            emit titleChanged(value.toString());
        else if (name == "done")
            emit doneChanged(value.toBool());
        else if (name == "startDate")
            emit startDateChanged(value.toDateTime());
        else if (name == "dueDate")
            emit dueDateChanged(value.toDateTime());
        else if (name == "hasTaskProperties")
            emit hasTaskPropertiesChanged(value.toBool());
        else
            qFatal("Unsupported property %s", name.constData());
    }

public slots:
    void setTitle(const QString &title) { setPropertyAndSignal("title", title); }
    void setText(const QString &text) { setPropertyAndSignal("text", text); }
    void setDone(bool done) { setPropertyAndSignal("done", done); }
    void setStartDate(const QDateTime &start) { setPropertyAndSignal("startDate", start); }
    void setDueDate(const QDateTime &due) { setPropertyAndSignal("dueDate", due); }

signals:
    void hasTaskPropertiesChanged(bool hasTaskProperties);
    void textChanged(const QString &text);
    void titleChanged(const QString &title);
    void doneChanged(bool done);
    void startDateChanged(const QDateTime &date);
    void dueDateChanged(const QDateTime &due);
};

class EditorViewTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldHaveDefaultState()
    {
        Widgets::EditorView editor;

        auto textEdit = editor.findChild<QPlainTextEdit*>("textEdit");
        QVERIFY(textEdit);
        QVERIFY(textEdit->isVisibleTo(&editor));

        auto startDateEdit = editor.findChild<KPIM::KDateEdit*>("startDateEdit");
        QVERIFY(startDateEdit);
        QVERIFY(!startDateEdit->isVisibleTo(&editor));

        auto dueDateEdit = editor.findChild<KPIM::KDateEdit*>("dueDateEdit");
        QVERIFY(dueDateEdit);
        QVERIFY(!dueDateEdit->isVisibleTo(&editor));

        auto doneButton = editor.findChild<QAbstractButton*>("doneButton");
        QVERIFY(doneButton);
        QVERIFY(!doneButton->isVisibleTo(&editor));
    }

    void shouldShowTaskPropertiesEditorsOnlyForTasks()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.setPropertyAndSignal("hasTaskProperties", true);

        auto startDateEdit = editor.findChild<KPIM::KDateEdit*>("startDateEdit");
        QVERIFY(!startDateEdit->isVisibleTo(&editor));

        auto dueDateEdit = editor.findChild<KPIM::KDateEdit*>("dueDateEdit");
        QVERIFY(!dueDateEdit->isVisibleTo(&editor));

        auto doneButton = editor.findChild<QAbstractButton*>("doneButton");
        QVERIFY(!doneButton->isVisibleTo(&editor));

        // WHEN
        editor.setModel(&model);

        // THEN
        QVERIFY(startDateEdit->isVisibleTo(&editor));
        QVERIFY(dueDateEdit->isVisibleTo(&editor));
        QVERIFY(doneButton->isVisibleTo(&editor));
    }

    void shouldReactToHasTaskPropertiesChanged()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        editor.setModel(&model);

        auto startDateEdit = editor.findChild<KPIM::KDateEdit*>("startDateEdit");
        QVERIFY(!startDateEdit->isVisibleTo(&editor));

        auto dueDateEdit = editor.findChild<KPIM::KDateEdit*>("dueDateEdit");
        QVERIFY(!dueDateEdit->isVisibleTo(&editor));

        auto doneButton = editor.findChild<QAbstractButton*>("doneButton");
        QVERIFY(!doneButton->isVisibleTo(&editor));

        // WHEN
        model.setPropertyAndSignal("hasTaskProperties", true);

        // THEN
        QVERIFY(startDateEdit->isVisibleTo(&editor));
        QVERIFY(dueDateEdit->isVisibleTo(&editor));
        QVERIFY(doneButton->isVisibleTo(&editor));
    }

    void shouldDisplayModelProperties()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.setProperty("hasTaskProperties", true);
        model.setProperty("title", "My title");
        model.setProperty("text", "\nMy text");
        model.setProperty("startDate", QDateTime::currentDateTime());
        model.setProperty("dueDate", QDateTime::currentDateTime().addDays(2));
        model.setProperty("done", true);

        auto textEdit = editor.findChild<QPlainTextEdit*>("textEdit");
        auto startDateEdit = editor.findChild<KPIM::KDateEdit*>("startDateEdit");
        auto dueDateEdit = editor.findChild<KPIM::KDateEdit*>("dueDateEdit");
        auto doneButton = editor.findChild<QAbstractButton*>("doneButton");

        // WHEN
        editor.setModel(&model);

        // THEN
        QCOMPARE(textEdit->toPlainText(), QString(model.property("title").toString()
                                                + "\n"
                                                + model.property("text").toString()));
        QCOMPARE(startDateEdit->date(), model.property("startDate").toDateTime().date());
        QCOMPARE(dueDateEdit->date(), model.property("dueDate").toDateTime().date());
        QCOMPARE(doneButton->isChecked(), model.property("done").toBool());
    }

    void shouldReactToTitleChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.setProperty("title", "My title");
        model.setProperty("text", "\nMy text");
        model.setProperty("startDate", QDateTime::currentDateTime());
        model.setProperty("dueDate", QDateTime::currentDateTime().addDays(2));
        model.setProperty("done", true);
        editor.setModel(&model);

        auto textEdit = editor.findChild<QPlainTextEdit*>("textEdit");

        // WHEN
        model.setPropertyAndSignal("title", "New title");

        // THEN
        QCOMPARE(textEdit->toPlainText(), QString(model.property("title").toString()
                                                + "\n"
                                                + model.property("text").toString()));
    }

    void shouldReactToTextChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.setProperty("title", "My title");
        model.setProperty("text", "\nMy text");
        model.setProperty("startDate", QDateTime::currentDateTime());
        model.setProperty("dueDate", QDateTime::currentDateTime().addDays(2));
        model.setProperty("done", true);
        editor.setModel(&model);

        auto textEdit = editor.findChild<QPlainTextEdit*>("textEdit");

        // WHEN
        model.setPropertyAndSignal("text", "\nNew text");

        // THEN
        QCOMPARE(textEdit->toPlainText(), QString(model.property("title").toString()
                                                + "\n"
                                                + model.property("text").toString()));
    }

    void shouldApplyTextEditChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        editor.setModel(&model);

        auto textEdit = editor.findChild<QPlainTextEdit*>("textEdit");

        // WHEN
        textEdit->setPlainText("Title\n\nText");

        // THEN
        QCOMPARE(model.property("title").toString(), QString("Title"));
        QCOMPARE(model.property("text").toString(), QString("\nText"));
    }

    void shouldReactToDoneChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.setProperty("title", "My title");
        model.setProperty("text", "\nMy text");
        model.setProperty("startDate", QDateTime::currentDateTime());
        model.setProperty("dueDate", QDateTime::currentDateTime().addDays(2));
        model.setProperty("done", false);
        editor.setModel(&model);

        auto doneButton = editor.findChild<QAbstractButton*>("doneButton");
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
        editor.setModel(&model);

        auto doneButton = editor.findChild<QAbstractButton*>("doneButton");

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
        model.setProperty("title", "My title");
        model.setProperty("text", "\nMy text");
        model.setProperty("startDate", QDateTime::currentDateTime());
        model.setProperty("dueDate", QDateTime::currentDateTime().addDays(2));
        model.setProperty("done", false);
        editor.setModel(&model);

        auto startDateEdit = editor.findChild<KPIM::KDateEdit*>("startDateEdit");

        // WHEN
        model.setPropertyAndSignal("startDate", QDateTime::currentDateTime().addDays(-2));

        // THEN
        QCOMPARE(startDateEdit->date(), model.property("startDate").toDateTime().date());
    }

    void shouldApplyStartDateEditChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        editor.setModel(&model);

        auto startDateEdit = editor.findChild<KPIM::KDateEdit*>("startDateEdit");
        auto today = QDate::currentDate();

        // WHEN
        startDateEdit->setEditText(today.toString(Qt::ISODate));
        QTest::keyClick(startDateEdit, Qt::Key_Enter);

        // THEN
        QCOMPARE(model.property("startDate").toDateTime().date(), today);
    }

    void shouldReactToDueDateChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.setProperty("title", "My title");
        model.setProperty("text", "\nMy text");
        model.setProperty("startDate", QDateTime::currentDateTime());
        model.setProperty("dueDate", QDateTime::currentDateTime().addDays(2));
        model.setProperty("done", false);
        editor.setModel(&model);

        auto dueDateEdit = editor.findChild<KPIM::KDateEdit*>("dueDateEdit");

        // WHEN
        model.setPropertyAndSignal("dueDate", QDateTime::currentDateTime().addDays(-2));

        // THEN
        QCOMPARE(dueDateEdit->date(), model.property("dueDate").toDateTime().date());
    }

    void shouldApplyDueDateEditChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        editor.setModel(&model);

        auto dueDateEdit = editor.findChild<KPIM::KDateEdit*>("dueDateEdit");
        auto today = QDate::currentDate();

        // WHEN
        dueDateEdit->setEditText(today.toString(Qt::ISODate));
        QTest::keyClick(dueDateEdit, Qt::Key_Enter);

        // THEN
        QCOMPARE(model.property("dueDate").toDateTime().date(), today);
    }
};

QTEST_MAIN(EditorViewTest)

#include "editorviewtest.moc"
