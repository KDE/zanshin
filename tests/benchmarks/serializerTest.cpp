/*
 * SPDX-FileCopyrightText: 2014 Bensi Mario <mbensi@ipsquad.net>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>
#include <Akonadi/Item>
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
    todo->setDtStart(QDateTime(QDate(2013, 11, 24)));
    todo->setDtDue(QDateTime(QDate(2014, 03, 01)));
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
