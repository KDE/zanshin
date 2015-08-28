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

typedef QSharedPointer<QObject> QObjectPtr;

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
        query.setFetchFunction([this] (const Domain::LiveQueryInput<QObject*>::AddFunction &add) {
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

    void shouldFilterOutNullRawPointers()
    {
        // GIVEN
        auto query = Domain::LiveQuery<QString, QObject*>();
        query.setFetchFunction([this] (const Domain::LiveQueryInput<QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add("0");
                add("1");
                add(QString());
                add("a");
                add("2");
            });
        });
        query.setConvertFunction([this] (const QString &s) -> QObject* {
            bool ok = false;
            const int id = s.toInt(&ok);
            if (ok) {
                auto object = new QObject(this);
                object->setProperty("id", id);
                return object;
            } else {
                return Q_NULLPTR;
            }
        });
        query.setPredicateFunction([] (const QString &s) {
            return !s.isEmpty();
        });

        // WHEN
        auto result = query.result();
        result->data();
        result = query.result(); // Should not cause any problem or wrong data
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);

        // THEN
        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->property("id").toInt(), 0);
        QCOMPARE(result->data().at(1)->property("id").toInt(), 1);
        QCOMPARE(result->data().at(2)->property("id").toInt(), 2);
    }

    void shouldFilterOutNullSharedPointers()
    {
        // GIVEN
        auto query = Domain::LiveQuery<QString, QObjectPtr>();
        query.setFetchFunction([this] (const Domain::LiveQueryInput<QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add("0");
                add("1");
                add(QString());
                add("a");
                add("2");
            });
        });
        query.setConvertFunction([this] (const QString &s) {
            bool ok = false;
            const int id = s.toInt(&ok);
            if (ok) {
                auto object = QObjectPtr::create();
                object->setProperty("id", id);
                return object;
            } else {
                return QObjectPtr();
            }
        });
        query.setPredicateFunction([] (const QString &s) {
            return !s.isEmpty();
        });

        // WHEN
        auto result = query.result();
        result->data();
        result = query.result(); // Should not cause any problem or wrong data
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);

        // THEN
        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->property("id").toInt(), 0);
        QCOMPARE(result->data().at(1)->property("id").toInt(), 1);
        QCOMPARE(result->data().at(2)->property("id").toInt(), 2);
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

    void shouldEmptyAndFetchAgainOnReset()
    {
        // GIVEN
        bool afterReset = false;

        Domain::LiveQuery<QObject*, QPair<int, QString>> query;
        query.setFetchFunction([this, &afterReset] (const Domain::LiveQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, &afterReset, add] {
                add(createObject(0, "0A"));
                add(createObject(1, "1A"));
                add(createObject(2, "2A"));
                add(createObject(3, "0B"));
                add(createObject(4, "1B"));
                add(createObject(5, "2B"));

                if (afterReset) {
                    add(createObject(6, "0C"));
                    add(createObject(7, "1C"));
                    add(createObject(8, "2C"));
                }
            });
        });
        query.setConvertFunction([] (QObject *object) {
            return QPair<int, QString>(object->property("objectId").toInt(), object->objectName());
        });
        query.setPredicateFunction([&afterReset] (QObject *object) {
            if (afterReset)
                return object->objectName().startsWith('1');
            else
                return object->objectName().startsWith('0');
        });

        Domain::QueryResult<QPair<int, QString>>::Ptr result = query.result();
        int removeHandlerCallCount = 0;
        result->addPostRemoveHandler([&removeHandlerCallCount](const QPair<int, QString> &, int) {
                                         removeHandlerCallCount++;
                                     });

        QTest::qWait(150);
        QVERIFY(!result->data().isEmpty());
        QCOMPARE(removeHandlerCallCount, 0);

        // WHEN
        query.reset();
        afterReset = true;
        QTest::qWait(150);

        // THEN
        QList<QPair<int, QString>> expected;
        expected << QPair<int, QString>(1, "1A")
                 << QPair<int, QString>(4, "1B")
                 << QPair<int, QString>(7, "1C");
        QVERIFY(afterReset);
        QCOMPARE(result->data(), expected);
        QCOMPARE(removeHandlerCallCount, 2);
    }
};

QTEST_MAIN(LiveQueryTest)

#include "livequerytest.moc"
