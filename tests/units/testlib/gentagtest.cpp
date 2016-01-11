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

#include "testlib/gentag.h"

#include <testlib/qtest_zanshin.h>

#include "akonadi/akonadiserializerinterface.h"

using namespace Testlib;

class GenTagTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldImplicitlyConvertBackToTag()
    {
        // GIVEN
        auto tag = Akonadi::Tag(42);
        auto gen = GenTag(tag);

        // WHEN
        Akonadi::Tag newTag = gen;

        // THEN
        QCOMPARE(newTag, tag);
    }

    void shouldAllowToSetId()
    {
        // GIVEN
        Akonadi::Tag tag = GenTag().withId(42);

        // THEN
        QCOMPARE(tag.id(), 42LL);
    }

    void shouldAllowToSetName()
    {
        // GIVEN
        Akonadi::Tag tag = GenTag().withName("42");

        // THEN
        QCOMPARE(tag.name(), QString("42"));
        QCOMPARE(tag.gid(), tag.name().toLatin1());
    }

    void shouldAllowToSetContextType()
    {
        // GIVEN
        Akonadi::Tag tag = GenTag().asContext();

        // THEN
        QCOMPARE(tag.type(), Akonadi::SerializerInterface::contextTagType());
    }

    void shouldAllowToSetPlainType()
    {
        // GIVEN
        Akonadi::Tag tag = GenTag().asPlain();

        // THEN
        QCOMPARE(tag.type(), QByteArray(Akonadi::Tag::PLAIN));
    }
};

ZANSHIN_TEST_MAIN(GenTagTest)

#include "gentagtest.moc"
