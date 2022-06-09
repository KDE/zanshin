/*
 * SPDX-FileCopyrightText: 2015 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include "akonadifakestorage.h"

#include <QTimer>

#include "akonadi/akonadistoragesettings.h"
#include "akonadifakedata.h"
#include "akonadifakejobs.h"

#include "utils/jobhandler.h"

using namespace Testlib;

class AkonadiFakeTransaction : public FakeJob
{
    Q_OBJECT
public:
    explicit AkonadiFakeTransaction(QObject *parent = nullptr)
        : FakeJob(parent),
          m_nextIdx(0)
    {
    }

private slots:
    void onTimeout() override
    {
        auto jobs = childJobs();
        if (m_nextIdx == 0) {
            const auto it = std::find_if(jobs.constBegin(), jobs.constEnd(),
                                         [] (FakeJob *job) { return job->expectedError() != 0; });
            if (it != jobs.constEnd()) {
                setError((*it)->expectedError());
                setErrorText((*it)->expectedErrorText());
                emitResult();
                return;
            }
        }

        if (m_nextIdx >= jobs.size()) {
            emitResult();
            return;
        }

        auto job = jobs[m_nextIdx];
        connect(job, &KJob::result, this, &AkonadiFakeTransaction::onTimeout);
        job->start();
        m_nextIdx++;
    }

private:
    QList<FakeJob*> childJobs() const
    {
        QList<FakeJob*> jobs = findChildren<FakeJob*>();
        jobs.erase(std::remove_if(jobs.begin(), jobs.end(),
                                  [this] (FakeJob *job) {
                                      return job->parent() != this;
                                 }),
                   jobs.end());
        return jobs;
    }

    int m_nextIdx;
};

Utils::JobHandler::StartMode startModeForParent(QObject *parent)
{
    bool isTransaction = qobject_cast<AkonadiFakeTransaction*>(parent);
    return isTransaction ? Utils::JobHandler::ManualStart
                         : Utils::JobHandler::AutoStart;
}

void noop() {}

AkonadiFakeStorage::AkonadiFakeStorage(AkonadiFakeData *data)
    : m_data(data)
{
}

Akonadi::Collection AkonadiFakeStorage::defaultCollection()
{
    return Akonadi::StorageSettings::instance().defaultCollection();
}

KJob *AkonadiFakeStorage::createItem(Akonadi::Item item, Akonadi::Collection collection)
{
    Q_ASSERT(!item.isValid());

    auto job = new FakeJob;
    if (!m_data->item(item.id()).isValid()) {
        job->setExpectedError(m_data->storageBehavior().createNextItemErrorCode(),
                              m_data->storageBehavior().createNextItemErrorText());
        Utils::JobHandler::install(job, [=] () mutable {
            if (!job->error()) {
                item.setId(m_data->maxItemId() + 1);
                item.setParentCollection(collection);
                // Force payload detach
                item.setPayloadFromData(item.payloadData());
                m_data->createItem(item);
            }
        });
    } else {
        job->setExpectedError(1, QStringLiteral("Item already exists"));
        Utils::JobHandler::install(job, noop);
    }
    return job;
}

KJob *AkonadiFakeStorage::updateItem(Akonadi::Item item, QObject *parent)
{
    auto job = new FakeJob(parent);
    auto startMode = startModeForParent(parent);

    if (m_data->item(item.id()).isValid()) {
        job->setExpectedError(m_data->storageBehavior().updateNextItemErrorCode(),
                              m_data->storageBehavior().updateNextItemErrorText());
        Utils::JobHandler::install(job, [=] () mutable {
            if (!job->error()) {
                // Force payload detach
                item.setPayloadFromData(item.payloadData());
                m_data->modifyItem(item);
            }
        }, startMode);
    } else {
        job->setExpectedError(1, QStringLiteral("Item doesn't exist"));
        Utils::JobHandler::install(job, noop, startMode);
    }
    return job;
}

KJob *AkonadiFakeStorage::removeItem(Akonadi::Item item, QObject *parent)
{
    auto job = new FakeJob(parent);
    if (m_data->item(item.id()).isValid()) {
        Utils::JobHandler::install(job, [=] {
            if (!job->error()) {
                m_data->removeItem(item);
            }
        });
    } else {
        job->setExpectedError(1, QStringLiteral("Item doesn't exist"));
        Utils::JobHandler::install(job, noop);
    }
    return job;
}

KJob *AkonadiFakeStorage::removeItems(Akonadi::Item::List items, QObject *parent)
{
    auto job = new FakeJob;
    auto startMode = startModeForParent(parent);
    bool allItemsExist = std::all_of(items.constBegin(), items.constEnd(),
                                     [=] (const Akonadi::Item &item) {
                                         return m_data->item(item.id()).isValid();
                                     });

    if (allItemsExist) {
        job->setExpectedError(m_data->storageBehavior().deleteNextItemErrorCode(),
                              m_data->storageBehavior().deleteNextItemErrorText());
        Utils::JobHandler::install(job, [=] {
            if (!job->error()) {
                foreach (const Akonadi::Item &item, items) {
                    m_data->removeItem(item);
                }
            }
        }, startMode);
    } else {
        job->setExpectedError(1, QStringLiteral("At least one item doesn't exist"));
        Utils::JobHandler::install(job, noop, startMode);
    }
    return job;
}

KJob *AkonadiFakeStorage::moveItem(Akonadi::Item item, Akonadi::Collection collection, QObject *parent)
{
    auto job = new FakeJob(parent);
    auto startMode = startModeForParent(parent);
    if (m_data->item(item.id()).isValid()
     && m_data->collection(collection.id()).isValid()) {
        Utils::JobHandler::install(job, [=] () mutable {
            if (!job->error()) {
                item.setParentCollection(collection);
                // Force payload detach
                item.setPayloadFromData(item.payloadData());
                m_data->modifyItem(item);
            }
        }, startMode);
    } else {
        job->setExpectedError(1, QStringLiteral("The item or the collection doesn't exist"));
        Utils::JobHandler::install(job, noop, startMode);
    }
    return job;
}

KJob *AkonadiFakeStorage::moveItems(Akonadi::Item::List items, Akonadi::Collection collection, QObject *parent)
{
    using namespace std::placeholders;

    auto job = new FakeJob(parent);
    auto startMode = startModeForParent(parent);
    bool allItemsExist = std::all_of(items.constBegin(), items.constEnd(),
                                     [=] (const Akonadi::Item &item) {
                                         return m_data->item(item.id()).isValid();
                                     });

    if (allItemsExist
     && m_data->collection(collection.id()).isValid()) {
        Utils::JobHandler::install(job, [=] () mutable {
            if (!job->error()) {
                std::transform(items.constBegin(), items.constEnd(),
                               items.begin(),
                               [=] (const Akonadi::Item &item) {
                    auto result = item;
                    result.setParentCollection(collection);
                    // Force payload detach
                    result.setPayloadFromData(result.payloadData());
                    return result;
                });

                foreach (const Akonadi::Item &item, items) {
                    m_data->modifyItem(item);
                }
            }
        }, startMode);
    } else {
        job->setExpectedError(1, QStringLiteral("One of the items or the collection doesn't exist"));
        Utils::JobHandler::install(job, noop, startMode);
    }
    return job;
}

KJob *AkonadiFakeStorage::createCollection(Akonadi::Collection collection, QObject *parent)
{
    Q_ASSERT(!collection.isValid());

    auto job = new FakeJob(parent);
    auto startMode = startModeForParent(parent);
    if (!m_data->collection(collection.id()).isValid()) {
        Utils::JobHandler::install(job, [=] () mutable {
            if (!job->error()) {
                collection.setId(m_data->maxCollectionId() + 1);
                m_data->createCollection(collection);
            }
        }, startMode);
    } else {
        job->setExpectedError(1, QStringLiteral("The collection already exists"));
        Utils::JobHandler::install(job, noop, startMode);
    }
    return job;
}

KJob *AkonadiFakeStorage::updateCollection(Akonadi::Collection collection, QObject *parent)
{
    auto job = new FakeJob(parent);
    auto startMode = startModeForParent(parent);
    if (m_data->collection(collection.id()).isValid()) {
        Utils::JobHandler::install(job, [=] {
            if (!job->error()) {
                m_data->modifyCollection(collection);
            }
        }, startMode);
    } else {
        job->setExpectedError(1, QStringLiteral("The collection doesn't exist"));
        Utils::JobHandler::install(job, noop, startMode);
    }
    return job;
}

KJob *AkonadiFakeStorage::removeCollection(Akonadi::Collection collection, QObject *parent)
{
    auto job = new FakeJob(parent);
    auto startMode = startModeForParent(parent);
    if (m_data->collection(collection.id()).isValid()) {
        Utils::JobHandler::install(job, [=] {
            if (!job->error()) {
                m_data->removeCollection(collection);
            }
        }, startMode);
    } else {
        job->setExpectedError(1, QStringLiteral("The collection doesn't exist"));
        Utils::JobHandler::install(job, noop, startMode);
    }
    return job;
}

KJob *AkonadiFakeStorage::createTransaction(QObject *parent)
{
    auto job = new AkonadiFakeTransaction(parent);
    Utils::JobHandler::install(job, noop);
    return job;
}

Akonadi::CollectionFetchJobInterface *AkonadiFakeStorage::fetchCollections(Akonadi::Collection collection,
                                                                           Akonadi::StorageInterface::FetchDepth depth,
                                                                           QObject *parent)
{
    auto job = new AkonadiFakeCollectionFetchJob(parent);
    auto children = Akonadi::Collection::List();

    switch (depth) {
    case Base:
        children << m_data->collection(findId(collection));
        break;
    case FirstLevel:
        children << m_data->childCollections(findId(collection));
        break;
    case Recursive:
        children = collectChildren(collection);
        break;
    }

    auto collections = children;

    if (depth != Base) {
        // Replace the dummy parents in the ancestor chain with proper ones
        // full of juicy data
        using namespace std::placeholders;
        auto completeCollection = std::bind(&AkonadiFakeData::reconstructAncestors,
                                            m_data, _1, collection);
        std::transform(collections.begin(), collections.end(),
                       collections.begin(), completeCollection);
    }

    const auto behavior = m_data->storageBehavior().fetchCollectionsBehavior(collection.id());
    if (behavior == AkonadiFakeStorageBehavior::NormalFetch)
        job->setCollections(collections);
    job->setExpectedError(m_data->storageBehavior().fetchCollectionsErrorCode(collection.id()));
    Utils::JobHandler::install(job, noop);
    return job;
}

Akonadi::ItemFetchJobInterface *AkonadiFakeStorage::fetchItems(Akonadi::Collection collection, QObject *parent)
{
    auto items = m_data->childItems(findId(collection));
    std::transform(items.begin(), items.end(),
                   items.begin(),
                   [this] (const Akonadi::Item &item) {
                       auto result = m_data->reconstructItemDependencies(item);
                       // Force payload detach
                       result.setPayloadFromData(result.payloadData());
                       return result;
                   });

    auto job = new AkonadiFakeItemFetchJob(parent);
    const auto behavior = m_data->storageBehavior().fetchItemsBehavior(collection.id());
    if (behavior == AkonadiFakeStorageBehavior::NormalFetch)
        job->setItems(items);
    job->setExpectedError(m_data->storageBehavior().fetchItemsErrorCode(collection.id()));
    Utils::JobHandler::install(job, noop);
    return job;
}

Akonadi::ItemFetchJobInterface *AkonadiFakeStorage::fetchItem(Akonadi::Item item, QObject *parent)
{
    auto fullItem = m_data->item(findId(item));
    fullItem = m_data->reconstructItemDependencies(fullItem);
    // Force payload detach
    fullItem.setPayloadFromData(fullItem.payloadData());

    auto job = new AkonadiFakeItemFetchJob(parent);
    const auto behavior = m_data->storageBehavior().fetchItemBehavior(item.id());
    if (behavior == AkonadiFakeStorageBehavior::NormalFetch)
        job->setItems(Akonadi::Item::List() << fullItem);
    job->setExpectedError(m_data->storageBehavior().fetchItemErrorCode(item.id()));
    Utils::JobHandler::install(job, noop);
    return job;
}

Akonadi::Collection::Id AkonadiFakeStorage::findId(const Akonadi::Collection &collection)
{
    if (collection.isValid() || collection.remoteId().isEmpty())
        return collection.id();

    const auto remoteId = collection.remoteId();
    auto collections = m_data->collections();
    auto result = std::find_if(collections.constBegin(), collections.constEnd(),
                               [remoteId] (const Akonadi::Collection &collection) {
                                   return collection.remoteId() == remoteId;
                               });
    return (result != collections.constEnd()) ? result->id() : collection.id();
}

Akonadi::Item::Id AkonadiFakeStorage::findId(const Akonadi::Item &item)
{
    if (item.isValid() || item.remoteId().isEmpty())
        return item.id();

    const auto remoteId = item.remoteId();
    auto items = m_data->items();
    auto result = std::find_if(items.constBegin(), items.constEnd(),
                               [remoteId] (const Akonadi::Item &item) {
                                   return item.remoteId() == remoteId;
                               });
    return (result != items.constEnd()) ? result->id() : item.id();

}

Akonadi::Collection::List AkonadiFakeStorage::collectChildren(const Akonadi::Collection &root)
{
    auto collections = Akonadi::Collection::List();

    foreach (const auto &child, m_data->childCollections(findId(root))) {
        if (child.enabled())
            collections << m_data->collection(findId(child));
        collections += collectChildren(child);
    }

    return collections;
}

#include "akonadifakestorage.moc"
