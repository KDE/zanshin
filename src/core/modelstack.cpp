/* This file is part of Zanshin Todo.

   Copyright 2008-2010 Kevin Ottens <ervin@kde.org>
   Copyright 2008, 2009 Mario Bensi <nef@ipsquad.net>

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

#include "modelstack.h"

#include <KDE/Akonadi/ChangeRecorder>
#include <KDE/Akonadi/Session>
#include <KDE/Akonadi/CollectionFetchScope>
#include <KDE/Akonadi/EntityMimeTypeFilterModel>
#include <KDE/Akonadi/ItemFetchScope>

#include "gui/itemeditor/combomodel.h"
#include "gui/sidebar/sidebarmodel.h"
#include "gui/shared/selectionproxymodel.h"
#include "todometadatamodel.h"
#include "pimitem.h"

#include "kdescendantsproxymodel.h"
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/CollectionFetchJob>
#include "pimitemmodel.h"
#include "reparentingmodel/reparentingmodel.h"
#include "core/projectstrategy.h"
#include "core/structurecachestrategy.h"
#include "core/settings.h"
#include <qitemselectionmodel.h>

CollectionFilter::CollectionFilter(QObject *parent)
: QSortFilterProxyModel(parent)
{
}

bool CollectionFilter::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    const QModelIndex sourceChild = sourceModel()->index(sourceRow, 0, sourceParent);
    const QVariant var = sourceChild.data(Akonadi::EntityTreeModel::CollectionIdRole);
    if (!var.isValid()) {
        return true;
    }
    const Akonadi::Collection::Id id = var.value<Akonadi::Collection::Id>();
    if (id < 0 || mActiveCollections.contains(id)) {
        return true;
    }
    return false;
}

void CollectionFilter::setActiveCollections(const QSet<Akonadi::Collection::Id> &set)
{
    mActiveCollections = set;
    invalidateFilter();
}

ModelStack::ModelStack(QObject *parent)
    : QObject(parent),
      m_itemMonitor(0),
      m_entityModel(0),
      m_baseModel(0),
      m_collectionsModel(0),
      m_treeModel(0),
      m_treeSideBarModel(0),
      m_treeSelectionModel(0),
      m_treeComboModel(0),
      m_treeSelection(0),
      m_knowledgeBaseModel(0),
      m_knowledgeSelectionModel(0),
      m_topicsTreeModel(0),
      m_knowledgeSidebarModel(0),
      m_knowledgeCollectionsModel(0),
      m_topicSelection(0),
      m_contextsModel(0),
      m_contextsSideBarModel(0),
      m_contextsSelectionModel(0),
      m_contextsComboModel(0),
      m_contextSelection(0)
{
}

void ModelStack::setOverridePimModel(QAbstractItemModel *model)
{
    m_entityModel = model;
}

QAbstractItemModel *ModelStack::pimitemModel()
{
    if (!m_entityModel) {
        Akonadi::Session *session = new Akonadi::Session("zanshin", this);

        Akonadi::ItemFetchScope itemScope;
        itemScope.fetchFullPayload();
        itemScope.setAncestorRetrieval(Akonadi::ItemFetchScope::All);

        Akonadi::CollectionFetchScope collectionScope;
        collectionScope.setAncestorRetrieval(Akonadi::CollectionFetchScope::All);

        m_itemMonitor = new Akonadi::ChangeRecorder(this);
        m_itemMonitor->setMimeTypeMonitored(PimItem::mimeType(PimItem::Todo));
        m_itemMonitor->setMimeTypeMonitored(PimItem::mimeType(PimItem::Note));
        m_itemMonitor->setCollectionFetchScope(collectionScope);
        m_itemMonitor->setItemFetchScope(itemScope);
        m_itemMonitor->setSession(session);

        PimItemModel *pimModel = new PimItemModel(m_itemMonitor, this);
        CollectionFilter *collectionFilter = new CollectionFilter(this);
        collectionFilter->setActiveCollections(Settings::instance().activeCollections());
        connect(&Settings::instance(), SIGNAL(activeCollectionsChanged(QSet<Akonadi::Collection::Id>)), collectionFilter, SLOT(setActiveCollections(QSet<Akonadi::Collection::Id>)));
        collectionFilter->setSourceModel(pimModel);
        m_entityModel = collectionFilter;
    }
    return m_entityModel;
}

QAbstractItemModel *ModelStack::baseModel()
{
    if (!m_baseModel) {
        TodoMetadataModel *metadataModel = new TodoMetadataModel(this);
        metadataModel->setSourceModel(pimitemModel());
        m_baseModel = metadataModel;
    }
    return m_baseModel;
}

QAbstractItemModel *ModelStack::collectionsModel()
{
    if (!m_collectionsModel) {
        Akonadi::EntityMimeTypeFilterModel *collectionsModel = new Akonadi::EntityMimeTypeFilterModel(this);
        collectionsModel->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );
        collectionsModel->setSourceModel(m_entityModel);
        m_collectionsModel = collectionsModel;
    }
    return m_collectionsModel;
}

QAbstractItemModel *ModelStack::treeModel()
{
    if (!m_treeModel) {
        ReparentingModel *treeModel = new ReparentingModel(new ProjectStrategy());
        treeModel->setSourceModel(baseModel());
        m_treeModel = treeModel;
    }
    return m_treeModel;
}

QAbstractItemModel *ModelStack::treeSideBarModel()
{
    if (!m_treeSideBarModel) {
        SideBarModel *treeSideBarModel = new SideBarModel(this);
        treeSideBarModel->setSourceModel(treeModel());
        m_treeSideBarModel = treeSideBarModel;
    }
    return m_treeSideBarModel;
}

QItemSelectionModel *ModelStack::treeSelection()
{
    if (!m_treeSelection) {
        m_treeSelection = new QItemSelectionModel(treeSideBarModel());
    }
    return m_treeSelection;
}

QAbstractItemModel *ModelStack::treeSelectionModel()
{
    if (!m_treeSelectionModel) {
        SelectionProxyModel *treeSelectionModel = new SelectionProxyModel(this);
        treeSelectionModel->setSelectionModel(treeSelection());
        treeSelectionModel->setSourceModel(treeModel());
        m_treeSelectionModel = treeSelectionModel;
    }
    return m_treeSelectionModel;
}

QAbstractItemModel *ModelStack::treeComboModel()
{
    if (!m_treeComboModel) {
        ComboModel *treeComboModel = new ComboModel(this);

        KDescendantsProxyModel *descendantProxyModel = new KDescendantsProxyModel(treeComboModel);
        descendantProxyModel->setSourceModel(treeSideBarModel());
        descendantProxyModel->setDisplayAncestorData(true);

        treeComboModel->setSourceModel(descendantProxyModel);
        m_treeComboModel = treeComboModel;
    }
    return m_treeComboModel;
}

QAbstractItemModel *ModelStack::contextsModel()
{
    if (!m_contextsModel) {
        ReparentingModel *contextsModel = new ReparentingModel(new StructureCacheStrategy(PimItemRelation::Context), this);
        contextsModel->setSourceModel(baseModel());
        m_contextsModel = contextsModel;
    }
    return m_contextsModel;
}

QAbstractItemModel *ModelStack::contextsSideBarModel()
{
    if (!m_contextsSideBarModel) {
        SideBarModel *contextsSideBarModel = new SideBarModel(this);
        contextsSideBarModel->setSourceModel(contextsModel());
        m_contextsSideBarModel = contextsSideBarModel;
    }
    return m_contextsSideBarModel;
}

QItemSelectionModel *ModelStack::contextsSelection()
{
    if (!m_contextSelection) {
        m_contextSelection = new QItemSelectionModel(contextsSideBarModel());
    }
    return m_contextSelection;
}

QAbstractItemModel *ModelStack::contextsSelectionModel()
{
    if (!m_contextsSelectionModel) {
        SelectionProxyModel *contextsSelectionModel = new SelectionProxyModel(this);
        contextsSelectionModel->setSelectionModel(contextsSelection());
        contextsSelectionModel->setSourceModel(contextsModel());
        m_contextsSelectionModel = contextsSelectionModel;
    }
    return m_contextsSelectionModel;
}

QAbstractItemModel *ModelStack::contextsComboModel()
{
    if (!m_contextsComboModel) {
        ComboModel *contextsComboModel = new ComboModel(this);

        KDescendantsProxyModel *descendantProxyModel = new KDescendantsProxyModel(contextsComboModel);
        descendantProxyModel->setSourceModel(contextsSideBarModel());
        descendantProxyModel->setDisplayAncestorData(true);

        contextsComboModel->setSourceModel(descendantProxyModel);
        m_contextsComboModel = contextsComboModel;
    }
    return m_contextsComboModel;
}

QAbstractItemModel* ModelStack::knowledgeBaseModel()
{
    if (m_knowledgeBaseModel) {
        return m_knowledgeBaseModel;
    }
    
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
    collectionFilter->setActiveCollections(Settings::instance().activeCollections());
    connect(&Settings::instance(), SIGNAL(activeCollectionsChanged(QSet<Akonadi::Collection::Id>)), collectionFilter, SLOT(setActiveCollections(QSet<Akonadi::Collection::Id>)));
    collectionFilter->setSourceModel(notetakerModel);

    //FIXME because invisible collectionfetch is broken we use this instead
    KDescendantsProxyModel *desc = new KDescendantsProxyModel(this);
    desc->setSourceModel(collectionFilter);
    Akonadi::EntityMimeTypeFilterModel *collectionsModel = new Akonadi::EntityMimeTypeFilterModel(this);
    collectionsModel->addMimeTypeExclusionFilter( Akonadi::Collection::mimeType() );
    collectionsModel->setSourceModel(desc);
    m_knowledgeBaseModel = collectionsModel;

    return m_knowledgeBaseModel;
}

QAbstractItemModel *ModelStack::topicsTreeModel()
{
    if (!m_topicsTreeModel) {
        ReparentingModel *treeModel = new ReparentingModel(new StructureCacheStrategy(PimItemRelation::Topic), this);
        treeModel->setSourceModel(knowledgeBaseModel());
        m_topicsTreeModel = treeModel;
    }
    return m_topicsTreeModel;
}

QAbstractItemModel *ModelStack::knowledgeSideBarModel()
{
    if (!m_knowledgeSidebarModel) {
        SideBarModel *sideBarModel = new SideBarModel(this);
        sideBarModel->setSourceModel(topicsTreeModel());
        m_knowledgeSidebarModel = sideBarModel;
    }
    return m_knowledgeSidebarModel;
}

QItemSelectionModel* ModelStack::knowledgeSelection()
{
    if (!m_topicSelection) {
        m_topicSelection = new QItemSelectionModel(knowledgeSideBarModel());
    }
    return m_topicSelection;
}


QAbstractItemModel* ModelStack::knowledgeSelectionModel()
{
    if (!m_knowledgeSelectionModel) {
        SelectionProxyModel *contextsSelectionModel = new SelectionProxyModel(this);
        contextsSelectionModel->setSelectionModel(knowledgeSelection());
        contextsSelectionModel->setSourceModel(topicsTreeModel());
        m_knowledgeSelectionModel = contextsSelectionModel;
    }
    return m_knowledgeSelectionModel;
}

QAbstractItemModel *ModelStack::knowledgeCollectionsModel()
{
    if (!m_knowledgeCollectionsModel) {
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
        m_knowledgeCollectionsModel = collectionsModel;
    }
    return m_knowledgeCollectionsModel;
}
