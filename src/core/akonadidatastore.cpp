/* This file is part of Zanshin Todo.

   Copyright 2013 Kevin Ottens <ervin@kde.org>
   Copyright 2013 Mario Bensi <nef@ipsquad.net>

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

#include "akonadidatastore.h"
#include "akonadicollectionitem.h"
#include "akonadiincidenceitem.h"
#include "akonadinoteitem.h"
#include "virtualitem.h"
#include "todohelpers.h"
#include "kdescendantsproxymodel.h"
#include "pimitemmodel.h"
#include "akonadi/akonadistoragesettings.h"

#include <KDE/Akonadi/ChangeRecorder>
#include <KDE/Akonadi/Session>
#include <KDE/Akonadi/CollectionFetchJob>
#include <KDE/Akonadi/CollectionFetchScope>
#include <KDE/Akonadi/EntityMimeTypeFilterModel>
#include <KDE/Akonadi/ItemFetchJob>
#include <KDE/Akonadi/ItemFetchScope>
#include <KDE/Akonadi/EntityDisplayAttribute>

#include <KCalCore/Todo>

template<class T>
typename T::Ptr unwrap(const Akonadi::Item &item)
{
    Q_ASSERT(item.hasPayload<typename T::Ptr>());
    return item.payload< typename T::Ptr>();
}

AkonadiDataStore &AkonadiDataStore::instance()
{
    AkonadiDataStore *store = dynamic_cast<AkonadiDataStore*>(&DataStoreInterface::instance());
    Q_ASSERT(store != 0);
    return *store;
}

AkonadiDataStore::AkonadiDataStore()
    : QObject(),
      m_todoBaseModel(0),
      m_todoCollectionModel(0),
      m_noteBaseModel(0),
      m_noteCollectionModel(0)
{
}

AkonadiDataStore::~AkonadiDataStore()
{
}

QAbstractItemModel *AkonadiDataStore::todoBaseModel()
{
    if (!m_todoBaseModel) {
        Akonadi::Session *session = new Akonadi::Session("zanshin", this);

        Akonadi::ItemFetchScope itemScope;
        itemScope.fetchFullPayload();
        itemScope.setAncestorRetrieval(Akonadi::ItemFetchScope::All);

        Akonadi::CollectionFetchScope collectionScope;
        collectionScope.setAncestorRetrieval(Akonadi::CollectionFetchScope::All);

        Akonadi::ChangeRecorder *itemMonitor = new Akonadi::ChangeRecorder(this);
        itemMonitor->setMimeTypeMonitored(PimItem::mimeType(PimItem::Todo));
        itemMonitor->setMimeTypeMonitored(PimItem::mimeType(PimItem::Note));
        itemMonitor->setCollectionFetchScope(collectionScope);
        itemMonitor->setItemFetchScope(itemScope);
        itemMonitor->setSession(session);

        PimItemModel *pimModel = new PimItemModel(itemMonitor, this);
        CollectionFilter *collectionFilter = new CollectionFilter(this);
        collectionFilter->setActiveCollections(Akonadi::StorageSettings::instance().activeCollections());
        connect(&Akonadi::StorageSettings::instance(), SIGNAL(activeCollectionsChanged(Akonadi::Collection::List)), collectionFilter, SLOT(setActiveCollections(Akonadi::Collection::List)));
        collectionFilter->setSourceModel(pimModel);
        m_todoBaseModel = collectionFilter;
    }

    return m_todoBaseModel;
}

QAbstractItemModel *AkonadiDataStore::todoCollectionModel()
{
    if (!m_todoCollectionModel) {
        Akonadi::EntityMimeTypeFilterModel *collectionsModel = new Akonadi::EntityMimeTypeFilterModel(this);
        collectionsModel->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );
        collectionsModel->setSourceModel(todoBaseModel());
        m_todoCollectionModel = collectionsModel;
    }

    return m_todoCollectionModel;
}

QAbstractItemModel *AkonadiDataStore::noteBaseModel()
{
    if (!m_noteBaseModel) {
        Akonadi::ItemFetchScope scope;
        scope.fetchFullPayload( true ); // Need to have full item when adding it to the internal data structure
        scope.fetchAttribute<Akonadi::EntityDisplayAttribute>(true);

        Akonadi::Session *session = new Akonadi::Session("zanshin", this);

        Akonadi::ChangeRecorder *monitor = new Akonadi::ChangeRecorder(this);
        monitor->fetchCollection( true );
        monitor->setItemFetchScope( scope );
        monitor->setCollectionMonitored(Akonadi::Collection::root());
        monitor->setSession(session);
        monitor->setMimeTypeMonitored(PimItem::mimeType(PimItem::Note), true);

        PimItemModel *notetakerModel = new PimItemModel(monitor, this);
        notetakerModel->setSupportedDragActions(Qt::MoveAction);
        //notetakerModel->setCollectionFetchStrategy(EntityTreeModel::InvisibleCollectionFetch); //List of Items, collections are hidden

        CollectionFilter *collectionFilter = new CollectionFilter(this);
        collectionFilter->setActiveCollections(Akonadi::StorageSettings::instance().activeCollections());
        connect(&Akonadi::StorageSettings::instance(), SIGNAL(activeCollectionsChanged(Akonadi::Collection::List)), collectionFilter, SLOT(setActiveCollections(Akonadi::Collection::List)));
        collectionFilter->setSourceModel(notetakerModel);

        //FIXME because invisible collectionfetch is broken we use this instead
        KDescendantsProxyModel *desc = new KDescendantsProxyModel(this);
        desc->setSourceModel(collectionFilter);
        Akonadi::EntityMimeTypeFilterModel *collectionsModel = new Akonadi::EntityMimeTypeFilterModel(this);
        collectionsModel->addMimeTypeExclusionFilter( Akonadi::Collection::mimeType() );
        collectionsModel->setSourceModel(desc);
        m_noteBaseModel = collectionsModel;
    }

    return m_noteBaseModel;
}

QAbstractItemModel *AkonadiDataStore::noteCollectionModel()
{
    if (!m_noteCollectionModel) {
        Akonadi::Session *session = new Akonadi::Session("zanshin", this);
        Akonadi::ChangeRecorder *collectionsMonitor = new Akonadi::ChangeRecorder( this );
        collectionsMonitor->fetchCollection( true );
        collectionsMonitor->setCollectionMonitored(Akonadi::Collection::root());
        collectionsMonitor->setSession(session);
        collectionsMonitor->setMimeTypeMonitored(PimItem::mimeType(PimItem::Note), true);

        Akonadi::EntityTreeModel *model = new Akonadi::EntityTreeModel(collectionsMonitor, this);

        Akonadi::EntityMimeTypeFilterModel *collectionsModel = new Akonadi::EntityMimeTypeFilterModel(this);
        collectionsModel->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );
        collectionsModel->setSourceModel(model);
        m_noteCollectionModel = collectionsModel;
    }

    return m_noteCollectionModel;
}

bool AkonadiDataStore::isProject(const Akonadi::Item &item) const
{
    const KCalCore::Incidence::Ptr i = unwrap<KCalCore::Incidence>(item);
    if (i->comments().contains("X-Zanshin-Project")
     || !i->customProperty("Zanshin", "Project").isEmpty()) {
        return true;
    }
    return PimItemServices::projectInstance().hasChildren(i->uid());
}

PimItem::Ptr AkonadiDataStore::indexFromUrl(const KUrl &url) const
{
    const Akonadi::Item urlItem = Akonadi::Item::fromUrl(url);
    Q_ASSERT(urlItem.isValid());

    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(urlItem);
    job->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
    job->fetchScope().fetchFullPayload();
    if ( !job->exec() ) {
        return PimItem::Ptr();
    }

    Q_ASSERT(job->items().size()==1);
    const Akonadi::Item resolvedItem = job->items().first();
    Q_ASSERT(resolvedItem.isValid());

    if (AkonadiBaseItem::typeFromItem(resolvedItem) == PimItem::Todo) {
        return PimItem::Ptr(new AkonadiIncidenceItem(resolvedItem));
    } else {
        return PimItem::Ptr(new AkonadiNoteItem(resolvedItem));
    }
}

bool AkonadiDataStore::moveTodoToProject(const PimItem::Ptr &item, const PimItem::Ptr &parent)
{
    Zanshin::ItemType parentItemType = Zanshin::StandardTodo;
    Akonadi::Collection collection;
    switch (parent->itemType()) {
    case PimItem::Inbox:
        parentItemType = Zanshin::Inbox;
        collection = item.dynamicCast<AkonadiBaseItem>()->getItem().parentCollection();
        break;
    case PimItem::Collection:
        parentItemType = Zanshin::Collection;
        collection = parent.dynamicCast<AkonadiCollectionItem>()->collection();
        break;
    case PimItem::Project:
        parentItemType = Zanshin::ProjectTodo;
    case PimItem::Todo: // Fall through
        collection = parent.dynamicCast<AkonadiBaseItem>()->getItem().parentCollection();
        break;
    default:
        qFatal("Unsupported parent type");
        break;
    }

    if (!TodoHelpers::moveTodoToProject(item.dynamicCast<AkonadiBaseItem>()->getItem(),
                                        parent->uid(), parentItemType, collection)) {
        return false;
    }

    return true;
}



CollectionFilter::CollectionFilter(QObject *parent)
: QSortFilterProxyModel(parent)
{
}

bool CollectionFilter::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    const QModelIndex sourceChild = sourceModel()->index(sourceRow, 0, sourceParent);
    const QVariant var = sourceChild.data(Akonadi::EntityTreeModel::CollectionRole);
    if (!var.isValid()) {
        return true;
    }
    const Akonadi::Collection col = var.value<Akonadi::Collection>();
    if (col.id() < 0 || mActiveCollections.contains(col)) {
        return true;
    }
    return false;
}

void CollectionFilter::setActiveCollections(const Akonadi::Collection::List &list)
{
    mActiveCollections = list;
    invalidateFilter();
}
