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

#include "categorymanager.h"
#include "combomodel.h"
#include "sidebarmodel.h"
#include "selectionproxymodel.h"
#include "todocategoriesmodel.h"
#include "todometadatamodel.h"
#include "todotreemodel.h"
#include "abstractpimitem.h"

#include "kdescendantsproxymodel.h"
#include <Akonadi/EntityDisplayAttribute>
#include "parentstructuremodel.h"
#include "pimitemmodel.h"

ModelStack::ModelStack(QObject *parent)
    : QObject(parent),
      m_entityModel(0),
      m_baseModel(0),
      m_collectionsModel(0),
      m_treeModel(0),
      m_treeSideBarModel(0),
      m_treeSelectionModel(0),
      m_treeComboModel(0),
      m_categoriesModel(0),
      m_categoriesSideBarModel(0),
      m_categoriesSelectionModel(0),
      m_categoriesComboModel(0),
      m_treeSelection(0),
      m_categorySelection(0),
      m_knowledgeMonitor(0),
      m_knowledgeBaseModel(0),
      m_knowledgeSelectionModel(0),
      m_knowledgeSidebarModel(0),
      m_topicsTreeModel(0),
      m_topicSelection(0),
      m_knowledgeCollectionsModel(0)
{
}

QAbstractItemModel *ModelStack::baseModel()
{
    if (!m_baseModel) {
        Akonadi::Session *session = new Akonadi::Session("zanshin", this);

        Akonadi::ItemFetchScope itemScope;
        itemScope.fetchFullPayload();
        itemScope.setAncestorRetrieval(Akonadi::ItemFetchScope::All);

        Akonadi::CollectionFetchScope collectionScope;
        collectionScope.setAncestorRetrieval(Akonadi::CollectionFetchScope::All);

        Akonadi::ChangeRecorder *changeRecorder = new Akonadi::ChangeRecorder(this);
        changeRecorder->setCollectionMonitored(Akonadi::Collection::root());
        changeRecorder->setMimeTypeMonitored(AbstractPimItem::mimeType(AbstractPimItem::Todo));
        changeRecorder->setCollectionFetchScope(collectionScope);
        changeRecorder->setItemFetchScope(itemScope);
        changeRecorder->setSession(session);

        m_entityModel = new PimItemModel(changeRecorder, this);
        TodoMetadataModel *metadataModel = new TodoMetadataModel(this);
        metadataModel->setSourceModel(m_entityModel);
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
        TodoTreeModel *treeModel = new TodoTreeModel(this);
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

QAbstractItemModel *ModelStack::treeSelectionModel()
{
    if (!m_treeSelectionModel) {
        SelectionProxyModel *treeSelectionModel = new SelectionProxyModel(this);
        treeSelectionModel->setSelectionModel(m_treeSelection);
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

QAbstractItemModel *ModelStack::categoriesModel()
{
    if (!m_categoriesModel) {
        CategoryManager::instance().setModel(baseModel());
        TodoCategoriesModel *categoriesModel = new TodoCategoriesModel(this);
        categoriesModel->setSourceModel(baseModel());
        m_categoriesModel = categoriesModel;
    }
    return m_categoriesModel;
}

QAbstractItemModel *ModelStack::categoriesSideBarModel()
{
    if (!m_categoriesSideBarModel) {
        SideBarModel *categoriesSideBarModel = new SideBarModel(this);
        categoriesSideBarModel->setSourceModel(categoriesModel());
        m_categoriesSideBarModel = categoriesSideBarModel;
    }
    return m_categoriesSideBarModel;
}

QAbstractItemModel *ModelStack::categoriesSelectionModel()
{
    if (!m_categoriesSelectionModel) {
        SelectionProxyModel *categoriesSelectionModel = new SelectionProxyModel(this);
        categoriesSelectionModel->setSelectionModel(m_categorySelection);
        categoriesSelectionModel->setSourceModel(categoriesModel());
        m_categoriesSelectionModel = categoriesSelectionModel;
    }
    return m_categoriesSelectionModel;
}

QAbstractItemModel *ModelStack::categoriesComboModel()
{
    if (!m_categoriesComboModel) {
        ComboModel *categoriesComboModel = new ComboModel(this);

        KDescendantsProxyModel *descendantProxyModel = new KDescendantsProxyModel(categoriesComboModel);
        descendantProxyModel->setSourceModel(categoriesSideBarModel());
        descendantProxyModel->setDisplayAncestorData(true);

        categoriesComboModel->setSourceModel(descendantProxyModel);
        m_categoriesComboModel = categoriesComboModel;
    }
    return m_categoriesComboModel;
}

void ModelStack::setItemTreeSelectionModel(QItemSelectionModel *selection)
{
    m_treeSelection = selection;
    if (m_treeSelectionModel) {
        static_cast<SelectionProxyModel*>(m_treeSelectionModel)->setSelectionModel(m_treeSelection);
    }
}

void ModelStack::setItemCategorySelectionModel(QItemSelectionModel *selection)
{
    m_categorySelection = selection;
    if (m_categoriesSelectionModel) {
        static_cast<SelectionProxyModel*>(m_categoriesSelectionModel)->setSelectionModel(m_categorySelection);
    }
}

QAbstractItemModel* ModelStack::knowledgeBaseModel()
{
    if (m_knowledgeBaseModel) {
        return m_knowledgeBaseModel;
    }
    
    Akonadi::ItemFetchScope scope;
    scope.fetchFullPayload( true ); // Need to have full item when adding it to the internal data structure
    scope.fetchAttribute<Akonadi::EntityDisplayAttribute>(true);
    //scope.fetchAttribute<Akonadi::EntityDeletedAttribute>(true);
    //scope.setAncestorRetrieval(Akonadi::ItemFetchScope::All);

    //Akonadi::CollectionFetchScope collectionScope;
    //collectionScope.setAncestorRetrieval(Akonadi::CollectionFetchScope::All);

    Akonadi::Session *session = new Akonadi::Session("zanshin", this);

    m_knowledgeMonitor = new Akonadi::ChangeRecorder( this );
    m_knowledgeMonitor->fetchCollection( true );
    m_knowledgeMonitor->setItemFetchScope( scope );
    //m_knowledgeMonitor->setCollectionFetchScope( collectionScope );
    m_knowledgeMonitor->setCollectionMonitored(Akonadi::Collection::root());
    m_knowledgeMonitor->setSession(session);

    //m_knowledgeMonitor->setMimeTypeMonitored(AbstractPimItem::mimeType(AbstractPimItem::Incidence), true);
    m_knowledgeMonitor->setMimeTypeMonitored(AbstractPimItem::mimeType(AbstractPimItem::Note), true);

    PimItemModel *notetakerModel = new PimItemModel ( m_knowledgeMonitor, this );
    notetakerModel->setSupportedDragActions(Qt::MoveAction);
    //notetakerModel->setCollectionFetchStrategy(EntityTreeModel::InvisibleCollectionFetch); //List of Items, collections are hidden
    //m_knowledgeBaseModel = notetakerModel;

    //FIXME because invisible collectionfetch is broken we use this instead
    KDescendantsProxyModel *desc = new KDescendantsProxyModel(this);
    desc->setSourceModel(notetakerModel);
    Akonadi::EntityMimeTypeFilterModel *collectionsModel = new Akonadi::EntityMimeTypeFilterModel(this);
    collectionsModel->addMimeTypeExclusionFilter( Akonadi::Collection::mimeType() );
    collectionsModel->setSourceModel(desc);
    m_knowledgeBaseModel = collectionsModel;

    return m_knowledgeBaseModel;
}

QAbstractItemModel *ModelStack::topicsTreeModel()
{
    if (!m_topicsTreeModel) {
        ParentStructureModel *treeModel = new ParentStructureModel(new NepomukParentStructureStrategy(this), this);
        treeModel->setSourceModel(knowledgeBaseModel());
        m_topicsTreeModel = treeModel;
    }
    return m_topicsTreeModel;
}

QAbstractItemModel *ModelStack::knowledgeSidebarModel()
{
    if (!m_knowledgeSidebarModel) {
        SideBarModel *sideBarModel = new SideBarModel(this);
        sideBarModel->setSourceModel(topicsTreeModel());
        m_knowledgeSidebarModel = sideBarModel;
    }
    return m_knowledgeSidebarModel;
}

void ModelStack::setKnowledgeSelectionModel(QItemSelectionModel *selection)
{
    m_topicSelection = selection;
    if (m_knowledgeSelectionModel) {
        static_cast<SelectionProxyModel*>(m_knowledgeSelectionModel)->setSelectionModel(m_topicSelection);
    }
}

QItemSelectionModel* ModelStack::knowledgeSelection()
{
    return m_topicSelection;
}


QAbstractItemModel* ModelStack::knowledgeSelectionModel()
{
    if (!m_knowledgeSelectionModel) {
        SelectionProxyModel *categoriesSelectionModel = new SelectionProxyModel(this);
        categoriesSelectionModel->setSelectionModel(m_topicSelection);
        categoriesSelectionModel->setSourceModel(topicsTreeModel());
        m_knowledgeSelectionModel = categoriesSelectionModel;
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
        collectionsMonitor->setMimeTypeMonitored(AbstractPimItem::mimeType(AbstractPimItem::Note), true);

        Akonadi::EntityTreeModel *model = new Akonadi::EntityTreeModel(collectionsMonitor, this);

        Akonadi::EntityMimeTypeFilterModel *collectionsModel = new Akonadi::EntityMimeTypeFilterModel(this);
        collectionsModel->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );
        collectionsModel->setSourceModel(model);
        m_knowledgeCollectionsModel = collectionsModel;
    }
    return m_knowledgeCollectionsModel;
}
