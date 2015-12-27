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
#include <QLabel>
#include <QPlainTextEdit>

#include "domain/note.h"
#include "domain/task.h"

#include "widgets/editorview.h"

#include "addressline/addresseelineedit.h"
#include "kdateedit.h"
#include <kglobal.h>
#include <klocale.h>

class EditorModelStub : public QObject
{
    Q_OBJECT
public:
    void setPropertyAndSignal(const QByteArray &name, const QVariant &value)
    {
        if (property(name) == value)
            return;

        setProperty(name, value);
        if (name == "artifact")
            emit artifactChanged(value.value<Domain::Artifact::Ptr>());
        else if (name == "text")
            emit textChanged(value.toString());
        else if (name == "title")
            emit titleChanged(value.toString());
        else if (name == "done")
            emit doneChanged(value.toBool());
        else if (name == "startDate")
            emit startDateChanged(value.toDateTime());
        else if (name == "dueDate")
            emit dueDateChanged(value.toDateTime());
        else if (name == "delegateText")
            emit delegateTextChanged(value.toString());
        else if (name == "hasTaskProperties")
            emit hasTaskPropertiesChanged(value.toBool());
        else
            qFatal("Unsupported property %s", name.constData());
    }

public slots:
    void setArtifact(const Domain::Artifact::Ptr &artifact) { setPropertyAndSignal("artifact", QVariant::fromValue(artifact)); }
    void setTitle(const QString &title) { setPropertyAndSignal("title", title); }
    void setText(const QString &text) { setPropertyAndSignal("text", text); }
    void setDone(bool done) { setPropertyAndSignal("done", done); }
    void setStartDate(const QDateTime &start) { setPropertyAndSignal("startDate", start); }
    void setDueDate(const QDateTime &due) { setPropertyAndSignal("dueDate", due); }
    void setDelegateText(const QString &text) { setPropertyAndSignal("delegateText", text); }
    void makeTaskAvailable() { setArtifact(Domain::Artifact::Ptr(new Domain::Task)); }

    void delegate(const QString &name, const QString &email)
    {
        delegateNames << name;
        delegateEmails << email;
    }

signals:
    void artifactChanged(const Domain::Artifact::Ptr &artifact);
    void hasTaskPropertiesChanged(bool hasTaskProperties);
    void textChanged(const QString &text);
    void titleChanged(const QString &title);
    void doneChanged(bool done);
    void startDateChanged(const QDateTime &date);
    void dueDateChanged(const QDateTime &due);
    void delegateTextChanged(const QString &delegateText);

public:
    QStringList delegateNames;
    QStringList delegateEmails;
};

class EditorViewTest : public QObject
{
    Q_OBJECT
public:
    explicit EditorViewTest(QObject *parent = Q_NULLPTR)
        : QObject(parent)
    {
        qputenv("ZANSHIN_UNIT_TEST_RUN", "1");
    }

private slots:
    void shouldHaveDefaultState()
    {
        Widgets::EditorView editor;

        QVERIFY(!editor.isEnabled());

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

        auto delegateLabel = editor.findChild<QLabel*>("delegateLabel");
        QVERIFY(delegateLabel);
        QVERIFY(!delegateLabel->isVisibleTo(&editor));

        auto delegateEdit = editor.findChild<KLineEdit*>("delegateEdit");
        QVERIFY(delegateEdit);
        QVERIFY(!delegateEdit->isVisibleTo(&editor));
    }

    void shouldNotCrashForNullModel()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.setTitle("Foo");
        model.setText("Bar");
        model.setPropertyAndSignal("hasTaskProperties", true);
        editor.setModel(&model);

        auto textEdit = editor.findChild<QPlainTextEdit*>("textEdit");
        QVERIFY(textEdit);

        auto startDateEdit = editor.findChild<KPIM::KDateEdit*>("startDateEdit");
        QVERIFY(startDateEdit);

        auto dueDateEdit = editor.findChild<KPIM::KDateEdit*>("dueDateEdit");
        QVERIFY(dueDateEdit);

        auto doneButton = editor.findChild<QAbstractButton*>("doneButton");
        QVERIFY(doneButton);

        auto delegateLabel = editor.findChild<QLabel*>("delegateLabel");
        QVERIFY(delegateLabel);

        auto delegateEdit = editor.findChild<KLineEdit*>("delegateEdit");
        QVERIFY(delegateEdit);

        // WHEN
        editor.setModel(Q_NULLPTR);

        // THEN
        QVERIFY(!editor.isEnabled());
        QVERIFY(textEdit->toPlainText().isEmpty());
        QVERIFY(!startDateEdit->isVisibleTo(&editor));
        QVERIFY(!dueDateEdit->isVisibleTo(&editor));
        QVERIFY(!doneButton->isVisibleTo(&editor));
        QVERIFY(!delegateLabel->isVisibleTo(&editor));
        QVERIFY(!delegateEdit->isVisibleTo(&editor));
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

        auto delegateLabel = editor.findChild<QLabel*>("delegateLabel");
        QVERIFY(!delegateLabel->isVisibleTo(&editor));

        auto delegateEdit = editor.findChild<KLineEdit*>("delegateEdit");
        QVERIFY(!delegateEdit->isVisibleTo(&editor));

        // WHEN
        editor.setModel(&model);

        // THEN
        QVERIFY(startDateEdit->isVisibleTo(&editor));
        QVERIFY(dueDateEdit->isVisibleTo(&editor));
        QVERIFY(doneButton->isVisibleTo(&editor));
        QVERIFY(!delegateLabel->isVisibleTo(&editor));
        QVERIFY(delegateEdit->isVisibleTo(&editor));
    }

    void shouldDisplayDelegateLabelOnlyWhenNeeded()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
        model.setDelegateText("John Doe");

        auto delegateLabel = editor.findChild<QLabel*>("delegateLabel");
        QVERIFY(!delegateLabel->isVisibleTo(&editor));

        // WHEN
        editor.setModel(&model);

        // THEN
        auto expectedText = tr("Delegated to: <b>%1</b>").arg(model.property("delegateText").toString());
        QVERIFY(delegateLabel->isVisibleTo(&editor));
        QCOMPARE(delegateLabel->text(), expectedText);
    }

    void shouldBeEnabledOnlyWhenAnArtifactIsAvailable()
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
        Domain::Artifact::Ptr artifact(new Domain::Task);
        model.setPropertyAndSignal("artifact", QVariant::fromValue(artifact));

        // THEN
        QVERIFY(editor.isEnabled());

        // WHEN
        model.setPropertyAndSignal("artifact", QVariant::fromValue(Domain::Artifact::Ptr()));

        // THEN
        QVERIFY(!editor.isEnabled());



        // GIVEN
        EditorModelStub model2;
        model2.setPropertyAndSignal("artifact", QVariant::fromValue(artifact));

        // WHEN
        editor.setModel(&model2);

        // THEN
        QVERIFY(editor.isEnabled());
    }

    void shouldReactToHasTaskPropertiesChanged()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
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
        model.makeTaskAvailable();
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
        model.makeTaskAvailable();
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

    void shouldNotReactToTitleTrim()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
        model.setProperty("title", "My title  ");
        model.setProperty("text", "\nMy text");
        model.setProperty("startDate", QDateTime::currentDateTime());
        model.setProperty("dueDate", QDateTime::currentDateTime().addDays(2));
        model.setProperty("done", true);
        editor.setModel(&model);

        auto textEdit = editor.findChild<QPlainTextEdit*>("textEdit");

        // WHEN
        model.setPropertyAndSignal("title", "My title");

        // THEN
        QCOMPARE(textEdit->toPlainText(), QString(model.property("title").toString() + "  "
                                                + "\n"
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

    void shouldNotReactToTextTrim()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
        model.setProperty("title", "My title");
        model.setProperty("text", "\nMy text \n ");
        model.setProperty("startDate", QDateTime::currentDateTime());
        model.setProperty("dueDate", QDateTime::currentDateTime().addDays(2));
        model.setProperty("done", true);
        editor.setModel(&model);

        auto textEdit = editor.findChild<QPlainTextEdit*>("textEdit");

        // WHEN
        model.setPropertyAndSignal("text", "\nMy text");

        // THEN
        QCOMPARE(textEdit->toPlainText(), QString(model.property("title").toString()
                                                + "\n"
                                                + model.property("text").toString() + " \n "));
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

        auto textEdit = editor.findChild<QPlainTextEdit*>("textEdit");

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
        model.makeTaskAvailable();
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
        model.makeTaskAvailable();
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
        model.makeTaskAvailable();
        editor.setModel(&model);

        auto startDateEdit = editor.findChild<KPIM::KDateEdit*>("startDateEdit");
        auto today = QDate::currentDate();

        // WHEN
        startDateEdit->setEditText(today.toString(Qt::ISODate)); // ### ISO? really? this only works because KLocale::readDate is clever, but it's not what the user would type
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
        model.makeTaskAvailable();
        editor.setModel(&model);

        auto dueDateEdit = editor.findChild<KPIM::KDateEdit*>("dueDateEdit");
        auto today = QDate::currentDate();

        // WHEN
        QVERIFY(dueDateEdit->isEnabled());
        dueDateEdit->setEditText(today.toString(Qt::ISODate));
        QTest::keyClick(dueDateEdit, Qt::Key_Enter);

        // THEN
        QCOMPARE(model.property("dueDate").toDateTime().date(), today);
    }

    void shouldApplyStartTodayChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
        editor.setModel(&model);

        QAbstractButton *startTodayButton = editor.findChild<QAbstractButton *>("startTodayButton");
        QVERIFY(startTodayButton);
        auto startDateEdit = editor.findChild<KPIM::KDateEdit*>("startDateEdit");
        auto today = QDate::currentDate();

        // WHEN
        QVERIFY(startTodayButton->isEnabled());
        startTodayButton->click();

        // THEN
        QCOMPARE(startDateEdit->currentText(), KGlobal::locale()->formatDate(today, KLocale::ShortDate));
        QCOMPARE(model.property("startDate").toDateTime().date(), today);
    }

    void shouldReactToDelegateTextChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
        model.setDelegateText("John Doe");
        editor.setModel(&model);

        auto delegateLabel = editor.findChild<QLabel*>("delegateLabel");

        // WHEN
        model.setDelegateText("John Smith");

        // THEN
        auto expectedText = tr("Delegated to: <b>%1</b>").arg(model.property("delegateText").toString());
        QCOMPARE(delegateLabel->text(), expectedText);
    }

    void shouldClearDelegateEditOnArtifactChanges()
    {
        // GIVEN
        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
        editor.setModel(&model);

        auto delegateEdit = editor.findChild<KLineEdit*>("delegateEdit");
        delegateEdit->setText("Foo");

        // WHEN
        model.makeTaskAvailable(); // simulates an artifact change

        // THEN
        QVERIFY(delegateEdit->text().isEmpty());
    }

    void shouldRequestDelegationOnInput_data()
    {
        QTest::addColumn<QString>("userInput");
        QTest::addColumn<QString>("expectedName");
        QTest::addColumn<QString>("expectedEmail");
        QTest::addColumn<bool>("expectedCall");

        QTest::newRow("nominal case") << "John Doe <john@doe.com>" << "John Doe" << "john@doe.com" << true;
        QTest::newRow("nominal case") << "John Doe <j.doe@some.server.com>" << "John Doe" << "j.doe@some.server.com" << true;
        QTest::newRow("only name") << "John Doe" << QString() << QString() << false;
        QTest::newRow("only email") << "john@doe.com" << QString() << "john@doe.com" << true;
        QTest::newRow("only email again") << "<john@doe.com>" << QString() << "john@doe.com" << true;
        QTest::newRow("nonsense case") << "bleh" << QString() << QString() << false;
    }

    void shouldRequestDelegationOnInput()
    {
        // GIVEN
        QFETCH(QString, userInput);
        QFETCH(QString, expectedName);
        QFETCH(QString, expectedEmail);
        QFETCH(bool, expectedCall);

        Widgets::EditorView editor;
        EditorModelStub model;
        model.makeTaskAvailable();
        editor.setModel(&model);

        auto delegateEdit = editor.findChild<KLineEdit*>("delegateEdit");

        // WHEN
        QVERIFY(delegateEdit->isEnabled());
        delegateEdit->setText(userInput);
        QTest::keyClick(delegateEdit, Qt::Key_Enter);

        // THEN
        if (expectedCall) {
            QCOMPARE(model.delegateNames.size(), 1);
            QCOMPARE(model.delegateNames.first(), expectedName);
            QCOMPARE(model.delegateEmails.size(), 1);
            QCOMPARE(model.delegateEmails.first(), expectedEmail);
            QVERIFY(delegateEdit->text().isEmpty());
        } else {
            QCOMPARE(model.delegateNames.size(), 0);
            QCOMPARE(model.delegateEmails.size(), 0);
        }
    }
};

QTEST_MAIN(EditorViewTest)

#include "editorviewtest.moc"
