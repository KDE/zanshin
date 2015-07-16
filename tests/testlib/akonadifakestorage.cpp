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

#include "akonadifakestorage.h"

#include <Akonadi/Notes/NoteUtils>
#include <KCalCore/Todo>
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
    explicit AkonadiFakeTransaction()
        : FakeJob(),
          m_nextIdx(0)
    {
    }

private slots:
    void onTimeout()
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
        connect(job, SIGNAL(result(KJob*)), this, SLOT(onTimeout()));
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

Akonadi::Collection AkonadiFakeStorage::defaultTaskCollection()
{
    return Akonadi::StorageSettings::instance().defaultTaskCollection();
}

Akonadi::Collection AkonadiFakeStorage::defaultNoteCollection()
{
    return Akonadi::StorageSettings::instance().defaultNoteCollection();
}

KJob *AkonadiFakeStorage::createItem(Akonadi::Item item, Akonadi::Collection collection)
{
    Q_ASSERT(!item.isValid());

    auto job = new FakeJob;
    if (!m_data->item(item.id()).isValid()) {
        Utils::JobHandler::install(job, [=] () mutable {
            item.setId(m_data->maxItemId() + 1);
            item.setParentCollection(collection);
            // Force payload detach
            item.setPayloadFromData(item.payloadData());
            m_data->createItem(item);
        });
    } else {
        job->setExpectedError(1, "Item already exists");
        Utils::JobHandler::install(job, noop);
    }
    return job;
}

KJob *AkonadiFakeStorage::updateItem(Akonadi::Item item, QObject *parent)
{
    auto job = new FakeJob(parent);
    auto startMode = startModeForParent(parent);

    if (m_data->item(item.id()).isValid()) {
        Utils::JobHandler::install(job, [=] () mutable {
            // Force payload detach
            item.setPayloadFromData(item.payloadData());
            m_data->modifyItem(item);
        }, startMode);
    } else {
        job->setExpectedError(1, "Item doesn't exist");
        Utils::JobHandler::install(job, noop, startMode);
    }
    return job;
}

KJob *AkonadiFakeStorage::removeItem(Akonadi::Item item)
{
    auto job = new FakeJob;
    if (m_data->item(item.id()).isValid()) {
        Utils::JobHandler::install(job, [=] {
            m_data->removeItem(item);
        });
    } else {
        job->setExpectedError(1, "Item doesn't exist");
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
        Utils::JobHandler::install(job, [=] {
            foreach (const Akonadi::Item &item, items) {
                m_data->removeItem(item);
            }
        }, startMode);
    } else {
        job->setExpectedError(1, "At least one item doesn't exist");
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
            item.setParentCollection(collection);
            // Force payload detach
            item.setPayloadFromData(item.payloadData());
            m_data->modifyItem(item);
        }, startMode);
    } else {
        job->setExpectedError(1, "The item or the collection doesn't exist");
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
        }, startMode);
    } else {
        job->setExpectedError(1, "One of the items or the collection doesn't exist");
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
            collection.setId(m_data->maxCollectionId() + 1);
            m_data->createCollection(collection);
        }, startMode);
    } else {
        job->setExpectedError(1, "The collection already exists");
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
            m_data->modifyCollection(collection);
        }, startMode);
    } else {
        job->setExpectedError(1, "The collection doesn't exist");
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
            m_data->removeCollection(collection);
        }, startMode);
    } else {
        job->setExpectedError(1, "The collection doesn't exist");
        Utils::JobHandler::install(job, noop, startMode);
    }
    return job;
}

KJob *AkonadiFakeStorage::createTransaction()
{
    auto job = new AkonadiFakeTransaction;
    Utils::JobHandler::install(job, noop);
    return job;
}

KJob *AkonadiFakeStorage::createTag(Akonadi::Tag tag)
{
    Q_ASSERT(!tag.isValid());

    auto job = new FakeJob;
    if (!m_data->tag(tag.id()).isValid()) {
        Utils::JobHandler::install(job, [=] () mutable {
            tag.setId(m_data->maxTagId() + 1);
            m_data->createTag(tag);
        });
    } else {
        job->setExpectedError(1, "The tag already exists");
        Utils::JobHandler::install(job, noop);
    }
    return job;
}

KJob *AkonadiFakeStorage::updateTag(Akonadi::Tag tag)
{
    auto job = new FakeJob;
    if (m_data->tag(tag.id()).isValid()) {
        Utils::JobHandler::install(job, [=] {
            m_data->modifyTag(tag);
        });
    } else {
        job->setExpectedError(1, "The tag doesn't exist");
        Utils::JobHandler::install(job, noop);
    }
    return job;
}

KJob *AkonadiFakeStorage::removeTag(Akonadi::Tag tag)
{
    auto job = new FakeJob;
    if (m_data->tag(tag.id()).isValid()) {
        Utils::JobHandler::install(job, [=] {
            m_data->removeTag(tag);
        });
    } else {
        job->setExpectedError(1, "The tag doesn't exist");
        Utils::JobHandler::install(job, noop);
    }
    return job;
}

Akonadi::CollectionFetchJobInterface *AkonadiFakeStorage::fetchCollections(Akonadi::Collection collection,
                                                                           Akonadi::StorageInterface::FetchDepth depth,
                                                                           Akonadi::StorageInterface::FetchContentTypes types)
{
    auto job = new AkonadiFakeCollectionFetchJob;
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

    auto collections = Akonadi::Collection::List();

    if (types == Akonadi::StorageInterface::AllContent) {
        collections = children;
    } else {
        std::copy_if(children.constBegin(), children.constEnd(),
                     std::back_inserter(collections),
                     [types] (const Akonadi::Collection &col) {
                         const auto mime = col.contentMimeTypes();
                         return ((types & Akonadi::StorageInterface::Tasks) && mime.contains(KCalCore::Todo::todoMimeType()))
                             || ((types & Akonadi::StorageInterface::Notes) && mime.contains(Akonadi::NoteUtils::noteMimeType()));
                     });
    }

    if (depth != Base) {
        // Replace the dummy parents in the ancestor chain with proper ones
        // full of juicy data
        using namespace std::placeholders;
        auto completeCollection = std::bind(&AkonadiFakeData::reconstructAncestors,
                                            m_data, _1, collection);
        std::transform(collections.begin(), collections.end(),
                       collections.begin(), completeCollection);
    }

    job->setCollections(collections);
    Utils::JobHandler::install(job, noop);
    return job;
}

Akonadi::CollectionSearchJobInterface *AkonadiFakeStorage::searchCollections(QString collectionName)
{
    auto job = new AkonadiFakeCollectionSearchJob;
    const auto allCollections = m_data->collections();
    auto foundCollections = Akonadi::Collection::List();

    std::copy_if(allCollections.constBegin(), allCollections.constEnd(),
                 std::back_inserter(foundCollections),
                 [collectionName] (const Akonadi::Collection &col) {
                     const auto mime = col.contentMimeTypes();
                     const bool supportedType = mime.contains(KCalCore::Todo::todoMimeType())
                                             || mime.contains(Akonadi::NoteUtils::noteMimeType());
                     return supportedType && col.displayName().contains(collectionName, Qt::CaseInsensitive);
                 });

    job->setCollections(foundCollections);
    Utils::JobHandler::install(job, noop);
    return job;
}

Akonadi::ItemFetchJobInterface *AkonadiFakeStorage::fetchItems(Akonadi::Collection collection)
{
    auto items = m_data->childItems(findId(collection));
    std::transform(items.begin(), items.end(),
                   items.begin(),
                   [this] (const Akonadi::Item &item) {
                       auto collection = m_data->reconstructAncestors(item.parentCollection());
                       auto result = item;
                       result.setParentCollection(collection);
                       // Force payload detach
                       result.setPayloadFromData(result.payloadData());
                       return result;
                   });

    auto job = new AkonadiFakeItemFetchJob;
    job->setItems(items);
    return job;
}

Akonadi::ItemFetchJobInterface *AkonadiFakeStorage::fetchItem(Akonadi::Item item)
{
    auto fullItem = m_data->item(findId(item));
    fullItem = m_data->reconstructItemDependencies(fullItem);
    // Force payload detach
    fullItem.setPayloadFromData(fullItem.payloadData());

    auto job = new AkonadiFakeItemFetchJob;
    job->setItems(Akonadi::Item::List() << fullItem);
    return job;
}

Akonadi::ItemFetchJobInterface *AkonadiFakeStorage::fetchTagItems(Akonadi::Tag tag)
{
    auto items = m_data->tagItems(findId(tag));
    std::transform(items.begin(), items.end(),
                   items.begin(),
                   [this] (const Akonadi::Item &item) {
                       auto collection = m_data->reconstructAncestors(item.parentCollection());
                       auto result = item;
                       result.setParentCollection(collection);
                       // Force payload detach
                       result.setPayloadFromData(result.payloadData());
                       return result;
                   });

    auto job = new AkonadiFakeItemFetchJob;
    job->setItems(items);
    return job;
}

Akonadi::TagFetchJobInterface *AkonadiFakeStorage::fetchTags()
{
    auto job = new AkonadiFakeTagFetchJob;
    job->setTags(m_data->tags());
    return job;
}

Akonadi::Tag::Id AkonadiFakeStorage::findId(const Akonadi::Tag &tag)
{
    if (tag.isValid() || tag.gid().isEmpty())
        return tag.id();

    const auto gid = tag.gid();
    auto tags = m_data->tags();
    auto result = std::find_if(tags.constBegin(), tags.constEnd(),
                               [gid] (const Akonadi::Tag &tag) {
                                   return tag.gid() == gid;
                               });
    return (result != tags.constEnd()) ? result->id() : tag.id();
}

Akonadi::Entity::Id AkonadiFakeStorage::findId(const Akonadi::Collection &collection)
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

Akonadi::Entity::Id AkonadiFakeStorage::findId(const Akonadi::Item &item)
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
        if (!child.enabled() && !child.referenced())
            continue;

        collections << m_data->collection(findId(child));
        collections += collectChildren(child);
    }

    return collections;
}

#include "akonadifakestorage.moc"
