/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

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

private Q_SLOTS:
    void shouldHaveInitialFetchFunctionAndPredicate()
    {
        // GIVEN
        Domain::LiveQuery<QObject*, QPair<int, QString>> query;
        query.setFetchFunction([this] (const Domain::LiveQueryInput<QObject*>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add(createObject(0, QStringLiteral("0A")));
                add(createObject(1, QStringLiteral("1A")));
                add(createObject(2, QStringLiteral("2A")));
                add(createObject(3, QStringLiteral("0B")));
                add(createObject(4, QStringLiteral("1B")));
                add(createObject(5, QStringLiteral("2B")));
                add(createObject(6, QStringLiteral("0C")));
                add(createObject(7, QStringLiteral("1C")));
                add(createObject(8, QStringLiteral("2C")));
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
        expected << QPair<int, QString>(0, QStringLiteral("0A"))
                 << QPair<int, QString>(3, QStringLiteral("0B"))
                 << QPair<int, QString>(6, QStringLiteral("0C"));
        QCOMPARE(result->data(), expected);
    }

    void shouldFilterOutNullRawPointers()
    {
        // GIVEN
        auto query = Domain::LiveQuery<QString, QObject*>();
        query.setFetchFunction([] (const Domain::LiveQueryInput<QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [add] {
                add(QStringLiteral("0"));
                add(QStringLiteral("1"));
                add(QString());
                add(QStringLiteral("a"));
                add(QStringLiteral("2"));
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
                return nullptr;
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
        query.setFetchFunction([] (const Domain::LiveQueryInput<QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [add] {
                add(QStringLiteral("0"));
                add(QStringLiteral("1"));
                add(QString());
                add(QStringLiteral("a"));
                add(QStringLiteral("2"));
            });
        });
        query.setConvertFunction([] (const QString &s) {
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
                add(createObject(0, QStringLiteral("0A")));
                add(createObject(1, QStringLiteral("1A")));
                add(createObject(2, QStringLiteral("2A")));
                add(createObject(3, QStringLiteral("0B")));
                add(createObject(4, QStringLiteral("1B")));
                add(createObject(5, QStringLiteral("2B")));
                add(createObject(6, QStringLiteral("0C")));
                add(createObject(7, QStringLiteral("1C")));
                add(createObject(8, QStringLiteral("2C")));
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
            expected << QPair<int, QString>(0, QStringLiteral("0A"))
                     << QPair<int, QString>(3, QStringLiteral("0B"))
                     << QPair<int, QString>(6, QStringLiteral("0C"));
            QCOMPARE(result->data(), expected);
        }
    }

    void shouldClearProviderWhenDeleted()
    {
        // GIVEN
        auto query = new Domain::LiveQuery<QObject*, QPair<int, QString>>;
        query->setFetchFunction([this] (const Domain::LiveQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add(createObject(0, QStringLiteral("0A")));
                add(createObject(1, QStringLiteral("1A")));
                add(createObject(2, QStringLiteral("2A")));
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
                add(createObject(0, QStringLiteral("0A")));
                add(createObject(1, QStringLiteral("1A")));
                add(createObject(2, QStringLiteral("2A")));
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
        expected << QPair<int, QString>(0, QStringLiteral("0A"));
        QCOMPARE(result->data(), expected);

        // WHEN
        query.onAdded(createObject(3, QStringLiteral("0B")));
        query.onAdded(createObject(4, QStringLiteral("1B")));
        query.onAdded(createObject(5, QStringLiteral("2B")));

        // THEN
        expected << QPair<int, QString>(3, QStringLiteral("0B"));
        QCOMPARE(result->data(), expected);
    }

    void shouldReactToRemoves()
    {
        // GIVEN
        Domain::LiveQuery<QObject*, QPair<int, QString>> query;
        query.setFetchFunction([this] (const Domain::LiveQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add(createObject(0, QStringLiteral("0A")));
                add(createObject(1, QStringLiteral("1A")));
                add(createObject(2, QStringLiteral("2A")));
                add(createObject(3, QStringLiteral("0B")));
                add(createObject(4, QStringLiteral("1B")));
                add(createObject(5, QStringLiteral("2B")));
                add(createObject(6, QStringLiteral("0C")));
                add(createObject(7, QStringLiteral("1C")));
                add(createObject(8, QStringLiteral("2C")));
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
        expected << QPair<int, QString>(0, QStringLiteral("0A"))
                 << QPair<int, QString>(3, QStringLiteral("0B"))
                 << QPair<int, QString>(6, QStringLiteral("0C"));
        QCOMPARE(result->data(), expected);

        // WHEN
        query.onRemoved(createObject(3, QStringLiteral("0B")));
        query.onRemoved(createObject(4, QStringLiteral("1B")));
        query.onRemoved(createObject(5, QStringLiteral("2B")));

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
                add(createObject(0, QStringLiteral("0A")));
                add(createObject(1, QStringLiteral("1A")));
                add(createObject(2, QStringLiteral("2A")));
                add(createObject(3, QStringLiteral("0B")));
                add(createObject(4, QStringLiteral("1B")));
                add(createObject(5, QStringLiteral("2B")));
                add(createObject(6, QStringLiteral("0C")));
                add(createObject(7, QStringLiteral("1C")));
                add(createObject(8, QStringLiteral("2C")));
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
        expected << QPair<int, QString>(0, QStringLiteral("0A"))
                 << QPair<int, QString>(3, QStringLiteral("0B"))
                 << QPair<int, QString>(6, QStringLiteral("0C"));
        QCOMPARE(result->data(), expected);

        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const QPair<int, QString> &, int) {
                                          replaceHandlerCalled = true;
                                      });

        // WHEN
        query.onChanged(createObject(3, QStringLiteral("0BB")));

        // Then
        expected.clear();
        expected << QPair<int, QString>(0, QStringLiteral("0A"))
                 << QPair<int, QString>(3, QStringLiteral("0BB"))
                 << QPair<int, QString>(6, QStringLiteral("0C"));
        QCOMPARE(result->data(), expected);
        QVERIFY(replaceHandlerCalled);
    }

    void shouldRemoveWhenChangesMakeInputUnsuitableForQuery()
    {
        // GIVEN
        Domain::LiveQuery<QObject*, QPair<int, QString>> query;
        query.setFetchFunction([this] (const Domain::LiveQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add(createObject(0, QStringLiteral("0A")));
                add(createObject(1, QStringLiteral("1A")));
                add(createObject(2, QStringLiteral("2A")));
                add(createObject(3, QStringLiteral("0B")));
                add(createObject(4, QStringLiteral("1B")));
                add(createObject(5, QStringLiteral("2B")));
                add(createObject(6, QStringLiteral("0C")));
                add(createObject(7, QStringLiteral("1C")));
                add(createObject(8, QStringLiteral("2C")));
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
        expected << QPair<int, QString>(0, QStringLiteral("0A"))
                 << QPair<int, QString>(3, QStringLiteral("0B"))
                 << QPair<int, QString>(6, QStringLiteral("0C"));
        QCOMPARE(result->data(), expected);

        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const QPair<int, QString> &, int) {
                                          replaceHandlerCalled = true;
                                      });

        // WHEN
        query.onChanged(createObject(3, QStringLiteral("1B")));

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
                add(createObject(0, QStringLiteral("0A")));
                add(createObject(1, QStringLiteral("1A")));
                add(createObject(2, QStringLiteral("2A")));
                add(createObject(3, QStringLiteral("0B")));
                add(createObject(4, QStringLiteral("1B")));
                add(createObject(5, QStringLiteral("2B")));
                add(createObject(6, QStringLiteral("0C")));
                add(createObject(7, QStringLiteral("1C")));
                add(createObject(8, QStringLiteral("2C")));
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
        expected << QPair<int, QString>(0, QStringLiteral("0A"))
                 << QPair<int, QString>(3, QStringLiteral("0B"))
                 << QPair<int, QString>(6, QStringLiteral("0C"));
        QCOMPARE(result->data(), expected);

        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const QPair<int, QString> &, int) {
                                          replaceHandlerCalled = true;
                                      });

        // WHEN
        query.onChanged(createObject(4, QStringLiteral("0BB")));

        // Then
        expected << QPair<int, QString>(4, QStringLiteral("0BB"));
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
                add(createObject(0, QStringLiteral("0A")));
                add(createObject(1, QStringLiteral("1A")));
                add(createObject(2, QStringLiteral("2A")));
                add(createObject(3, QStringLiteral("0B")));
                add(createObject(4, QStringLiteral("1B")));
                add(createObject(5, QStringLiteral("2B")));

                if (afterReset) {
                    add(createObject(6, QStringLiteral("0C")));
                    add(createObject(7, QStringLiteral("1C")));
                    add(createObject(8, QStringLiteral("2C")));
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
        expected << QPair<int, QString>(1, QStringLiteral("1A"))
                 << QPair<int, QString>(4, QStringLiteral("1B"))
                 << QPair<int, QString>(7, QStringLiteral("1C"));
        QVERIFY(afterReset);
        QCOMPARE(result->data(), expected);
        QCOMPARE(removeHandlerCallCount, 2);
    }
};

ZANSHIN_TEST_MAIN(LiveQueryTest)

#include "livequerytest.moc"
