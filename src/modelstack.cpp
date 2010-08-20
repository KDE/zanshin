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
#include <KDE/Akonadi/ItemFetchScope>

#include "sidebarmodel.h"
#include "selectionproxymodel.h"
#include "todocategoriesmodel.h"
#include "todomodel.h"
#include "todotreemodel.h"

ModelStack::ModelStack(QObject *parent)
    : QObject(parent),
      m_baseModel(0),
      m_treeModel(0),
      m_treeSideBarModel(0),
      m_treeSelectionModel(0),
      m_categoriesModel(0),
      m_categoriesSideBarModel(0),
      m_categoriesSelectionModel(0)
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

QAbstractItemModel *ModelStack::treeSelectionModel(QItemSelectionModel *selection)
{
    Q_ASSERT( (m_treeSelectionModel!=0 && selection==0)
           || (m_treeSelectionModel==0 && selection!=0) );
    if (!m_treeSelectionModel) {
        SelectionProxyModel *treeSelectionModel = new SelectionProxyModel(selection, this);
        treeSelectionModel->setSourceModel(treeModel());
        m_treeSelectionModel = treeSelectionModel;
    }
    return m_treeSelectionModel;
}

QAbstractItemModel *ModelStack::categoriesModel()
{
    if (!m_categoriesModel) {
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

QAbstractItemModel *ModelStack::categoriesSelectionModel(QItemSelectionModel *selection)
{
    Q_ASSERT( (m_categoriesSelectionModel!=0 && selection==0)
           || (m_categoriesSelectionModel==0 && selection!=0) );
    if (!m_categoriesSelectionModel) {
        SelectionProxyModel *categoriesSelectionModel = new SelectionProxyModel(selection, this);
        categoriesSelectionModel->setSourceModel(categoriesModel());
        m_categoriesSelectionModel = categoriesSelectionModel;
    }
    return m_categoriesSelectionModel;
}
