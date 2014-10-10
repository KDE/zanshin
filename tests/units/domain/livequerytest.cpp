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

#include <QtTest>

#include "domain/livequery.h"

#include "utils/jobhandler.h"

#include "testlib/fakejob.h"

using namespace Domain;

class LiveQueryTest : public QObject
{
    Q_OBJECT
private:
    QObject *createObject(int id, const QString &name)
    {
        QObject *obj = new QObject(this);
        obj->setObjectName(name);
        obj->setProperty("objectId", id);
        return obj;
    }

private slots:
    void shouldHaveInitialFetchFunctionAndPredicate()
    {
        // GIVEN
        Domain::LiveQuery<QObject*, QPair<int, QString>> query;
        query.setFetchFunction([this] (const Domain::LiveQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add(createObject(0, "0A"));
                add(createObject(1, "1A"));
                add(createObject(2, "2A"));
                add(createObject(3, "0B"));
                add(createObject(4, "1B"));
                add(createObject(5, "2B"));
                add(createObject(6, "0C"));
                add(createObject(7, "1C"));
                add(createObject(8, "2C"));
            });
        });
        query.setConvertFunction([] (QObject *object) {
            return QPair<int, QString>(object->property("objectId").toInt(), object->objectName());
        });
        query.setPredicateFunction([] (QObject *object) {
            return object->objectName().startsWith('0');
        });

        // WHEN
        Domain::QueryResult<QPair<int, QString>>::Ptr result = query.result();
        result->data();
        result = query.result(); // Should not cause any problem or wrong data
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);

        // THEN
        QList<QPair<int, QString>> expected;
        expected << QPair<int, QString>(0, "0A")
                 << QPair<int, QString>(3, "0B")
                 << QPair<int, QString>(6, "0C");
        QCOMPARE(result->data(), expected);
    }

    void shouldDealWithSeveralFetchesProperly()
    {
        // GIVEN
        Domain::LiveQuery<QObject*, QPair<int, QString>> query;
        query.setFetchFunction([this] (const Domain::LiveQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add(createObject(0, "0A"));
                add(createObject(1, "1A"));
                add(createObject(2, "2A"));
                add(createObject(3, "0B"));
                add(createObject(4, "1B"));
                add(createObject(5, "2B"));
                add(createObject(6, "0C"));
                add(createObject(7, "1C"));
                add(createObject(8, "2C"));
            });
        });
        query.setConvertFunction([] (QObject *object) {
            return QPair<int, QString>(object->property("objectId").toInt(), object->objectName());
        });
        query.setPredicateFunction([] (QObject *object) {
            return object->objectName().startsWith('0');
        });

        for (int i = 0; i < 2; i++) {
            // WHEN * 2
            Domain::QueryResult<QPair<int, QString>>::Ptr result = query.result();

            // THEN * 2
            QVERIFY(result->data().isEmpty());
            QTest::qWait(150);
            QList<QPair<int, QString>> expected;
            expected << QPair<int, QString>(0, "0A")
                     << QPair<int, QString>(3, "0B")
                     << QPair<int, QString>(6, "0C");
            QCOMPARE(result->data(), expected);
        }
    }

    void shouldClearProviderWhenDeleted()
    {
        // GIVEN
        auto query = new Domain::LiveQuery<QObject*, QPair<int, QString>>;
        query->setFetchFunction([this] (const Domain::LiveQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add(createObject(0, "0A"));
                add(createObject(1, "1A"));
                add(createObject(2, "2A"));
            });
        });
        query->setConvertFunction([] (QObject *object) {
            return QPair<int, QString>(object->property("objectId").toInt(), object->objectName());
        });
        query->setPredicateFunction([] (QObject *object) {
            return object->objectName().startsWith('0');
        });

        Domain::QueryResult<QPair<int, QString>>::Ptr result = query->result();
        QTest::qWait(150);
        QCOMPARE(result->data().count(), 1);

        // WHEN
        delete query;

        // THEN
        QVERIFY(result->data().isEmpty());
    }

    void shouldReactToAdds()
    {
        // GIVEN
        Domain::LiveQuery<QObject*, QPair<int, QString>> query;
        query.setFetchFunction([this] (const Domain::LiveQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add(createObject(0, "0A"));
                add(createObject(1, "1A"));
                add(createObject(2, "2A"));
            });
        });
        query.setConvertFunction([] (QObject *object) {
            return QPair<int, QString>(object->property("objectId").toInt(), object->objectName());
        });
        query.setPredicateFunction([] (QObject *object) {
            return object->objectName().startsWith('0');
        });

        Domain::QueryResult<QPair<int, QString>>::Ptr result = query.result();
        QTest::qWait(150);
        QList<QPair<int, QString>> expected;
        expected << QPair<int, QString>(0, "0A");
        QCOMPARE(result->data(), expected);

        // WHEN
        query.onAdded(createObject(3, "0B"));
        query.onAdded(createObject(4, "1B"));
        query.onAdded(createObject(5, "2B"));

        // THEN
        expected << QPair<int, QString>(3, "0B");
        QCOMPARE(result->data(), expected);
    }

    void shouldReactToRemoves()
    {
        // GIVEN
        Domain::LiveQuery<QObject*, QPair<int, QString>> query;
        query.setFetchFunction([this] (const Domain::LiveQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add(createObject(0, "0A"));
                add(createObject(1, "1A"));
                add(createObject(2, "2A"));
                add(createObject(3, "0B"));
                add(createObject(4, "1B"));
                add(createObject(5, "2B"));
                add(createObject(6, "0C"));
                add(createObject(7, "1C"));
                add(createObject(8, "2C"));
            });
        });
        query.setConvertFunction([] (QObject *object) {
            return QPair<int, QString>(object->property("objectId").toInt(), object->objectName());
        });
        query.setPredicateFunction([] (QObject *object) {
            return object->objectName().startsWith('0');
        });
        query.setRepresentsFunction([] (QObject *object, const QPair<int, QString> &output) {
            return object->property("objectId").toInt() == output.first;
        });

        Domain::QueryResult<QPair<int, QString>>::Ptr result = query.result();
        QTest::qWait(150);
        QList<QPair<int, QString>> expected;
        expected << QPair<int, QString>(0, "0A")
                 << QPair<int, QString>(3, "0B")
                 << QPair<int, QString>(6, "0C");
        QCOMPARE(result->data(), expected);

        // WHEN
        query.onRemoved(createObject(3, "0B"));
        query.onRemoved(createObject(4, "1B"));
        query.onRemoved(createObject(5, "2B"));

        // THEN
        expected.removeAt(1);
        QCOMPARE(result->data(), expected);
    }

    void shouldReactToChanges()
    {
        // GIVEN
        Domain::LiveQuery<QObject*, QPair<int, QString>> query;
        query.setFetchFunction([this] (const Domain::LiveQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add(createObject(0, "0A"));
                add(createObject(1, "1A"));
                add(createObject(2, "2A"));
                add(createObject(3, "0B"));
                add(createObject(4, "1B"));
                add(createObject(5, "2B"));
                add(createObject(6, "0C"));
                add(createObject(7, "1C"));
                add(createObject(8, "2C"));
            });
        });
        query.setConvertFunction([] (QObject *object) {
            return QPair<int, QString>(object->property("objectId").toInt(), object->objectName());
        });
        query.setUpdateFunction([] (QObject *object, QPair<int, QString> &output) {
            output.second = object->objectName();
        });
        query.setPredicateFunction([] (QObject *object) {
            return object->objectName().startsWith('0');
        });
        query.setRepresentsFunction([] (QObject *object, const QPair<int, QString> &output) {
            return object->property("objectId").toInt() == output.first;
        });

        Domain::QueryResult<QPair<int, QString>>::Ptr result = query.result();
        QTest::qWait(150);
        QList<QPair<int, QString>> expected;
        expected << QPair<int, QString>(0, "0A")
                 << QPair<int, QString>(3, "0B")
                 << QPair<int, QString>(6, "0C");
        QCOMPARE(result->data(), expected);

        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const QPair<int, QString> &, int) {
                                          replaceHandlerCalled = true;
                                      });

        // WHEN
        query.onChanged(createObject(3, "0BB"));

        // Then
        expected.clear();
        expected << QPair<int, QString>(0, "0A")
                 << QPair<int, QString>(3, "0BB")
                 << QPair<int, QString>(6, "0C");
        QCOMPARE(result->data(), expected);
        QVERIFY(replaceHandlerCalled);
    }

    void shouldRemoveWhenChangesMakeInputUnsuitableForQuery()
    {
        // GIVEN
        Domain::LiveQuery<QObject*, QPair<int, QString>> query;
        query.setFetchFunction([this] (const Domain::LiveQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add(createObject(0, "0A"));
                add(createObject(1, "1A"));
                add(createObject(2, "2A"));
                add(createObject(3, "0B"));
                add(createObject(4, "1B"));
                add(createObject(5, "2B"));
                add(createObject(6, "0C"));
                add(createObject(7, "1C"));
                add(createObject(8, "2C"));
            });
        });
        query.setConvertFunction([] (QObject *object) {
            return QPair<int, QString>(object->property("objectId").toInt(), object->objectName());
        });
        query.setPredicateFunction([] (QObject *object) {
            return object->objectName().startsWith('0');
        });
        query.setRepresentsFunction([] (QObject *object, const QPair<int, QString> &output) {
            return object->property("objectId").toInt() == output.first;
        });

        Domain::QueryResult<QPair<int, QString>>::Ptr result = query.result();
        QTest::qWait(150);
        QList<QPair<int, QString>> expected;
        expected << QPair<int, QString>(0, "0A")
                 << QPair<int, QString>(3, "0B")
                 << QPair<int, QString>(6, "0C");
        QCOMPARE(result->data(), expected);

        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const QPair<int, QString> &, int) {
                                          replaceHandlerCalled = true;
                                      });

        // WHEN
        query.onChanged(createObject(3, "1B"));

        // Then
        expected.removeAt(1);
        QCOMPARE(result->data(), expected);
        QVERIFY(!replaceHandlerCalled);
    }

    void shouldAddWhenChangesMakeInputSuitableForQuery()
    {
        // GIVEN
        Domain::LiveQuery<QObject*, QPair<int, QString>> query;
        query.setFetchFunction([this] (const Domain::LiveQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add(createObject(0, "0A"));
                add(createObject(1, "1A"));
                add(createObject(2, "2A"));
                add(createObject(3, "0B"));
                add(createObject(4, "1B"));
                add(createObject(5, "2B"));
                add(createObject(6, "0C"));
                add(createObject(7, "1C"));
                add(createObject(8, "2C"));
            });
        });
        query.setConvertFunction([] (QObject *object) {
            return QPair<int, QString>(object->property("objectId").toInt(), object->objectName());
        });
        query.setPredicateFunction([] (QObject *object) {
            return object->objectName().startsWith('0');
        });
        query.setRepresentsFunction([] (QObject *object, const QPair<int, QString> &output) {
            return object->property("objectId").toInt() == output.first;
        });

        Domain::QueryResult<QPair<int, QString>>::Ptr result = query.result();
        QTest::qWait(150);
        QList<QPair<int, QString>> expected;
        expected << QPair<int, QString>(0, "0A")
                 << QPair<int, QString>(3, "0B")
                 << QPair<int, QString>(6, "0C");
        QCOMPARE(result->data(), expected);

        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const QPair<int, QString> &, int) {
                                          replaceHandlerCalled = true;
                                      });

        // WHEN
        query.onChanged(createObject(4, "0BB"));

        // Then
        expected << QPair<int, QString>(4, "0BB");
        QCOMPARE(result->data(), expected);
        QVERIFY(!replaceHandlerCalled);
    }
};

QTEST_MAIN(LiveQueryTest)

#include "livequerytest.moc"
