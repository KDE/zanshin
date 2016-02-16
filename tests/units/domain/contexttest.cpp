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

#include "domain/context.h"

using namespace Domain;

class ContextTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldHaveEmptyPropertiesByDefault()
    {
        Context c;
        QCOMPARE(c.name(), QString());
    }

    void shouldNotifyNameChanges()
    {
        Context c;
        QSignalSpy spy(&c, &Context::nameChanged);
        c.setName("foo");
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toString(), QString("foo"));
    }

    void shouldNotNotifyIdenticalNameChanges()
    {
        Context c;
        c.setName("foo");
        QSignalSpy spy(&c, &Context::nameChanged);
        c.setName("foo");
        QCOMPARE(spy.count(), 0);
    }
};

ZANSHIN_TEST_MAIN(ContextTest)

#include "contexttest.moc"
