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
#include "todomodel.h"
#include "todotreemodel.h"

#include "kdescendantsproxymodel.h"

ModelStack::ModelStack(QObject *parent)
    : QObject(parent),
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
      m_categorySelection(0)
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
        changeRecorder->setMimeTypeMonitored("application/x-vnd.akonadi.calendar.todo");
        changeRecorder->setCollectionFetchScope(collectionScope);
        changeRecorder->setItemFetchScope(itemScope);
        changeRecorder->setSession(session);

        m_baseModel = new TodoModel(changeRecorder, this);
    }
    return m_baseModel;
}

QAbstractItemModel *ModelStack::collectionsModel()
{
    if (!m_collectionsModel) {
        Akonadi::EntityMimeTypeFilterModel *collectionsModel = new Akonadi::EntityMimeTypeFilterModel(this);
        collectionsModel->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );
        collectionsModel->setSourceModel(baseModel());
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
