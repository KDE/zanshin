/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
   Copyright 2018 David Faure <faure@kde.org>

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

#include "domain/livequery.h"

#include "utils/jobhandler.h"

#include "testlib/fakejob.h"

using namespace Domain;

typedef QSharedPointer<QObject> QObjectPtr;
static const char objectIdPropName[] = "objectId";

class LiveRelationshipQueryTest : public QObject
{
    Q_OBJECT
private:

    QObject *createObject(int id, const QString &name)
    {
        QObject *obj = new QObject(this);
        obj->setObjectName(name);
        obj->setProperty(objectIdPropName, id);
        return obj;
    }

    static bool compareObjectIds(QObject *obj1, QObject *obj2)
    {
        return obj1->property(objectIdPropName).toInt() == obj2->property(objectIdPropName).toInt();
    }

    static bool isProject(QObject *obj)
    {
        return obj->objectName().startsWith(QLatin1String("Project"));
    }

    static QPair<int, QString> convertToPair(QObject *object)
    {
        return qMakePair(object->property(objectIdPropName).toInt(), object->objectName());
    }

    static bool representsPair(QObject *object, const QPair<int, QString> &output) {
        return object->property(objectIdPropName).toInt() == output.first;
    };

private slots:
    void shouldHaveInitialFetchFunctionAndPredicate()
    {
        // GIVEN
        Domain::LiveRelationshipQuery<QObject*, QPair<int, QString>> query;
        query.setFetchFunction([this] (const Domain::LiveQueryInput<QObject*>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add(createObject(0, QStringLiteral("ProjectA")));
                add(createObject(1, QStringLiteral("ItemA")));
                add(createObject(2, QStringLiteral("ParentA")));
                add(createObject(3, QStringLiteral("ProjectB")));
                add(createObject(4, QStringLiteral("ItemB")));
                add(createObject(5, QStringLiteral("ParentB")));
                add(createObject(6, QStringLiteral("ProjectC")));
                add(createObject(7, QStringLiteral("ItemC")));
                add(createObject(8, QStringLiteral("ParentC")));
            });
        });
        query.setConvertFunction(convertToPair);
        query.setPredicateFunction(isProject);
        query.setCompareFunction(compareObjectIds);

        // WHEN
        Domain::QueryResult<QPair<int, QString>>::Ptr result = query.result();
        result->data();
        result = query.result(); // Should not cause any problem or wrong data
        QVERIFY(result->data().isEmpty());

        // THEN
        QList<QPair<int, QString>> expected;
        expected << QPair<int, QString>(0, QStringLiteral("ProjectA"))
                 << QPair<int, QString>(3, QStringLiteral("ProjectB"))
                 << QPair<int, QString>(6, QStringLiteral("ProjectC"));
        QTRY_COMPARE(result->data(), expected);
    }

    void shouldFilterOutNullRawPointers()
    {
        // GIVEN
        auto query = Domain::LiveRelationshipQuery<QString, QObject*>();
        query.setFetchFunction([this] (const Domain::LiveQueryInput<QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
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

        // THEN
        QTRY_COMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->property("id").toInt(), 0);
        QCOMPARE(result->data().at(1)->property("id").toInt(), 1);
        QCOMPARE(result->data().at(2)->property("id").toInt(), 2);
    }

    void shouldFilterOutNullSharedPointers()
    {
        // GIVEN
        auto query = Domain::LiveRelationshipQuery<QString, QObjectPtr>();
        query.setFetchFunction([this] (const Domain::LiveQueryInput<QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add(QStringLiteral("0"));
                add(QStringLiteral("1"));
                add(QString());
                add(QStringLiteral("a"));
                add(QStringLiteral("2"));
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

        // THEN
        QTRY_COMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->property("id").toInt(), 0);
        QCOMPARE(result->data().at(1)->property("id").toInt(), 1);
        QCOMPARE(result->data().at(2)->property("id").toInt(), 2);
    }

    void shouldDealWithSeveralFetchesProperly()
    {
        // GIVEN
        Domain::LiveRelationshipQuery<QObject*, QPair<int, QString>> query;
        query.setFetchFunction([this] (const Domain::LiveRelationshipQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add(createObject(0, QStringLiteral("ProjectA")));
                add(createObject(1, QStringLiteral("ItemA")));
                add(createObject(2, QStringLiteral("ParentA")));
                add(createObject(3, QStringLiteral("ProjectB")));
                add(createObject(4, QStringLiteral("ItemB")));
                add(createObject(5, QStringLiteral("ParentB")));
                add(createObject(6, QStringLiteral("ProjectC")));
                add(createObject(7, QStringLiteral("ItemC")));
                add(createObject(8, QStringLiteral("ParentC")));
            });
        });
        query.setConvertFunction(convertToPair);
        query.setPredicateFunction(isProject);

        for (int i = 0; i < 2; i++) {
            // WHEN * 2
            Domain::QueryResult<QPair<int, QString>>::Ptr result = query.result();

            // THEN * 2
            QVERIFY(result->data().isEmpty());
            QList<QPair<int, QString>> expected;
            expected << QPair<int, QString>(0, QStringLiteral("ProjectA"))
                     << QPair<int, QString>(3, QStringLiteral("ProjectB"))
                     << QPair<int, QString>(6, QStringLiteral("ProjectC"));
            QTRY_COMPARE(result->data(), expected);
        }
    }

    void shouldClearProviderWhenDeleted()
    {
        // GIVEN
        auto query = new Domain::LiveRelationshipQuery<QObject*, QPair<int, QString>>;
        query->setFetchFunction([this] (const Domain::LiveRelationshipQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add(createObject(0, QStringLiteral("ProjectA")));
                add(createObject(1, QStringLiteral("ItemA")));
                add(createObject(2, QStringLiteral("ParentA")));
            });
        });
        query->setConvertFunction(convertToPair);
        query->setPredicateFunction(isProject);
        query->setCompareFunction(compareObjectIds);

        Domain::QueryResult<QPair<int, QString>>::Ptr result = query->result();
        QTRY_COMPARE(result->data().count(), 1);

        // WHEN
        delete query;

        // THEN
        QVERIFY(result->data().isEmpty());
    }

    void shouldReactToAdds()
    {
        // GIVEN
        Domain::LiveRelationshipQuery<QObject*, QPair<int, QString>> query;
        query.setFetchFunction([this] (const Domain::LiveRelationshipQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add(createObject(0, QStringLiteral("ProjectA")));
                add(createObject(1, QStringLiteral("ItemA")));
                add(createObject(2, QStringLiteral("ParentA")));
            });
        });
        query.setConvertFunction(convertToPair);
        query.setPredicateFunction(isProject);
        query.setCompareFunction(compareObjectIds);

        Domain::QueryResult<QPair<int, QString>>::Ptr result = query.result();
        QList<QPair<int, QString>> expected{ qMakePair(0, QString::fromLatin1("ProjectA")) };
        QTRY_COMPARE(result->data(), expected);

        // WHEN
        query.onAdded(createObject(3, QStringLiteral("ProjectB")));
        query.onAdded(createObject(4, QStringLiteral("ItemB")));
        query.onAdded(createObject(5, QStringLiteral("ParentB")));

        // THEN
        expected << QPair<int, QString>(3, QStringLiteral("ProjectB"));
        QCOMPARE(result->data(), expected);
    }

    void shouldReactToRemoves()
    {
        // GIVEN
        Domain::LiveRelationshipQuery<QObject*, QPair<int, QString>> query;
        query.setFetchFunction([this] (const Domain::LiveRelationshipQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add(createObject(0, QStringLiteral("ProjectA")));
                add(createObject(1, QStringLiteral("ItemA")));
                add(createObject(2, QStringLiteral("ParentA")));
            });
        });
        query.setConvertFunction(convertToPair);
        query.setPredicateFunction(isProject);
        query.setCompareFunction(compareObjectIds);
        query.setRepresentsFunction(representsPair);

        Domain::QueryResult<QPair<int, QString>>::Ptr result = query.result();
        QList<QPair<int, QString>> expected{ qMakePair(0, QString::fromLatin1("ProjectA")) };
        QTRY_COMPARE(result->data(), expected);

        // WHEN
        query.setFetchFunction([this] (const Domain::LiveRelationshipQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {});
        });

        // unrelated remove -> ignore
        query.onRemoved(createObject(3, QStringLiteral("ItemB")));
        QTRY_COMPARE(result->data(), expected);

        // remove item -> reset happens
        query.onRemoved(createObject(1, QStringLiteral("ItemA")));

        // THEN
        expected.clear();
        QTRY_COMPARE(result->data(), expected);
    }

    void shouldReactToChanges()
    {
        // GIVEN
        Domain::LiveRelationshipQuery<QObject*, QPair<int, QString>> query;
        query.setFetchFunction([this] (const Domain::LiveRelationshipQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add(createObject(0, QStringLiteral("ProjectA")));
                add(createObject(1, QStringLiteral("ItemA")));
                add(createObject(2, QStringLiteral("ParentA")));
                add(createObject(3, QStringLiteral("ProjectB")));
                add(createObject(4, QStringLiteral("ItemB")));
                add(createObject(5, QStringLiteral("ParentB")));
                add(createObject(6, QStringLiteral("ProjectC")));
                add(createObject(7, QStringLiteral("ItemC")));
                add(createObject(8, QStringLiteral("ParentC")));
            });
        });
        query.setConvertFunction(convertToPair);
        query.setPredicateFunction(isProject);
        query.setCompareFunction(compareObjectIds);
        query.setRepresentsFunction(representsPair);

        Domain::QueryResult<QPair<int, QString>>::Ptr result = query.result();
        QList<QPair<int, QString>> expected{ qMakePair(0, QString::fromLatin1("ProjectA")),
                                             qMakePair(3, QString::fromLatin1("ProjectB")),
                                             qMakePair(6, QString::fromLatin1("ProjectC")) };
        QTRY_COMPARE(result->data(), expected);

        // WHEN
        query.setFetchFunction([this] (const Domain::LiveRelationshipQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add(createObject(0, QStringLiteral("ProjectA")));
                add(createObject(1, QStringLiteral("ItemA")));
                add(createObject(2, QStringLiteral("ParentA")));
                add(createObject(3, QStringLiteral("ProjectB-Renamed")));
                add(createObject(4, QStringLiteral("ItemB")));
                add(createObject(5, QStringLiteral("ParentB")));
                add(createObject(6, QStringLiteral("ProjectC")));
                add(createObject(7, QStringLiteral("ItemC")));
                add(createObject(8, QStringLiteral("ParentC")));
            });
        });
        query.onChanged(createObject(3, QStringLiteral("whatever")));

        // THEN
        expected[1] = qMakePair(3, QString::fromLatin1("ProjectB-Renamed"));
        QTRY_COMPARE(result->data(), expected);
    }

    void shouldIgnoreUnrelatedChangesWhenEmpty()
    {
        // GIVEN
        Domain::LiveRelationshipQuery<QObject*, QPair<int, QString>> query;
        bool listingDone = false;
        query.setFetchFunction([this, &listingDone] (const Domain::LiveRelationshipQuery<QObject*, QString>::AddFunction &add) {
            Q_UNUSED(add);
            Utils::JobHandler::install(new FakeJob, [&listingDone] {
                listingDone = true;
            });
        });
        query.setConvertFunction(convertToPair);
        query.setPredicateFunction(isProject);
        query.setCompareFunction(compareObjectIds);
        query.setRepresentsFunction(representsPair);

        Domain::QueryResult<QPair<int, QString>>::Ptr result = query.result();
        QTRY_VERIFY(listingDone);
        listingDone = false;
        QVERIFY(result->data().isEmpty());

        // WHEN
        query.onChanged(createObject(1, QStringLiteral("ProjectA")));

        // THEN
        QTest::qWait(150);
        QVERIFY(!listingDone);
        QVERIFY(result->data().isEmpty());
    }

    void shouldAddWhenChangesMakeInputSuitableForQuery()
    {
        // GIVEN
        Domain::LiveRelationshipQuery<QObject*, QPair<int, QString>> query;
        bool listingDone = false;
        query.setFetchFunction([this, &listingDone] (const Domain::LiveRelationshipQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add, &listingDone] {
                add(createObject(1, QStringLiteral("ItemA")));
                add(createObject(2, QStringLiteral("ParentA")));
                listingDone = true;
            });
        });
        query.setConvertFunction(convertToPair);
        query.setPredicateFunction(isProject);
        query.setCompareFunction(compareObjectIds);
        query.setRepresentsFunction(representsPair);

        Domain::QueryResult<QPair<int, QString>>::Ptr result = query.result();
        QList<QPair<int, QString>> expected;
        QTRY_VERIFY(listingDone);
        QCOMPARE(result->data(), expected);

        // WHEN
        query.setFetchFunction([this] (const Domain::LiveRelationshipQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, add] {
                add(createObject(1, QStringLiteral("ItemA")));
                add(createObject(2, QStringLiteral("ProjectA"))); // parent promoted to project
            });
        });
        query.onChanged(createObject(2, QStringLiteral("whatever")));

        // Then
        expected << qMakePair(2, QStringLiteral("ProjectA"));
        QTRY_COMPARE(result->data(), expected);
    }

    void shouldEmptyAndFetchAgainOnReset()
    {
        // GIVEN
        bool afterReset = false;

        Domain::LiveRelationshipQuery<QObject*, QPair<int, QString>> query;
        query.setFetchFunction([this, &afterReset] (const Domain::LiveRelationshipQuery<QObject*, QString>::AddFunction &add) {
            Utils::JobHandler::install(new FakeJob, [this, &afterReset, add] {
                add(createObject(0, QStringLiteral("ProjectA")));
                add(createObject(1, QStringLiteral("ItemA")));
                add(createObject(2, QStringLiteral("ParentA")));
                add(createObject(3, QStringLiteral("ProjectB")));
                add(createObject(4, QStringLiteral("ItemB")));
                add(createObject(5, QStringLiteral("ParentB")));

                if (afterReset) {
                    add(createObject(6, QStringLiteral("ProjectC")));
                    add(createObject(7, QStringLiteral("ItemC")));
                    add(createObject(8, QStringLiteral("ParentC")));
                }
            });
        });
        query.setConvertFunction(convertToPair);
        query.setPredicateFunction([&afterReset] (QObject *object) {
            if (afterReset)
                return object->objectName().startsWith(QLatin1String("Item"));
            else
                return object->objectName().startsWith(QLatin1String("Project"));
        });
        query.setCompareFunction(compareObjectIds);

        Domain::QueryResult<QPair<int, QString>>::Ptr result = query.result();
        int removeHandlerCallCount = 0;
        result->addPostRemoveHandler([&removeHandlerCallCount](const QPair<int, QString> &, int) {
                                         removeHandlerCallCount++;
                                     });

        QTRY_VERIFY(!result->data().isEmpty());
        QCOMPARE(removeHandlerCallCount, 0);

        // WHEN
        query.reset();
        afterReset = true;

        // THEN
        const QList<QPair<int, QString>> expected = { qMakePair(1, QStringLiteral("ItemA")),
                                                      qMakePair(4, QStringLiteral("ItemB")),
                                                      qMakePair(7, QStringLiteral("ItemC")) };
        QTRY_COMPARE(result->data(), expected);
        QCOMPARE(removeHandlerCallCount, 2);
    }
};

ZANSHIN_TEST_MAIN(LiveRelationshipQueryTest)

#include "liverelationshipquerytest.moc"
