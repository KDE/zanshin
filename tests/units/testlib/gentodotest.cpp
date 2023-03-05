/*
 * SPDX-FileCopyrightText: 2015 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include "testlib/gentodo.h"

#include <KCalendarCore/Todo>

#include <testlib/qtest_zanshin.h>
#include <akonadi/akonadiserializer.h>

using namespace Testlib;
using Akonadi::Serializer;

class GenTodoTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldImplicitlyConvertBackToItem()
    {
        // GIVEN
        auto item = Akonadi::Item(42);
        auto gen = GenTodo(item);

        // WHEN
        Akonadi::Item newItem = gen;

        // THEN
        QCOMPARE(newItem, item);
        QCOMPARE(newItem.mimeType(), KCalendarCore::Todo::todoMimeType());
        QVERIFY(newItem.hasPayload<KCalendarCore::Todo::Ptr>());
    }

    void shouldAllowToSetId()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withId(42);

        // THEN
        QCOMPARE(item.id(), 42LL);
    }

    void shouldAllowToSetParent()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withParent(42);

        // THEN
        QCOMPARE(item.parentCollection().id(), 42LL);
    }

    void shouldAllowToSetContexts()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withContexts({"42", "43", "44"});

        // THEN
        auto todo = item.payload<KCalendarCore::Todo::Ptr>();
        QStringList contextUids = todo->customProperty(Serializer::customPropertyAppName(), Serializer::customPropertyContextList()).split(',');
        QCOMPARE(contextUids.size(), 3);
        QCOMPARE(contextUids.at(0), "42");
        QCOMPARE(contextUids.at(1), "43");
        QCOMPARE(contextUids.at(2), "44");
    }

    void shouldAllowToSetProjectType()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().asProject();

        // THEN
        QVERIFY(!item.payload<KCalendarCore::Todo::Ptr>()->customProperty(Serializer::customPropertyAppName(), Serializer::customPropertyIsProject()).isEmpty());

        // WHEN
        item = GenTodo(item).asProject(false);

        // THEN
        QVERIFY(item.payload<KCalendarCore::Todo::Ptr>()->customProperty(Serializer::customPropertyAppName(), Serializer::customPropertyIsProject()).isEmpty());
    }

    void shouldAllowToSetContextType()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().asContext();

        // THEN
        QVERIFY(!item.payload<KCalendarCore::Todo::Ptr>()->customProperty(Serializer::customPropertyAppName(), Serializer::customPropertyIsContext()).isEmpty());

        // WHEN
        item = GenTodo(item).asContext(false);

        // THEN
        QVERIFY(item.payload<KCalendarCore::Todo::Ptr>()->customProperty(Serializer::customPropertyAppName(), Serializer::customPropertyIsContext()).isEmpty());
    }

    void shouldAllowToSetUid()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withUid(QStringLiteral("42"));

        // THEN
        QCOMPARE(item.payload<KCalendarCore::Todo::Ptr>()->uid(), QStringLiteral("42"));
    }

    void shouldAllowToSetParentUid()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withParentUid(QStringLiteral("42"));

        // THEN
        QCOMPARE(item.payload<KCalendarCore::Todo::Ptr>()->relatedTo(), QStringLiteral("42"));
    }

    void shouldAllowToSetTitle()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withTitle(QStringLiteral("42"));

        // THEN
        QCOMPARE(item.payload<KCalendarCore::Todo::Ptr>()->summary(), QStringLiteral("42"));
    }

    void shouldAllowToSetText()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withText(QStringLiteral("42"));

        // THEN
        QCOMPARE(item.payload<KCalendarCore::Todo::Ptr>()->description(), QStringLiteral("42"));
    }

    void shouldAllowToSetDoneState()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().done();

        // THEN
        QVERIFY(item.payload<KCalendarCore::Todo::Ptr>()->isCompleted());

        // WHEN
        item = GenTodo(item).done(false);

        // THEN
        QVERIFY(!item.payload<KCalendarCore::Todo::Ptr>()->isCompleted());
    }

    void shouldAllowToSetDoneDate()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withDoneDate(QDate(2015, 4, 12));

        // THEN
        QCOMPARE(item.payload<KCalendarCore::Todo::Ptr>()->completed().toLocalTime().date(), QDate(2015, 04, 12));
    }

    void shouldAllowToSetDoneDateString()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withDoneDate(QStringLiteral("2015-04-12"));

        // THEN
        QCOMPARE(item.payload<KCalendarCore::Todo::Ptr>()->completed().toLocalTime().date(), QDate(2015, 04, 12));
    }

    void shouldAllowToSetStartDate()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withStartDate(QDate(2015, 4, 12));

        // THEN
        QCOMPARE(item.payload<KCalendarCore::Todo::Ptr>()->dtStart().date(), QDate(2015, 04, 12));
    }

    void shouldAllowToSetStartDateString()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withStartDate(QStringLiteral("2015-04-12"));

        // THEN
        QCOMPARE(item.payload<KCalendarCore::Todo::Ptr>()->dtStart().date(), QDate(2015, 04, 12));
    }

    void shouldAllowToSetDueDate()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withDueDate(QDate(2015, 4, 12));

        // THEN
        QCOMPARE(item.payload<KCalendarCore::Todo::Ptr>()->dtDue().date(), QDate(2015, 04, 12));
    }

    void shouldAllowToSetDueDateString()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withDueDate(QStringLiteral("2015-04-12"));

        // THEN
        QCOMPARE(item.payload<KCalendarCore::Todo::Ptr>()->dtDue().date(), QDate(2015, 04, 12));
    }
};

ZANSHIN_TEST_MAIN(GenTodoTest)

#include "gentodotest.moc"
