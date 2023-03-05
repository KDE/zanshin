/*
 * SPDX-FileCopyrightText: 2015 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include <KCalendarCore/Todo>

#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"

#include "akonadi/akonadilivequeryintegrator.h"
#include "akonadi/akonadiserializer.h"
#include "akonadi/akonadistorage.h"

#include "testlib/akonadifakedata.h"
#include "testlib/gencollection.h"
#include "testlib/gentodo.h"
#include "testlib/testhelpers.h"

#include "utils/jobhandler.h"

using namespace Testlib;

static QString titleFromItem(const Akonadi::Item &item)
{
    if (item.hasPayload<KCalendarCore::Todo::Ptr>()) {
        const auto todo = item.payload<KCalendarCore::Todo::Ptr>();
        return todo->summary();
    } else {
        return QString();
    }
}

class AkonadiLiveQueryIntegratorTest : public QObject
{
    Q_OBJECT

private:
    Akonadi::LiveQueryIntegrator::Ptr createIntegrator(AkonadiFakeData &data)
    {
        return Akonadi::LiveQueryIntegrator::Ptr(
                    new Akonadi::LiveQueryIntegrator(createSerializer(),
                                                     Akonadi::MonitorInterface::Ptr(data.createMonitor())
                                                    )
                    );
    }

    Akonadi::StorageInterface::Ptr createStorage(AkonadiFakeData &data)
    {
        return Akonadi::StorageInterface::Ptr(data.createStorage());
    }

    Akonadi::SerializerInterface::Ptr createSerializer()
    {
        return Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
    }

    auto fetchCollectionsFunction(Akonadi::StorageInterface::Ptr storage, QObject *parent) {
        return [storage, parent] (const Domain::LiveQueryInput<Akonadi::Collection>::AddFunction &add) {
            auto job = storage->fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive, parent);
            Utils::JobHandler::install(job->kjob(), [add, job] {
                foreach (const auto &col, job->collections()) {
                    add(col);
                }
            });
        };
    }

    auto fetchItemsInAllCollectionsFunction(Akonadi::StorageInterface::Ptr storage, QObject *parent) {
        return [storage, parent] (const Domain::LiveQueryInput<Akonadi::Item>::AddFunction &add) {
            auto job = storage->fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive, parent);
            Utils::JobHandler::install(job->kjob(), [add, job, storage] {
                foreach (const auto &col, job->collections()) {
                    auto itemJob = storage->fetchItems(col, nullptr);
                    Utils::JobHandler::install(itemJob->kjob(), [add, itemJob] {
                        foreach (const auto &item, itemJob->items())
                            add(item);
                    });
                }
            });
        };
    }

    auto fetchItemsInSelectedCollectionsFunction(Akonadi::StorageInterface::Ptr storage, Akonadi::SerializerInterface::Ptr serializer, QObject *parent)
    {
        return [storage, serializer, parent] (const Domain::LiveQueryInput<Akonadi::Item>::AddFunction &add) {
            auto job = storage->fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive, parent);
            Utils::JobHandler::install(job->kjob(), [add, job, storage, serializer] {
                foreach (const auto &col, job->collections()) {
                    if (!serializer->isSelectedCollection(col))
                        continue;

                    auto itemJob = storage->fetchItems(col, nullptr);
                    Utils::JobHandler::install(itemJob->kjob(), [add, itemJob] {
                        foreach (const auto &item, itemJob->items())
                            add(item);
                    });
                }
            });
        };
    }

private slots:
    void shouldBindContextQueries()
    {
        // GIVEN
        AkonadiFakeData data;

        // One toplevel collection
        const auto collection = Akonadi::Collection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("folder")).withTaskContent());
        data.createCollection(collection);

        // Three context todos, one not matching the predicate
        data.createItem(GenTodo().withParent(42).withId(42).withUid("ctx-42").asContext().withTitle(QStringLiteral("42-in")));
        data.createItem(GenTodo().withParent(42).withId(43).withUid("ctx-43").asContext().withTitle(QStringLiteral("43-in")));
        data.createItem(GenTodo().withParent(42).withId(44).withUid("ctx-44").asContext().withTitle(QStringLiteral("44-ex")));

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);

        auto query = Domain::LiveQueryOutput<Domain::Context::Ptr>::Ptr();
        auto fetch = [storage, collection] (const Domain::LiveQueryInput<Akonadi::Item>::AddFunction &add) {
            auto job = storage->fetchItems(collection, nullptr);
            Utils::JobHandler::install(job->kjob(), [add, job] {
                foreach (const auto &item, job->items()) {
                    add(item);
                }
            });
        };
        auto predicate = [] (const Akonadi::Item &contextItem) {
            auto todo = contextItem.payload<KCalendarCore::Todo::Ptr>();
            return todo->summary().endsWith(QLatin1String("-in"));
        };

        // Initial listing
        // WHEN
        integrator->bind("context1", query, fetch, predicate);
        auto result = query->result();
        result->data();
        integrator->bind("context2", query, fetch, predicate);
        result = query->result(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));

        // Reacts to add
        // WHEN
        data.createItem(GenTodo().withId(45).withUid("ctx-45").asContext().withTitle(QStringLiteral("45-in")));

        // THEN
        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));
        QCOMPARE(result->data().at(2)->name(), QStringLiteral("45-in"));

        // Reacts to remove
        // WHEN
        data.removeItem(Akonadi::Item(45));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));

        // Reacts to change
        // WHEN
        data.modifyItem(GenTodo(data.item(42)).withTitle(QStringLiteral("42-bis-in")));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-bis-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));

        // Reacts to change (which adds)
        // WHEN
        data.modifyItem(GenTodo(data.item(44)).withTitle(QStringLiteral("44-in")));

        // THEN
        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-bis-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));
        QCOMPARE(result->data().at(2)->name(), QStringLiteral("44-in"));

        // Reacts to change (which removes)
        // WHEN
        data.modifyItem(GenTodo(data.item(44)).withTitle(QStringLiteral("44-ex")));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-bis-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));

        // Don't keep a reference on any result
        result.clear();

        // The bug we're trying to hit here is the following:
        //  - when bind is called the first time a provider is created internally
        //  - result is deleted at the end of the loop, no one holds the provider with
        //    a strong reference anymore so it is deleted as well
        //  - when bind is called the second time, there's a risk of a dangling
        //    pointer if the recycling of providers is wrongly implemented which can lead
        //    to a crash, if it is properly done no crash will occur
        for (int i = 0; i < 2; i++) {
            // WHEN * 2
            integrator->bind("contextN", query, fetch, predicate);
            auto result = query->result();

            // THEN * 2
            QVERIFY(result->data().isEmpty());
            TestHelpers::waitForEmptyJobQueue();
            QVERIFY(!result->data().isEmpty());
        }
    }

    void shouldMoveContextBetweenQueries()
    {
        // GIVEN
        AkonadiFakeData data;

        // One toplevel collection
        const auto collection = Akonadi::Collection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("folder")).withTaskContent());
        data.createCollection(collection);

        // One context which shows in one query not the other
        data.createItem(GenTodo().withParent(42).withId(42).withUid("ctx-42").asContext().withTitle(QStringLiteral("42-in")));

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);

        auto inQuery = Domain::LiveQueryOutput<Domain::Context::Ptr>::Ptr();
        auto exQuery = Domain::LiveQueryOutput<Domain::Context::Ptr>::Ptr();
        auto fetch = [storage, collection] (const Domain::LiveQueryInput<Akonadi::Item>::AddFunction &add) {
            auto job = storage->fetchItems(collection, nullptr);
            Utils::JobHandler::install(job->kjob(), [add, job] {
                foreach (const auto &item, job->items()) {
                    add(item);
                }
            });
        };
        auto inPredicate = [] (const Akonadi::Item &contextItem) {
            auto todo = contextItem.payload<KCalendarCore::Todo::Ptr>();
            return todo->summary().endsWith(QLatin1String("-in"));
        };
        auto exPredicate = [] (const Akonadi::Item &contextItem) {
            auto todo = contextItem.payload<KCalendarCore::Todo::Ptr>();
            return todo->summary().endsWith(QLatin1String("-ex"));
        };

        integrator->bind("context-in", inQuery, fetch, inPredicate);
        auto inResult = inQuery->result();

        integrator->bind("context-ex", exQuery, fetch, exPredicate);
        auto exResult = exQuery->result();

        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(inResult->data().size(), 1);
        QCOMPARE(exResult->data().size(), 0);

        // WHEN
        data.modifyItem(GenTodo(data.item(42)).withTitle(QStringLiteral("42-ex")));

        // THEN
        QCOMPARE(inResult->data().size(), 0);
        QCOMPARE(exResult->data().size(), 1);
    }




    void shouldBindDataSourceQueries()
    {
        // GIVEN
        AkonadiFakeData data;

        // Three top level collections, one not matching the predicate
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42-in")).withTaskContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withName(QStringLiteral("43-in")).withTaskContent());
        data.createCollection(GenCollection().withId(44).withRootAsParent().withName(QStringLiteral("44-ex")).withTaskContent());

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);

        auto query = Domain::LiveQueryOutput<Domain::DataSource::Ptr>::Ptr();
        auto fetch = fetchCollectionsFunction(storage, nullptr);
        auto predicate = [] (const Akonadi::Collection &collection) {
            return collection.name().endsWith(QLatin1String("-in"));
        };

        // Initial listing
        // WHEN
        integrator->bind("ds1", query, fetch, predicate);
        auto result = query->result();
        result->data();
        integrator->bind("ds2", query, fetch, predicate);
        result = query->result(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));

        // Reacts to add
        // WHEN
        data.createCollection(GenCollection().withId(45).withRootAsParent().withName(QStringLiteral("45-in")));

        // THEN
        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));
        QCOMPARE(result->data().at(2)->name(), QStringLiteral("45-in"));

        // Reacts to remove
        // WHEN
        data.removeCollection(Akonadi::Collection(45));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));

        // Reacts to change
        // WHEN
        data.modifyCollection(GenCollection(data.collection(42)).withName(QStringLiteral("42-bis-in")));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-bis-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));

        // Reacts to change (which adds)
        // WHEN
        data.modifyCollection(GenCollection(data.collection(44)).withName(QStringLiteral("44-in")));

        // THEN
        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-bis-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));
        QCOMPARE(result->data().at(2)->name(), QStringLiteral("44-in"));

        // Reacts to change (which removes)
        // WHEN
        data.modifyCollection(GenCollection(data.collection(44)).withName(QStringLiteral("44-ex")));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-bis-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));

        // Don't keep a reference on any result
        result.clear();

        // The bug we're trying to hit here is the following:
        //  - when bind is called the first time a provider is created internally
        //  - result is deleted at the end of the loop, no one holds the provider with
        //    a strong reference anymore so it is deleted as well
        //  - when bind is called the second time, there's a risk of a dangling
        //    pointer if the recycling of providers is wrongly implemented which can lead
        //    to a crash, if it is properly done no crash will occur
        for (int i = 0; i < 2; i++) {
            // WHEN * 2
            integrator->bind("dsN", query, fetch, predicate);
            auto result = query->result();

            // THEN * 2
            QVERIFY(result->data().isEmpty());
            TestHelpers::waitForEmptyJobQueue();
            QVERIFY(!result->data().isEmpty());
        }
    }

    void shouldMoveDataSourceBetweenQueries()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection which shows in one query not the other
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42-in")).withTaskContent());

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);

        auto inQuery = Domain::LiveQueryOutput<Domain::DataSource::Ptr>::Ptr();
        auto exQuery = Domain::LiveQueryOutput<Domain::DataSource::Ptr>::Ptr();
        auto fetch = fetchCollectionsFunction(storage, nullptr);
        auto inPredicate = [] (const Akonadi::Collection &collection) {
            return collection.name().endsWith(QLatin1String("-in"));
        };
        auto exPredicate = [] (const Akonadi::Collection &collection) {
            return collection.name().endsWith(QLatin1String("-ex"));
        };

        integrator->bind("ds-in", inQuery, fetch, inPredicate);
        auto inResult = inQuery->result();

        integrator->bind("ds-ex", exQuery, fetch, exPredicate);
        auto exResult = exQuery->result();

        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(inResult->data().size(), 1);
        QCOMPARE(exResult->data().size(), 0);

        // WHEN
        data.modifyCollection(GenCollection(data.collection(42)).withName(QStringLiteral("42-ex")));

        // THEN
        QCOMPARE(inResult->data().size(), 0);
        QCOMPARE(exResult->data().size(), 1);
    }




    void shouldBindProjectQueries()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")).withTaskContent());

        // Three projects in the collection, one not matching the predicate
        data.createItem(GenTodo().withId(42).withParent(42).asProject().withTitle(QStringLiteral("42-in")));
        data.createItem(GenTodo().withId(43).withParent(42).asProject().withTitle(QStringLiteral("43-in")));
        data.createItem(GenTodo().withId(44).withParent(42).asProject().withTitle(QStringLiteral("44-ex")));

        // Couple of tasks in the collection which should not appear or create trouble
        data.createItem(GenTodo().withId(40).withParent(42).withTitle(QStringLiteral("40")));
        data.createItem(GenTodo().withId(41).withParent(42).withTitle(QStringLiteral("41-in")));

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);

        auto query = Domain::LiveQueryOutput<Domain::Project::Ptr>::Ptr();
        auto fetch = fetchItemsInAllCollectionsFunction(storage, nullptr);
        auto predicate = [] (const Akonadi::Item &item) {
            return titleFromItem(item).endsWith(QLatin1String("-in"));
        };

        // Initial listing
        // WHEN
        integrator->bind("project1", query, fetch, predicate);
        auto result = query->result();
        result->data();
        integrator->bind("project2", query, fetch, predicate);
        result = query->result(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));

        // Reacts to add
        // WHEN
        data.createItem(GenTodo().withId(45).withParent(42).asProject().withTitle(QStringLiteral("45-in")));

        // THEN
        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));
        QCOMPARE(result->data().at(2)->name(), QStringLiteral("45-in"));

        // Reacts to remove
        // WHEN
        data.removeItem(Akonadi::Item(45));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));

        // Reacts to change
        // WHEN
        data.modifyItem(GenTodo(data.item(42)).withTitle(QStringLiteral("42-bis-in")));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-bis-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));

        // Reacts to change (which adds)
        // WHEN
        data.modifyItem(GenTodo(data.item(44)).withTitle(QStringLiteral("44-in")));

        // THEN
        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-bis-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));
        QCOMPARE(result->data().at(2)->name(), QStringLiteral("44-in"));

        // Reacts to change (which removes)
        // WHEN
        data.modifyItem(GenTodo(data.item(44)).withTitle(QStringLiteral("44-ex")));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-bis-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));

        // Don't keep a reference on any result
        result.clear();

        // The bug we're trying to hit here is the following:
        //  - when bind is called the first time a provider is created internally
        //  - result is deleted at the end of the loop, no one holds the provider with
        //    a strong reference anymore so it is deleted as well
        //  - when bind is called the second time, there's a risk of a dangling
        //    pointer if the recycling of providers is wrongly implemented which can lead
        //    to a crash, if it is properly done no crash will occur
        for (int i = 0; i < 2; i++) {
            // WHEN * 2
            integrator->bind("projectN", query, fetch, predicate);
            auto result = query->result();

            // THEN * 2
            QVERIFY(result->data().isEmpty());
            TestHelpers::waitForEmptyJobQueue();
            QVERIFY(!result->data().isEmpty());
        }
    }

    void shouldMoveProjectsBetweenQueries()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")).withTaskContent());

        // One project which shows in one query and not the other
        data.createItem(GenTodo().withId(42).withParent(42).asProject().withTitle(QStringLiteral("42-in")));

        // Couple of tasks in the collection which should not appear or create trouble
        data.createItem(GenTodo().withId(39).withParent(42).withTitle(QStringLiteral("39")));
        data.createItem(GenTodo().withId(40).withParent(42).withTitle(QStringLiteral("40-ex")));
        data.createItem(GenTodo().withId(41).withParent(42).withTitle(QStringLiteral("41-in")));

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);

        auto inQuery = Domain::LiveQueryOutput<Domain::Project::Ptr>::Ptr();
        auto exQuery = Domain::LiveQueryOutput<Domain::Project::Ptr>::Ptr();
        auto fetch = fetchItemsInAllCollectionsFunction(storage, nullptr);
        auto inPredicate = [] (const Akonadi::Item &item) {
            return titleFromItem(item).endsWith(QLatin1String("-in"));
        };
        auto exPredicate = [] (const Akonadi::Item &item) {
            return titleFromItem(item).endsWith(QLatin1String("-ex"));
        };

        integrator->bind("project-in", inQuery, fetch, inPredicate);
        auto inResult = inQuery->result();

        integrator->bind("project-ex", exQuery, fetch, exPredicate);
        auto exResult = exQuery->result();

        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(inResult->data().size(), 1);
        QCOMPARE(exResult->data().size(), 0);

        // WHEN
        data.modifyItem(GenTodo(data.item(42)).withTitle(QStringLiteral("42-ex")));

        // THEN
        QCOMPARE(inResult->data().size(), 0);
        QCOMPARE(exResult->data().size(), 1);
    }

    void shouldReactToCollectionSelectionChangesForProjectQueries()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withTaskContent());

        // One project in each collection
        data.createItem(GenTodo().withId(42).withParent(42).asProject().withTitle(QStringLiteral("42")));
        data.createItem(GenTodo().withId(43).withParent(43).asProject().withTitle(QStringLiteral("43")));

        // Couple of tasks in the collections which should not appear or create trouble
        data.createItem(GenTodo().withId(40).withParent(42).withTitle(QStringLiteral("40")));
        data.createItem(GenTodo().withId(41).withParent(43).withTitle(QStringLiteral("41")));

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);
        auto serializer = createSerializer();

        auto query = Domain::LiveQueryOutput<Domain::Project::Ptr>::Ptr();
        auto fetch = fetchItemsInSelectedCollectionsFunction(storage, serializer, nullptr);
        auto predicate = [] (const Akonadi::Item &) {
            return true;
        };

        integrator->bind("project query", query, fetch, predicate);
        auto result = query->result();
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.modifyCollection(GenCollection(data.collection(43)).selected(false));
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42"));
    }





    void shouldBindTaskQueries()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")).withTaskContent());

        // Three tasks in the collection, one not matching the predicate
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42-in")));
        data.createItem(GenTodo().withId(43).withParent(42).withTitle(QStringLiteral("43-in")));
        data.createItem(GenTodo().withId(44).withParent(42).withTitle(QStringLiteral("44-ex")));

        // Couple of projects in the collection which should not appear or create trouble
        data.createItem(GenTodo().withId(38).withParent(42).asProject().withTitle(QStringLiteral("38")));
        data.createItem(GenTodo().withId(39).withParent(42).asProject().withTitle(QStringLiteral("39-in")));

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);

        auto query = Domain::LiveQueryOutput<Domain::Task::Ptr>::Ptr();
        auto fetch = fetchItemsInAllCollectionsFunction(storage, nullptr);
        auto predicate = [] (const Akonadi::Item &item) {
            return titleFromItem(item).endsWith(QLatin1String("-in"));
        };

        // Initial listing
        // WHEN
        integrator->bind("task1", query, fetch, predicate);
        auto result = query->result();
        result->data();
        integrator->bind("task2", query, fetch, predicate);
        result = query->result(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("42-in"));
        QCOMPARE(result->data().at(1)->title(), QStringLiteral("43-in"));

        // Reacts to add
        // WHEN
        data.createItem(GenTodo().withId(45).withParent(42).withTitle(QStringLiteral("45-in")));

        // THEN
        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("42-in"));
        QCOMPARE(result->data().at(1)->title(), QStringLiteral("43-in"));
        QCOMPARE(result->data().at(2)->title(), QStringLiteral("45-in"));

        // Reacts to remove
        // WHEN
        data.removeItem(Akonadi::Item(45));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("42-in"));
        QCOMPARE(result->data().at(1)->title(), QStringLiteral("43-in"));

        // Reacts to change
        // WHEN
        data.modifyItem(GenTodo(data.item(42)).withTitle(QStringLiteral("42-bis-in")));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("42-bis-in"));
        QCOMPARE(result->data().at(1)->title(), QStringLiteral("43-in"));

        // Reacts to change (which adds)
        // WHEN
        data.modifyItem(GenTodo(data.item(44)).withTitle(QStringLiteral("44-in")));

        // THEN
        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("42-bis-in"));
        QCOMPARE(result->data().at(1)->title(), QStringLiteral("43-in"));
        QCOMPARE(result->data().at(2)->title(), QStringLiteral("44-in"));

        // Reacts to change (which removes)
        // WHEN
        data.modifyItem(GenTodo(data.item(44)).withTitle(QStringLiteral("44-ex")));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("42-bis-in"));
        QCOMPARE(result->data().at(1)->title(), QStringLiteral("43-in"));

        // Don't keep a reference on any result
        result.clear();

        // The bug we're trying to hit here is the following:
        //  - when bind is called the first time a provider is created internally
        //  - result is deleted at the end of the loop, no one holds the provider with
        //    a strong reference anymore so it is deleted as well
        //  - when bind is called the second time, there's a risk of a dangling
        //    pointer if the recycling of providers is wrongly implemented which can lead
        //    to a crash, if it is properly done no crash will occur
        for (int i = 0; i < 2; i++) {
            // WHEN * 2
            integrator->bind("taskN", query, fetch, predicate);
            auto result = query->result();

            // THEN * 2
            QVERIFY(result->data().isEmpty());
            TestHelpers::waitForEmptyJobQueue();
            QVERIFY(!result->data().isEmpty());
        }
    }

    void shouldMoveTasksBetweenQueries()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")).withTaskContent());

        // One task which shows in one query and not the other
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42-in")));

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);

        auto inQuery = Domain::LiveQueryOutput<Domain::Task::Ptr>::Ptr();
        auto exQuery = Domain::LiveQueryOutput<Domain::Task::Ptr>::Ptr();
        auto fetch = fetchItemsInAllCollectionsFunction(storage, nullptr);
        auto inPredicate = [] (const Akonadi::Item &item) {
            return titleFromItem(item).endsWith(QLatin1String("-in"));
        };
        auto exPredicate = [] (const Akonadi::Item &item) {
            return titleFromItem(item).endsWith(QLatin1String("-ex"));
        };

        integrator->bind("task-in", inQuery, fetch, inPredicate);
        auto inResult = inQuery->result();

        integrator->bind("task-ex", exQuery, fetch, exPredicate);
        auto exResult = exQuery->result();

        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(inResult->data().size(), 1);
        QCOMPARE(exResult->data().size(), 0);

        // WHEN
        data.modifyItem(GenTodo(data.item(42)).withTitle(QStringLiteral("42-ex")));

        // THEN
        QCOMPARE(inResult->data().size(), 0);
        QCOMPARE(exResult->data().size(), 1);
    }

    void shouldReactToCollectionSelectionChangesForTaskQueries()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections
        data.createCollection(GenCollection().withId(42).withRootAsParent().withTaskContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withTaskContent());

        // One task in each collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42")));
        data.createItem(GenTodo().withId(43).withParent(43).withTitle(QStringLiteral("43")));

        // Couple of projects in the collections which should not appear or create trouble
        data.createItem(GenTodo().withId(40).withParent(42).asProject().withTitle(QStringLiteral("40")));
        data.createItem(GenTodo().withId(41).withParent(43).asProject().withTitle(QStringLiteral("41")));

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);
        auto serializer = createSerializer();

        auto query = Domain::LiveQueryOutput<Domain::Task::Ptr>::Ptr();
        auto fetch = fetchItemsInSelectedCollectionsFunction(storage, serializer, nullptr);
        auto predicate = [] (const Akonadi::Item &) {
            return true;
        };

        integrator->bind("task query", query, fetch, predicate);
        auto result = query->result();
        TestHelpers::waitForEmptyJobQueue();
        QCOMPARE(result->data().size(), 2);

        // WHEN
        data.modifyCollection(GenCollection(data.collection(43)).selected(false));
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("42"));
    }




    void shouldCallCollectionRemoveHandlers()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")));

        auto integrator = createIntegrator(data);
        qint64 removedId = -1;
        integrator->addRemoveHandler([&removedId] (const Akonadi::Collection &collection) {
            removedId = collection.id();
        });

        // WHEN
        data.removeCollection(Akonadi::Collection(42));

        // THEN
        QCOMPARE(removedId, qint64(42));
    }

    void shouldCallItemRemoveHandlers()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")));

        // One item in the collection
        data.createItem(GenTodo().withId(42).withParent(42));

        auto integrator = createIntegrator(data);
        qint64 removedId = -1;
        integrator->addRemoveHandler([&removedId] (const Akonadi::Item &item) {
            removedId = item.id();
        });

        // WHEN
        data.removeItem(Akonadi::Item(42));

        // THEN
        QCOMPARE(removedId, qint64(42));
    }

};

ZANSHIN_TEST_MAIN(AkonadiLiveQueryIntegratorTest)

#include "akonadilivequeryintegratortest.moc"
