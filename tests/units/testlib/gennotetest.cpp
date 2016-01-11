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

#include "testlib/gennote.h"

#include <Akonadi/Notes/NoteUtils>
#include <KMime/Message>

#include <testlib/qtest_zanshin.h>

using namespace Testlib;

class GenNoteTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldImplicitlyConvertBackToItem()
    {
        // GIVEN
        auto item = Akonadi::Item(42);
        auto gen = GenNote(item);

        // WHEN
        Akonadi::Item newItem = gen;

        // THEN
        QCOMPARE(newItem, item);
        QCOMPARE(newItem.mimeType(), Akonadi::NoteUtils::noteMimeType());
        QVERIFY(newItem.hasPayload<KMime::Message::Ptr>());
    }

    void shouldAllowToSetId()
    {
        // GIVEN
        Akonadi::Item item = GenNote().withId(42);

        // THEN
        QCOMPARE(item.id(), 42LL);
    }

    void shouldAllowToSetParent()
    {
        // GIVEN
        Akonadi::Item item = GenNote().withParent(42);

        // THEN
        QCOMPARE(item.parentCollection().id(), 42LL);
    }

    void shouldAllowToSetTags()
    {
        // GIVEN
        Akonadi::Item item = GenNote().withTags({42, 43, 44});

        // THEN
        QCOMPARE(item.tags().size(), 3);
        QCOMPARE(item.tags().at(0).id(), 42LL);
        QCOMPARE(item.tags().at(1).id(), 43LL);
        QCOMPARE(item.tags().at(2).id(), 44LL);
    }

    void shouldAllowToSetParentUid()
    {
        // GIVEN
        Akonadi::Item item = GenNote().withParentUid("42");

        // THEN
        QCOMPARE(item.payload<KMime::Message::Ptr>()->headerByType("X-Zanshin-RelatedProjectUid")->asUnicodeString(), QString("42"));
    }

    void shouldAllowToSetTitle()
    {
        // GIVEN
        Akonadi::Item item = GenNote().withTitle("42");

        // THEN
        QCOMPARE(item.payload<KMime::Message::Ptr>()->subject()->asUnicodeString(), QString("42"));
    }

    void shouldAllowToSetText()
    {
        // GIVEN
        Akonadi::Item item = GenNote().withText("42");

        // THEN
        QCOMPARE(item.payload<KMime::Message::Ptr>()->body(), QByteArray("42"));
    }
};

ZANSHIN_TEST_MAIN(GenNoteTest)

#include "gennotetest.moc"
