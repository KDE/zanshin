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

#include <testlib/qtest_zanshin.h>

#include <KCalCore/Todo>
#include <KMime/Message>
#include <Akonadi/Notes/NoteUtils>

#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"
#include "akonadi/akonaditagfetchjobinterface.h"

#include "akonadi/akonadilivequeryintegrator.h"
#include "akonadi/akonadiserializer.h"
#include "akonadi/akonadistorage.h"

#include "testlib/akonadifakedata.h"
#include "testlib/gencollection.h"
#include "testlib/gennote.h"
#include "testlib/gentag.h"
#include "testlib/gentodo.h"
#include "testlib/testhelpers.h"

#include "utils/jobhandler.h"

using namespace Testlib;

static QString titleFromItem(const Akonadi::Item &item)
{
    if (item.hasPayload<KCalCore::Todo::Ptr>()) {
        const auto todo = item.payload<KCalCore::Todo::Ptr>();
        return todo->summary();

    } else if (item.hasPayload<KMime::Message::Ptr>()) {
        const auto message = item.payload<KMime::Message::Ptr>();
        const Akonadi::NoteUtils::NoteMessageWrapper note(message);
        return note.title();

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

private slots:
    void shouldBindArtifactQueries()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")));

        // Three artifacts in the collection, one not matching the predicate
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42-in")));
        data.createItem(GenNote().withId(43).withParent(42).withTitle(QStringLiteral("43-in")));
        data.createItem(GenNote().withId(44).withParent(42).withTitle(QStringLiteral("44-ex")));

        // Couple of projects in the collection which should not appear or create trouble
        data.createItem(GenTodo().withId(40).withParent(42).asProject().withTitle(QStringLiteral("40")));
        data.createItem(GenTodo().withId(41).withParent(42).asProject().withTitle(QStringLiteral("41-in")));

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);

        auto query = Domain::LiveQueryOutput<Domain::Artifact::Ptr>::Ptr();
        auto fetch = [storage] (const Domain::LiveQueryInput<Akonadi::Item>::AddFunction &add) {
            auto job = storage->fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive, Akonadi::Storage::AllContent);
            Utils::JobHandler::install(job->kjob(), [add, job, storage] {
                foreach (const auto &col, job->collections()) {
                    auto itemJob = storage->fetchItems(col);
                    Utils::JobHandler::install(itemJob->kjob(), [add, itemJob] {
                        foreach (const auto &item, itemJob->items())
                            add(item);
                    });
                }
            });
        };
        auto predicate = [] (const Akonadi::Item &item) {
            return titleFromItem(item).endsWith(QLatin1String("-in"));
        };

        // Initial listing
        // WHEN
        integrator->bind(query, fetch, predicate);
        auto result = query->result();
        result->data();
        integrator->bind(query, fetch, predicate);
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
        data.modifyItem(GenNote(data.item(44)).withTitle(QStringLiteral("44-in")));

        // THEN
        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("42-bis-in"));
        QCOMPARE(result->data().at(1)->title(), QStringLiteral("43-in"));
        QCOMPARE(result->data().at(2)->title(), QStringLiteral("44-in"));

        // Reacts to change (which removes)
        // WHEN
        data.modifyItem(GenNote(data.item(44)).withTitle(QStringLiteral("44-ex")));

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
            integrator->bind(query, fetch, predicate);
            auto result = query->result();

            // THEN * 2
            QVERIFY(result->data().isEmpty());
            TestHelpers::waitForEmptyJobQueue();
            QVERIFY(!result->data().isEmpty());
        }
    }

    void shouldMoveArtifactsBetweenQueries()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")));

        // One task and one note which show in one query and not the other
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42-in")));
        data.createItem(GenNote().withId(43).withParent(42).withTitle(QStringLiteral("43-in")));

        // Couple of projects in the collection which should not appear or create trouble
        data.createItem(GenTodo().withId(39).withParent(42).asProject().withTitle(QStringLiteral("39")));
        data.createItem(GenTodo().withId(40).withParent(42).asProject().withTitle(QStringLiteral("40-ex")));
        data.createItem(GenTodo().withId(41).withParent(42).asProject().withTitle(QStringLiteral("41-in")));

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);

        auto inQuery = Domain::LiveQueryOutput<Domain::Artifact::Ptr>::Ptr();
        auto exQuery = Domain::LiveQueryOutput<Domain::Artifact::Ptr>::Ptr();
        auto fetch = [storage] (const Domain::LiveQueryInput<Akonadi::Item>::AddFunction &add) {
            auto job = storage->fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive, Akonadi::Storage::AllContent);
            Utils::JobHandler::install(job->kjob(), [add, job, storage] {
                foreach (const auto &col, job->collections()) {
                    auto itemJob = storage->fetchItems(col);
                    Utils::JobHandler::install(itemJob->kjob(), [add, itemJob] {
                        foreach (const auto &item, itemJob->items())
                            add(item);
                    });
                }
            });
        };
        auto inPredicate = [] (const Akonadi::Item &item) {
            return titleFromItem(item).endsWith(QLatin1String("-in"));
        };
        auto exPredicate = [] (const Akonadi::Item &item) {
            return titleFromItem(item).endsWith(QLatin1String("-ex"));
        };

        integrator->bind(inQuery, fetch, inPredicate);
        auto inResult = inQuery->result();

        integrator->bind(exQuery, fetch, exPredicate);
        auto exResult = exQuery->result();

        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(inResult->data().size(), 2);
        QCOMPARE(exResult->data().size(), 0);

        // WHEN
        data.modifyItem(GenTodo(data.item(42)).withTitle(QStringLiteral("42-ex")));
        data.modifyItem(GenNote(data.item(43)).withTitle(QStringLiteral("43-ex")));

        // THEN
        QCOMPARE(inResult->data().size(), 0);
        QCOMPARE(exResult->data().size(), 2);
    }

    void shouldReactToCollectionSelectionChangesForArtifactQueries()
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

        auto query = Domain::LiveQueryOutput<Domain::Artifact::Ptr>::Ptr();
        auto fetch = [storage, serializer] (const Domain::LiveQueryInput<Akonadi::Item>::AddFunction &add) {
            auto job = storage->fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive, Akonadi::Storage::AllContent);
            Utils::JobHandler::install(job->kjob(), [add, job, storage, serializer] {
                foreach (const auto &col, job->collections()) {
                    if (!serializer->isSelectedCollection(col))
                        continue;

                    auto itemJob = storage->fetchItems(col);
                    Utils::JobHandler::install(itemJob->kjob(), [add, itemJob] {
                        foreach (const auto &item, itemJob->items())
                            add(item);
                    });
                }
            });
        };
        auto predicate = [] (const Akonadi::Item &) {
            return true;
        };

        integrator->bind(query, fetch, predicate);
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




    void shouldBindContextQueries()
    {
        // GIVEN
        AkonadiFakeData data;

        // Three context tags, one not matching the predicate
        data.createTag(GenTag().withId(42).asContext().withName(QStringLiteral("42-in")));
        data.createTag(GenTag().withId(43).asContext().withName(QStringLiteral("43-in")));
        data.createTag(GenTag().withId(44).asContext().withName(QStringLiteral("44-ex")));

        // Couple of plain tags which should not appear or create trouble
        data.createTag(GenTag().withId(40).asPlain().withName(QStringLiteral("40")));
        data.createTag(GenTag().withId(41).asPlain().withName(QStringLiteral("41-in")));

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);

        auto query = Domain::LiveQueryOutput<Domain::Context::Ptr>::Ptr();
        auto fetch = [storage] (const Domain::LiveQueryInput<Akonadi::Tag>::AddFunction &add) {
            auto job = storage->fetchTags();
            Utils::JobHandler::install(job->kjob(), [add, job] {
                foreach (const auto &tag, job->tags()) {
                    add(tag);
                }
            });
        };
        auto predicate = [] (const Akonadi::Tag &tag) {
            return tag.name().endsWith(QLatin1String("-in"));
        };

        // Initial listing
        // WHEN
        integrator->bind(query, fetch, predicate);
        auto result = query->result();
        result->data();
        integrator->bind(query, fetch, predicate);
        result = query->result(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));

        // Reacts to add
        // WHEN
        data.createTag(GenTag().withId(45).asContext().withName(QStringLiteral("45-in")));

        // THEN
        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));
        QCOMPARE(result->data().at(2)->name(), QStringLiteral("45-in"));

        // Reacts to remove
        // WHEN
        data.removeTag(Akonadi::Tag(45));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));

        // Reacts to change
        // WHEN
        data.modifyTag(GenTag(data.tag(42)).withName(QStringLiteral("42-bis-in")));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-bis-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));

        // Reacts to change (which adds)
        // WHEN
        data.modifyTag(GenTag(data.tag(44)).withName(QStringLiteral("44-in")));

        // THEN
        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-bis-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));
        QCOMPARE(result->data().at(2)->name(), QStringLiteral("44-in"));

        // Reacts to change (which removes)
        // WHEN
        data.modifyTag(GenTag(data.tag(44)).withName(QStringLiteral("44-ex")));

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
            integrator->bind(query, fetch, predicate);
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

        // One context tag which shows in one query not the other
        data.createTag(GenTag().withId(42).asContext().withName(QStringLiteral("42-in")));

        // Couple of plain tags which should not appear or create trouble
        data.createTag(GenTag().withId(39).asPlain().withName(QStringLiteral("39")));
        data.createTag(GenTag().withId(40).asPlain().withName(QStringLiteral("40-ex")));
        data.createTag(GenTag().withId(41).asPlain().withName(QStringLiteral("41-in")));

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);

        auto inQuery = Domain::LiveQueryOutput<Domain::Context::Ptr>::Ptr();
        auto exQuery = Domain::LiveQueryOutput<Domain::Context::Ptr>::Ptr();
        auto fetch = [storage] (const Domain::LiveQueryInput<Akonadi::Tag>::AddFunction &add) {
            auto job = storage->fetchTags();
            Utils::JobHandler::install(job->kjob(), [add, job] {
                foreach (const auto &tag, job->tags()) {
                    add(tag);
                }
            });
        };
        auto inPredicate = [] (const Akonadi::Tag &tag) {
            return tag.name().endsWith(QLatin1String("-in"));
        };
        auto exPredicate = [] (const Akonadi::Tag &tag) {
            return tag.name().endsWith(QLatin1String("-ex"));
        };

        integrator->bind(inQuery, fetch, inPredicate);
        auto inResult = inQuery->result();

        integrator->bind(exQuery, fetch, exPredicate);
        auto exResult = exQuery->result();

        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(inResult->data().size(), 1);
        QCOMPARE(exResult->data().size(), 0);

        // WHEN
        data.modifyTag(GenTag(data.tag(42)).withName(QStringLiteral("42-ex")));

        // THEN
        QCOMPARE(inResult->data().size(), 0);
        QCOMPARE(exResult->data().size(), 1);
    }




    void shouldBindDataSourceQueries()
    {
        // GIVEN
        AkonadiFakeData data;

        // Three top level collections, one not matching the predicate
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42-in")));
        data.createCollection(GenCollection().withId(43).withRootAsParent().withName(QStringLiteral("43-in")));
        data.createCollection(GenCollection().withId(44).withRootAsParent().withName(QStringLiteral("44-ex")));

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);

        auto query = Domain::LiveQueryOutput<Domain::DataSource::Ptr>::Ptr();
        auto fetch = [storage] (const Domain::LiveQueryInput<Akonadi::Collection>::AddFunction &add) {
            auto job = storage->fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive, Akonadi::Storage::AllContent);
            Utils::JobHandler::install(job->kjob(), [add, job] {
                foreach (const auto &col, job->collections()) {
                    add(col);
                }
            });
        };
        auto predicate = [] (const Akonadi::Collection &collection) {
            return collection.name().endsWith(QLatin1String("-in"));
        };

        // Initial listing
        // WHEN
        integrator->bind(query, fetch, predicate);
        auto result = query->result();
        result->data();
        integrator->bind(query, fetch, predicate);
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
            integrator->bind(query, fetch, predicate);
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
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42-in")));

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);

        auto inQuery = Domain::LiveQueryOutput<Domain::DataSource::Ptr>::Ptr();
        auto exQuery = Domain::LiveQueryOutput<Domain::DataSource::Ptr>::Ptr();
        auto fetch = [storage] (const Domain::LiveQueryInput<Akonadi::Collection>::AddFunction &add) {
            auto job = storage->fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive, Akonadi::Storage::AllContent);
            Utils::JobHandler::install(job->kjob(), [add, job] {
                foreach (const auto &col, job->collections()) {
                    add(col);
                }
            });
        };
        auto inPredicate = [] (const Akonadi::Collection &collection) {
            return collection.name().endsWith(QLatin1String("-in"));
        };
        auto exPredicate = [] (const Akonadi::Collection &collection) {
            return collection.name().endsWith(QLatin1String("-ex"));
        };

        integrator->bind(inQuery, fetch, inPredicate);
        auto inResult = inQuery->result();

        integrator->bind(exQuery, fetch, exPredicate);
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




    void shouldBindNoteQueries()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")));

        // Three notes in the collection, one not matching the predicate
        data.createItem(GenNote().withId(42).withParent(42).withTitle(QStringLiteral("42-in")));
        data.createItem(GenNote().withId(43).withParent(42).withTitle(QStringLiteral("43-in")));
        data.createItem(GenNote().withId(44).withParent(42).withTitle(QStringLiteral("44-ex")));

        // Couple of tasks in the collection which should not appear or create trouble
        data.createItem(GenTodo().withId(40).withParent(42).withTitle(QStringLiteral("40")));
        data.createItem(GenTodo().withId(41).withParent(42).withTitle(QStringLiteral("41-in")));

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);

        auto query = Domain::LiveQueryOutput<Domain::Note::Ptr>::Ptr();
        auto fetch = [storage] (const Domain::LiveQueryInput<Akonadi::Item>::AddFunction &add) {
            auto job = storage->fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive, Akonadi::Storage::AllContent);
            Utils::JobHandler::install(job->kjob(), [add, job, storage] {
                foreach (const auto &col, job->collections()) {
                    auto itemJob = storage->fetchItems(col);
                    Utils::JobHandler::install(itemJob->kjob(), [add, itemJob] {
                        foreach (const auto &item, itemJob->items())
                            add(item);
                    });
                }
            });
        };
        auto predicate = [] (const Akonadi::Item &item) {
            return titleFromItem(item).endsWith(QLatin1String("-in"));
        };

        // Initial listing
        // WHEN
        integrator->bind(query, fetch, predicate);
        auto result = query->result();
        result->data();
        integrator->bind(query, fetch, predicate);
        result = query->result(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("42-in"));
        QCOMPARE(result->data().at(1)->title(), QStringLiteral("43-in"));

        // Reacts to add
        // WHEN
        data.createItem(GenNote().withId(45).withParent(42).withTitle(QStringLiteral("45-in")));

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
        data.modifyItem(GenNote(data.item(42)).withTitle(QStringLiteral("42-bis-in")));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("42-bis-in"));
        QCOMPARE(result->data().at(1)->title(), QStringLiteral("43-in"));

        // Reacts to change (which adds)
        // WHEN
        data.modifyItem(GenNote(data.item(44)).withTitle(QStringLiteral("44-in")));

        // THEN
        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->title(), QStringLiteral("42-bis-in"));
        QCOMPARE(result->data().at(1)->title(), QStringLiteral("43-in"));
        QCOMPARE(result->data().at(2)->title(), QStringLiteral("44-in"));

        // Reacts to change (which removes)
        // WHEN
        data.modifyItem(GenNote(data.item(44)).withTitle(QStringLiteral("44-ex")));

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
            integrator->bind(query, fetch, predicate);
            auto result = query->result();

            // THEN * 2
            QVERIFY(result->data().isEmpty());
            TestHelpers::waitForEmptyJobQueue();
            QVERIFY(!result->data().isEmpty());
        }
    }

    void shouldMoveNotesBetweenQueries()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")));

        // One note which shows in one query and not the other
        data.createItem(GenNote().withId(42).withParent(42).withTitle(QStringLiteral("42-in")));

        // Couple of tasks in the collection which should not appear or create trouble
        data.createItem(GenTodo().withId(39).withParent(42).withTitle(QStringLiteral("39")));
        data.createItem(GenTodo().withId(40).withParent(42).withTitle(QStringLiteral("40-ex")));
        data.createItem(GenTodo().withId(41).withParent(42).withTitle(QStringLiteral("41-in")));

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);

        auto inQuery = Domain::LiveQueryOutput<Domain::Note::Ptr>::Ptr();
        auto exQuery = Domain::LiveQueryOutput<Domain::Note::Ptr>::Ptr();
        auto fetch = [storage] (const Domain::LiveQueryInput<Akonadi::Item>::AddFunction &add) {
            auto job = storage->fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive, Akonadi::Storage::AllContent);
            Utils::JobHandler::install(job->kjob(), [add, job, storage] {
                foreach (const auto &col, job->collections()) {
                    auto itemJob = storage->fetchItems(col);
                    Utils::JobHandler::install(itemJob->kjob(), [add, itemJob] {
                        foreach (const auto &item, itemJob->items())
                            add(item);
                    });
                }
            });
        };
        auto inPredicate = [] (const Akonadi::Item &item) {
            return titleFromItem(item).endsWith(QLatin1String("-in"));
        };
        auto exPredicate = [] (const Akonadi::Item &item) {
            return titleFromItem(item).endsWith(QLatin1String("-ex"));
        };

        integrator->bind(inQuery, fetch, inPredicate);
        auto inResult = inQuery->result();

        integrator->bind(exQuery, fetch, exPredicate);
        auto exResult = exQuery->result();

        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(inResult->data().size(), 1);
        QCOMPARE(exResult->data().size(), 0);

        // WHEN
        data.modifyItem(GenNote(data.item(42)).withTitle(QStringLiteral("42-ex")));

        // THEN
        QCOMPARE(inResult->data().size(), 0);
        QCOMPARE(exResult->data().size(), 1);
    }

    void shouldReactToCollectionSelectionChangesForNoteQueries()
    {
        // GIVEN
        AkonadiFakeData data;

        // Two top level collections
        data.createCollection(GenCollection().withId(42).withRootAsParent().withNoteContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withNoteContent());

        // One note in each collection
        data.createItem(GenNote().withId(42).withParent(42).withTitle(QStringLiteral("42")));
        data.createItem(GenNote().withId(43).withParent(43).withTitle(QStringLiteral("43")));

        // Couple of tasks in the collections which should not appear or create trouble
        data.createItem(GenTodo().withId(40).withParent(42).withTitle(QStringLiteral("40")));
        data.createItem(GenTodo().withId(41).withParent(43).withTitle(QStringLiteral("41")));

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);
        auto serializer = createSerializer();

        auto query = Domain::LiveQueryOutput<Domain::Note::Ptr>::Ptr();
        auto fetch = [storage, serializer] (const Domain::LiveQueryInput<Akonadi::Item>::AddFunction &add) {
            auto job = storage->fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive, Akonadi::Storage::AllContent);
            Utils::JobHandler::install(job->kjob(), [add, job, storage, serializer] {
                foreach (const auto &col, job->collections()) {
                    if (!serializer->isSelectedCollection(col))
                        continue;

                    auto itemJob = storage->fetchItems(col);
                    Utils::JobHandler::install(itemJob->kjob(), [add, itemJob] {
                        foreach (const auto &item, itemJob->items())
                            add(item);
                    });
                }
            });
        };
        auto predicate = [] (const Akonadi::Item &) {
            return true;
        };

        integrator->bind(query, fetch, predicate);
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




    void shouldBindProjectQueries()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")));

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
        auto fetch = [storage] (const Domain::LiveQueryInput<Akonadi::Item>::AddFunction &add) {
            auto job = storage->fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive, Akonadi::Storage::AllContent);
            Utils::JobHandler::install(job->kjob(), [add, job, storage] {
                foreach (const auto &col, job->collections()) {
                    auto itemJob = storage->fetchItems(col);
                    Utils::JobHandler::install(itemJob->kjob(), [add, itemJob] {
                        foreach (const auto &item, itemJob->items())
                            add(item);
                    });
                }
            });
        };
        auto predicate = [] (const Akonadi::Item &item) {
            return titleFromItem(item).endsWith(QLatin1String("-in"));
        };

        // Initial listing
        // WHEN
        integrator->bind(query, fetch, predicate);
        auto result = query->result();
        result->data();
        integrator->bind(query, fetch, predicate);
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
            integrator->bind(query, fetch, predicate);
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
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")));

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
        auto fetch = [storage] (const Domain::LiveQueryInput<Akonadi::Item>::AddFunction &add) {
            auto job = storage->fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive, Akonadi::Storage::AllContent);
            Utils::JobHandler::install(job->kjob(), [add, job, storage] {
                foreach (const auto &col, job->collections()) {
                    auto itemJob = storage->fetchItems(col);
                    Utils::JobHandler::install(itemJob->kjob(), [add, itemJob] {
                        foreach (const auto &item, itemJob->items())
                            add(item);
                    });
                }
            });
        };
        auto inPredicate = [] (const Akonadi::Item &item) {
            return titleFromItem(item).endsWith(QLatin1String("-in"));
        };
        auto exPredicate = [] (const Akonadi::Item &item) {
            return titleFromItem(item).endsWith(QLatin1String("-ex"));
        };

        integrator->bind(inQuery, fetch, inPredicate);
        auto inResult = inQuery->result();

        integrator->bind(exQuery, fetch, exPredicate);
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
        auto fetch = [storage, serializer] (const Domain::LiveQueryInput<Akonadi::Item>::AddFunction &add) {
            auto job = storage->fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive, Akonadi::Storage::AllContent);
            Utils::JobHandler::install(job->kjob(), [add, job, storage, serializer] {
                foreach (const auto &col, job->collections()) {
                    if (!serializer->isSelectedCollection(col))
                        continue;

                    auto itemJob = storage->fetchItems(col);
                    Utils::JobHandler::install(itemJob->kjob(), [add, itemJob] {
                        foreach (const auto &item, itemJob->items())
                            add(item);
                    });
                }
            });
        };
        auto predicate = [] (const Akonadi::Item &) {
            return true;
        };

        integrator->bind(query, fetch, predicate);
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




    void shouldBindTagQueries()
    {
        // GIVEN
        AkonadiFakeData data;

        // Three plain tags, one not matching the predicate
        data.createTag(GenTag().withId(42).asPlain().withName(QStringLiteral("42-in")));
        data.createTag(GenTag().withId(43).asPlain().withName(QStringLiteral("43-in")));
        data.createTag(GenTag().withId(44).asPlain().withName(QStringLiteral("44-ex")));

        // Couple of context tags which should not appear or create trouble
        data.createTag(GenTag().withId(40).asContext().withName(QStringLiteral("40")));
        data.createTag(GenTag().withId(41).asContext().withName(QStringLiteral("41-in")));

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);

        auto query = Domain::LiveQueryOutput<Domain::Tag::Ptr>::Ptr();
        auto fetch = [storage] (const Domain::LiveQueryInput<Akonadi::Tag>::AddFunction &add) {
            auto job = storage->fetchTags();
            Utils::JobHandler::install(job->kjob(), [add, job] {
                foreach (const auto &tag, job->tags()) {
                    add(tag);
                }
            });
        };
        auto predicate = [] (const Akonadi::Tag &tag) {
            return tag.name().endsWith(QLatin1String("-in"));
        };

        // Initial listing
        // WHEN
        integrator->bind(query, fetch, predicate);
        auto result = query->result();
        result->data();
        integrator->bind(query, fetch, predicate);
        result = query->result(); // Should not cause any problem or wrong data

        // THEN
        QVERIFY(result->data().isEmpty());
        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));

        // Reacts to add
        // WHEN
        data.createTag(GenTag().withId(45).asPlain().withName(QStringLiteral("45-in")));

        // THEN
        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));
        QCOMPARE(result->data().at(2)->name(), QStringLiteral("45-in"));

        // Reacts to remove
        // WHEN
        data.removeTag(Akonadi::Tag(45));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));

        // Reacts to change
        // WHEN
        data.modifyTag(GenTag(data.tag(42)).withName(QStringLiteral("42-bis-in")));

        // THEN
        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-bis-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));

        // Reacts to change (which adds)
        // WHEN
        data.modifyTag(GenTag(data.tag(44)).withName(QStringLiteral("44-in")));

        // THEN
        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0)->name(), QStringLiteral("42-bis-in"));
        QCOMPARE(result->data().at(1)->name(), QStringLiteral("43-in"));
        QCOMPARE(result->data().at(2)->name(), QStringLiteral("44-in"));

        // Reacts to change (which removes)
        // WHEN
        data.modifyTag(GenTag(data.tag(44)).withName(QStringLiteral("44-ex")));

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
            integrator->bind(query, fetch, predicate);
            auto result = query->result();

            // THEN * 2
            QVERIFY(result->data().isEmpty());
            TestHelpers::waitForEmptyJobQueue();
            QVERIFY(!result->data().isEmpty());
        }
    }

    void shouldMoveTagBetweenQueries()
    {
        // GIVEN
        AkonadiFakeData data;

        // One plain tag which shows in one query not the other
        data.createTag(GenTag().withId(42).asPlain().withName(QStringLiteral("42-in")));

        // Couple of context tags which should not appear or create trouble
        data.createTag(GenTag().withId(39).asContext().withName(QStringLiteral("39")));
        data.createTag(GenTag().withId(40).asContext().withName(QStringLiteral("40-ex")));
        data.createTag(GenTag().withId(41).asContext().withName(QStringLiteral("41-in")));

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);

        auto inQuery = Domain::LiveQueryOutput<Domain::Tag::Ptr>::Ptr();
        auto exQuery = Domain::LiveQueryOutput<Domain::Tag::Ptr>::Ptr();
        auto fetch = [storage] (const Domain::LiveQueryInput<Akonadi::Tag>::AddFunction &add) {
            auto job = storage->fetchTags();
            Utils::JobHandler::install(job->kjob(), [add, job] {
                foreach (const auto &tag, job->tags()) {
                    add(tag);
                }
            });
        };
        auto inPredicate = [] (const Akonadi::Tag &tag) {
            return tag.name().endsWith(QLatin1String("-in"));
        };
        auto exPredicate = [] (const Akonadi::Tag &tag) {
            return tag.name().endsWith(QLatin1String("-ex"));
        };

        integrator->bind(inQuery, fetch, inPredicate);
        auto inResult = inQuery->result();

        integrator->bind(exQuery, fetch, exPredicate);
        auto exResult = exQuery->result();

        TestHelpers::waitForEmptyJobQueue();

        QCOMPARE(inResult->data().size(), 1);
        QCOMPARE(exResult->data().size(), 0);

        // WHEN
        data.modifyTag(GenTag(data.tag(42)).withName(QStringLiteral("42-ex")));

        // THEN
        QCOMPARE(inResult->data().size(), 0);
        QCOMPARE(exResult->data().size(), 1);
    }




    void shouldBindTaskQueries()
    {
        // GIVEN
        AkonadiFakeData data;

        // One top level collection
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")));

        // Three tasks in the collection, one not matching the predicate
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42-in")));
        data.createItem(GenTodo().withId(43).withParent(42).withTitle(QStringLiteral("43-in")));
        data.createItem(GenTodo().withId(44).withParent(42).withTitle(QStringLiteral("44-ex")));

        // Couple of notes and projects in the collection which should not appear or create trouble
        data.createItem(GenTodo().withId(38).withParent(42).asProject().withTitle(QStringLiteral("38")));
        data.createItem(GenTodo().withId(39).withParent(42).asProject().withTitle(QStringLiteral("39-in")));
        data.createItem(GenNote().withId(40).withParent(42).withTitle(QStringLiteral("40")));
        data.createItem(GenNote().withId(41).withParent(42).withTitle(QStringLiteral("41-in")));

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);

        auto query = Domain::LiveQueryOutput<Domain::Task::Ptr>::Ptr();
        auto fetch = [storage] (const Domain::LiveQueryInput<Akonadi::Item>::AddFunction &add) {
            auto job = storage->fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive, Akonadi::Storage::AllContent);
            Utils::JobHandler::install(job->kjob(), [add, job, storage] {
                foreach (const auto &col, job->collections()) {
                    auto itemJob = storage->fetchItems(col);
                    Utils::JobHandler::install(itemJob->kjob(), [add, itemJob] {
                        foreach (const auto &item, itemJob->items())
                            add(item);
                    });
                }
            });
        };
        auto predicate = [] (const Akonadi::Item &item) {
            return titleFromItem(item).endsWith(QLatin1String("-in"));
        };

        // Initial listing
        // WHEN
        integrator->bind(query, fetch, predicate);
        auto result = query->result();
        result->data();
        integrator->bind(query, fetch, predicate);
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
            integrator->bind(query, fetch, predicate);
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
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")));

        // One task which shows in one query and not the other
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42-in")));

        // Couple of notes in the collection which should not appear or create trouble
        data.createItem(GenNote().withId(39).withParent(42).withTitle(QStringLiteral("39")));
        data.createItem(GenNote().withId(40).withParent(42).withTitle(QStringLiteral("40-ex")));
        data.createItem(GenNote().withId(41).withParent(42).withTitle(QStringLiteral("41-in")));

        auto integrator = createIntegrator(data);
        auto storage = createStorage(data);

        auto inQuery = Domain::LiveQueryOutput<Domain::Task::Ptr>::Ptr();
        auto exQuery = Domain::LiveQueryOutput<Domain::Task::Ptr>::Ptr();
        auto fetch = [storage] (const Domain::LiveQueryInput<Akonadi::Item>::AddFunction &add) {
            auto job = storage->fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive, Akonadi::Storage::AllContent);
            Utils::JobHandler::install(job->kjob(), [add, job, storage] {
                foreach (const auto &col, job->collections()) {
                    auto itemJob = storage->fetchItems(col);
                    Utils::JobHandler::install(itemJob->kjob(), [add, itemJob] {
                        foreach (const auto &item, itemJob->items())
                            add(item);
                    });
                }
            });
        };
        auto inPredicate = [] (const Akonadi::Item &item) {
            return titleFromItem(item).endsWith(QLatin1String("-in"));
        };
        auto exPredicate = [] (const Akonadi::Item &item) {
            return titleFromItem(item).endsWith(QLatin1String("-ex"));
        };

        integrator->bind(inQuery, fetch, inPredicate);
        auto inResult = inQuery->result();

        integrator->bind(exQuery, fetch, exPredicate);
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
        auto fetch = [storage, serializer] (const Domain::LiveQueryInput<Akonadi::Item>::AddFunction &add) {
            auto job = storage->fetchCollections(Akonadi::Collection::root(), Akonadi::Storage::Recursive, Akonadi::Storage::AllContent);
            Utils::JobHandler::install(job->kjob(), [add, job, storage, serializer] {
                foreach (const auto &col, job->collections()) {
                    if (!serializer->isSelectedCollection(col))
                        continue;

                    auto itemJob = storage->fetchItems(col);
                    Utils::JobHandler::install(itemJob->kjob(), [add, itemJob] {
                        foreach (const auto &item, itemJob->items())
                            add(item);
                    });
                }
            });
        };
        auto predicate = [] (const Akonadi::Item &) {
            return true;
        };

        integrator->bind(query, fetch, predicate);
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

    void shouldCallTagRemoveHandlers()
    {
        // GIVEN
        AkonadiFakeData data;

        // One tag
        data.createTag(GenTag().withId(42).withName(QStringLiteral("42")));

        auto integrator = createIntegrator(data);
        qint64 removedId = -1;
        integrator->addRemoveHandler([&removedId] (const Akonadi::Tag &tag) {
            removedId = tag.id();
        });

        // WHEN
        data.removeTag(Akonadi::Tag(42));

        // THEN
        QCOMPARE(removedId, qint64(42));
    }
};

ZANSHIN_TEST_MAIN(AkonadiLiveQueryIntegratorTest)

#include "akonadilivequeryintegratortest.moc"
