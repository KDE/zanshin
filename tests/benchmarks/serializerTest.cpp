/* This file is part of Zanshin

   Copyright 2014 Bensi Mario <mbensi@ipsquad.net>

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
#include <AkonadiCore/Item>
#include <KCalCore/Todo>
#include "domain/task.h"
#include "akonadi/akonadiserializer.h"

class SerializerBenchmark : public QObject
{
    Q_OBJECT

    Akonadi::Item createTestItem();
private slots:
    void deserialize();
    void checkPayloadAndDeserialize();
    void deserializeAndDestroy();
    void checkPayload();
};

Akonadi::Item SerializerBenchmark::createTestItem()
{
    KCalCore::Todo::Ptr todo(new KCalCore::Todo);
    todo->setSummary(QStringLiteral("summary"));
    todo->setDescription(QStringLiteral("content"));
    todo->setCompleted(false);
    todo->setDtStart(KDateTime(QDateTime(QDate(2013, 11, 24))));
    todo->setDtDue(KDateTime(QDateTime(QDate(2014, 03, 01))));
    todo->setRelatedTo(QStringLiteral("5"));

    // ... as payload of an item
    Akonadi::Item item;
    item.setMimeType(QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
    item.setPayload<KCalCore::Todo::Ptr>(todo);

    return item;
}

void SerializerBenchmark::deserialize()
{
    Akonadi::Item item = createTestItem();
    Akonadi::Serializer serializer;

    Domain::Task::Ptr task(new Domain::Task);
    QBENCHMARK {
        task = serializer.createTaskFromItem(item);
    }
}

void SerializerBenchmark::checkPayloadAndDeserialize()
{
    Akonadi::Item item = createTestItem();
    Akonadi::Serializer serializer;
    Domain::Task::Ptr task(new Domain::Task);
    QBENCHMARK {
        if (!item.hasPayload<KCalCore::Todo::Ptr>())
            return;

        auto todoCheck = item.payload<KCalCore::Todo::Ptr>();
        if (todoCheck->relatedTo() != QLatin1String("5")) {
            return;
        }

        task = serializer.createTaskFromItem(item);
    }
}

void SerializerBenchmark::deserializeAndDestroy()
{
    Akonadi::Item item = createTestItem();
    Akonadi::Serializer serializer;

    QBENCHMARK {
        auto task = serializer.createTaskFromItem(item);
    }
}

void SerializerBenchmark::checkPayload()
{
    Akonadi::Item item = createTestItem();
    QBENCHMARK {
        if (!item.hasPayload<KCalCore::Todo::Ptr>())
            return;

        auto todoCheck = item.payload<KCalCore::Todo::Ptr>();
        if (todoCheck->relatedTo() != QLatin1String("5")) {
            return;
        }
    }
}

ZANSHIN_TEST_MAIN(SerializerBenchmark)
#include "serializerTest.moc"
