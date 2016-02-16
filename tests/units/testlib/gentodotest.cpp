/* This file is part of Zanshin

   Copyright 2015 Kevin Ottens <ervin@kde.org>

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

#include "testlib/gentodo.h"

#include <KCalCore/Todo>

#include <testlib/qtest_zanshin.h>

using namespace Testlib;

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
        QCOMPARE(newItem.mimeType(), KCalCore::Todo::todoMimeType());
        QVERIFY(newItem.hasPayload<KCalCore::Todo::Ptr>());
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

    void shouldAllowToSetTags()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withTags({42, 43, 44});

        // THEN
        QCOMPARE(item.tags().size(), 3);
        QCOMPARE(item.tags().at(0).id(), 42LL);
        QCOMPARE(item.tags().at(1).id(), 43LL);
        QCOMPARE(item.tags().at(2).id(), 44LL);
    }

    void shouldAllowToSetProjectType()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().asProject();

        // THEN
        QVERIFY(!item.payload<KCalCore::Todo::Ptr>()->customProperty("Zanshin", "Project").isEmpty());

        // WHEN
        item = GenTodo(item).asProject(false);

        // THEN
        QVERIFY(item.payload<KCalCore::Todo::Ptr>()->customProperty("Zanshin", "Project").isEmpty());
    }

    void shouldAllowToSetUid()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withUid(QStringLiteral("42"));

        // THEN
        QCOMPARE(item.payload<KCalCore::Todo::Ptr>()->uid(), QStringLiteral("42"));
    }

    void shouldAllowToSetParentUid()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withParentUid(QStringLiteral("42"));

        // THEN
        QCOMPARE(item.payload<KCalCore::Todo::Ptr>()->relatedTo(), QStringLiteral("42"));
    }

    void shouldAllowToSetTitle()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withTitle(QStringLiteral("42"));

        // THEN
        QCOMPARE(item.payload<KCalCore::Todo::Ptr>()->summary(), QStringLiteral("42"));
    }

    void shouldAllowToSetText()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withText(QStringLiteral("42"));

        // THEN
        QCOMPARE(item.payload<KCalCore::Todo::Ptr>()->description(), QStringLiteral("42"));
    }

    void shouldAllowToSetDoneState()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().done();

        // THEN
        QVERIFY(item.payload<KCalCore::Todo::Ptr>()->isCompleted());

        // WHEN
        item = GenTodo(item).done(false);

        // THEN
        QVERIFY(!item.payload<KCalCore::Todo::Ptr>()->isCompleted());
    }

    void shouldAllowToSetDoneDate()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withDoneDate(QDateTime(QDate(2015, 4, 12), QTime(12, 00)));

        // THEN
        QCOMPARE(item.payload<KCalCore::Todo::Ptr>()->completed().date(), QDate(2015, 04, 12));
    }

    void shouldAllowToSetDoneDateString()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withDoneDate(QStringLiteral("2015-04-12"));

        // THEN
        QCOMPARE(item.payload<KCalCore::Todo::Ptr>()->completed().date(), QDate(2015, 04, 12));
    }

    void shouldAllowToSetStartDate()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withStartDate(QDateTime(QDate(2015, 4, 12)));

        // THEN
        QCOMPARE(item.payload<KCalCore::Todo::Ptr>()->dtStart().date(), QDate(2015, 04, 12));
    }

    void shouldAllowToSetStartDateString()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withStartDate(QStringLiteral("2015-04-12"));

        // THEN
        QCOMPARE(item.payload<KCalCore::Todo::Ptr>()->dtStart().date(), QDate(2015, 04, 12));
    }

    void shouldAllowToSetDueDate()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withDueDate(QDateTime(QDate(2015, 4, 12)));

        // THEN
        QCOMPARE(item.payload<KCalCore::Todo::Ptr>()->dtDue().date(), QDate(2015, 04, 12));
    }

    void shouldAllowToSetDueDateString()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withDueDate(QStringLiteral("2015-04-12"));

        // THEN
        QCOMPARE(item.payload<KCalCore::Todo::Ptr>()->dtDue().date(), QDate(2015, 04, 12));
    }

    void shouldAllowToSetDelegate()
    {
        // GIVEN
        Akonadi::Item item = GenTodo().withDelegate(QStringLiteral("John Doe"), QStringLiteral("john@doe.net"));

        // THEN
        QCOMPARE(item.payload<KCalCore::Todo::Ptr>()->attendeeCount(), 1);

        const auto attendees = item.payload<KCalCore::Todo::Ptr>()->attendees();
        const auto delegate = std::find_if(attendees.begin(), attendees.end(),
                                           [] (const KCalCore::Attendee::Ptr &attendee) {
                                               return attendee->status() == KCalCore::Attendee::Delegated;
                                           });
        QVERIFY(delegate != attendees.constEnd());
        QCOMPARE((*delegate)->name(), QStringLiteral("John Doe"));
        QCOMPARE((*delegate)->email(), QStringLiteral("john@doe.net"));

        // WHEN
        item = GenTodo(item).withNoDelegate();

        // THEN
        QCOMPARE(item.payload<KCalCore::Todo::Ptr>()->attendeeCount(), 0);
    }
};

ZANSHIN_TEST_MAIN(GenTodoTest)

#include "gentodotest.moc"
