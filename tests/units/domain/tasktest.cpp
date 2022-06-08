/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include "domain/task.h"

#include "utils/datetime.h"

using namespace Domain;

class TaskTest : public QObject
{
    Q_OBJECT
public:
    explicit TaskTest(QObject *parent = nullptr)
        : QObject(parent)
    {
        qRegisterMetaType<Task::Recurrence>();
        qRegisterMetaType<Task::Attachments>();
    }

private Q_SLOTS:
    void shouldHaveEmptyPropertiesByDefault()
    {
        Task t;
        QCOMPARE(t.text(), QString());
        QCOMPARE(t.title(), QString());
        QCOMPARE(t.isDone(), false);
        QCOMPARE(t.startDate(), QDate());
        QCOMPARE(t.dueDate(), QDate());
        QCOMPARE(t.doneDate(), QDate());
        QCOMPARE(t.recurrence(), Domain::Task::NoRecurrence);
        QVERIFY(t.attachments().isEmpty());
    }

    void shouldNotifyTextChanges()
    {
        Task t;
        QSignalSpy spy(&t, &Task::textChanged);
        t.setText(QStringLiteral("foo"));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toString(), QStringLiteral("foo"));
    }

    void shouldNotNotifyIdenticalTextChanges()
    {
        Task t;
        t.setText(QStringLiteral("foo"));
        QSignalSpy spy(&t, &Task::textChanged);
        t.setText(QStringLiteral("foo"));
        QCOMPARE(spy.count(), 0);
    }

    void shouldNotifyTitleChanges()
    {
        Task t;
        QSignalSpy spy(&t, &Task::titleChanged);
        t.setTitle(QStringLiteral("foo"));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toString(), QStringLiteral("foo"));
    }

    void shouldNotNotifyIdenticalTitleChanges()
    {
        Task t;
        t.setTitle(QStringLiteral("foo"));
        QSignalSpy spy(&t, &Task::titleChanged);
        t.setTitle(QStringLiteral("foo"));
        QCOMPARE(spy.count(), 0);
    }

    void shouldHaveValueBasedAttachment()
    {
        Task::Attachment a;
        QVERIFY(!a.isValid());
        QVERIFY(!a.isUri());
        QCOMPARE(a.uri(), QUrl());
        QCOMPARE(a.data(), QByteArray());
        QCOMPARE(a.label(), QString());
        QCOMPARE(a.mimeType(), QString());
        QCOMPARE(a.iconName(), QString());

        a.setUri(QUrl("https://www.kde.org"));
        QVERIFY(a.isValid());
        QVERIFY(a.isUri());
        QCOMPARE(a.uri(), QUrl("https://www.kde.org"));
        QCOMPARE(a.data(), QByteArray());
        QCOMPARE(a.label(), QString());
        QCOMPARE(a.mimeType(), QString());
        QCOMPARE(a.iconName(), QString());

        a.setData(QByteArrayLiteral("foobar"));
        QVERIFY(a.isValid());
        QVERIFY(!a.isUri());
        QCOMPARE(a.uri(), QUrl());
        QCOMPARE(a.data(), QByteArrayLiteral("foobar"));
        QCOMPARE(a.label(), QString());
        QCOMPARE(a.mimeType(), QString());
        QCOMPARE(a.iconName(), QString());

        a.setLabel(QStringLiteral("baz"));
        QVERIFY(a.isValid());
        QVERIFY(!a.isUri());
        QCOMPARE(a.uri(), QUrl());
        QCOMPARE(a.data(), QByteArrayLiteral("foobar"));
        QCOMPARE(a.label(), QStringLiteral("baz"));
        QCOMPARE(a.mimeType(), QString());
        QCOMPARE(a.iconName(), QString());

        a.setMimeType(QStringLiteral("text/plain"));
        QVERIFY(a.isValid());
        QVERIFY(!a.isUri());
        QCOMPARE(a.uri(), QUrl());
        QCOMPARE(a.data(), QByteArrayLiteral("foobar"));
        QCOMPARE(a.label(), QStringLiteral("baz"));
        QCOMPARE(a.mimeType(), QStringLiteral("text/plain"));
        QCOMPARE(a.iconName(), QString());

        a.setIconName(QStringLiteral("text"));
        QVERIFY(a.isValid());
        QVERIFY(!a.isUri());
        QCOMPARE(a.uri(), QUrl());
        QCOMPARE(a.data(), QByteArrayLiteral("foobar"));
        QCOMPARE(a.label(), QStringLiteral("baz"));
        QCOMPARE(a.mimeType(), QStringLiteral("text/plain"));
        QCOMPARE(a.iconName(), QStringLiteral("text"));

        a.setUri(QUrl("https://www.kde.org"));
        QVERIFY(a.isValid());
        QVERIFY(a.isUri());
        QCOMPARE(a.uri(), QUrl("https://www.kde.org"));
        QCOMPARE(a.data(), QByteArray());
        QCOMPARE(a.label(), QStringLiteral("baz"));
        QCOMPARE(a.mimeType(), QStringLiteral("text/plain"));
        QCOMPARE(a.iconName(), QStringLiteral("text"));

        a.setUri(QUrl());
        QVERIFY(!a.isValid());
        QVERIFY(!a.isUri());
        QCOMPARE(a.uri(), QUrl());
        QCOMPARE(a.data(), QByteArray());
        QCOMPARE(a.label(), QStringLiteral("baz"));
        QCOMPARE(a.mimeType(), QStringLiteral("text/plain"));
        QCOMPARE(a.iconName(), QStringLiteral("text"));
    }

    void shouldNotifyStatusChanges()
    {
        Task t;
        QSignalSpy spy(&t, &Task::doneChanged);
        t.setDone(true);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toBool(), true);
    }

    void shouldNotNotifyIdenticalStatusChanges()
    {
        Task t;
        t.setDone(true);
        QSignalSpy spy(&t, &Task::doneChanged);
        t.setDone(true);
        QCOMPARE(spy.count(), 0);
    }

    void shouldNotifyStartDateChanges()
    {
        Task t;
        QSignalSpy spy(&t, &Task::startDateChanged);
        t.setStartDate(QDate(2014, 1, 13));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toDate(), QDate(2014, 1, 13));
    }

    void shouldNotNotifyIdenticalStartDateChanges()
    {
        Task t;
        t.setStartDate(QDate(2014, 1, 13));
        QSignalSpy spy(&t, &Task::startDateChanged);
        t.setStartDate(QDate(2014, 1, 13));
        QCOMPARE(spy.count(), 0);
    }

    void shouldNotifyDueDateChanges()
    {
        Task t;
        QSignalSpy spy(&t, &Task::dueDateChanged);
        t.setDueDate(QDate(2014, 1, 13));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toDate(), QDate(2014, 1, 13));
    }

    void shouldNotNotifyIdenticalDueDateChanges()
    {
        Task t;
        t.setDueDate(QDate(2014, 1, 13));
        QSignalSpy spy(&t, &Task::dueDateChanged);
        t.setDueDate(QDate(2014, 1, 13));
        QCOMPARE(spy.count(), 0);
    }

    void shouldNotifyRecurrenceChanges()
    {
        Task t;
        QSignalSpy spy(&t, &Task::recurrenceChanged);
        t.setRecurrence(Task::RecursWeekly);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().value<Task::Recurrence>(), Task::RecursWeekly);
    }

    void shouldNotNotifyIdenticalRecurrenceChanges()
    {
        Task t;
        t.setRecurrence(Task::RecursWeekly);
        QSignalSpy spy(&t, &Task::recurrenceChanged);
        t.setRecurrence(Task::RecursWeekly);
        QCOMPARE(spy.count(), 0);
    }

    void shouldNotifyAttachmentsChanges()
    {
        Task::Attachments attachments;
        attachments.append(Task::Attachment(QByteArrayLiteral("foobar")));
        attachments.append(Task::Attachment(QUrl("https://www.kde.org")));

        Task t;
        QSignalSpy spy(&t, &Task::attachmentsChanged);
        t.setAttachments(attachments);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().value<Task::Attachments>(), attachments);
    }

    void shouldNotNotifyIdenticalAttachmentsChanges()
    {
        Task::Attachments attachments;
        attachments.append(Task::Attachment(QByteArrayLiteral("foobar")));
        attachments.append(Task::Attachment(QUrl("https://www.kde.org")));

        Task t;
        t.setAttachments(attachments);
        QSignalSpy spy(&t, &Task::attachmentsChanged);
        t.setAttachments(attachments);
        QCOMPARE(spy.count(), 0);
    }

    void shouldNotifyDoneDateChanges()
    {
        Task t;
        QSignalSpy spy(&t, &Task::doneDateChanged);
        t.setDoneDate(QDate(2014, 1, 13));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toDate(), QDate(2014, 1, 13));
    }

    void shouldNotNotifyIdenticalDoneDateChanges()
    {
        Task t;
        t.setDoneDate(QDate(2014, 1, 13));
        QSignalSpy spy(&t, &Task::doneDateChanged);
        t.setDoneDate(QDate(2014, 1, 13));
        QCOMPARE(spy.count(), 0);
    }

    void shouldNotifyDoneDateSet()
    {
        Task t;
        QSignalSpy spy(&t, &Task::doneDateChanged);
        t.setDone(true);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).toDate(), Utils::DateTime::currentDate());
    }

    void shouldNotifyDoneDateUnset()
    {
        Task t;

        t.setDone(true);
        QSignalSpy spy(&t, &Task::doneDateChanged);
        t.setDone(false);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).toDate(), QDate());
    }
};

ZANSHIN_TEST_MAIN(TaskTest)

#include "tasktest.moc"
