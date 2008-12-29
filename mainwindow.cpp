/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.
*/

#include "mainwindow.h"

#include <akonadi/attributefactory.h>

#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/collectionstatisticsdelegate.h>
#include <akonadi/collectionstatisticsmodel.h>
#include <akonadi/collectionview.h>

#include <akonadi/control.h>

#include <akonadi/itemmodel.h>
#include <akonadi/itemview.h>

#include <KDE/KLocale>
#include <KDE/KTabWidget>

#include <QtGui/QDockWidget>

#include "contextsmodel.h"
#include "projectsmodel.h"
#include "todocategoriesattribute.h"
#include "todocategoriesmodel.h"
#include "todoflatmodel.h"
#include "todotreemodel.h"

MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent), m_globalModel(0), m_treeModel(0), m_categoriesModel(0)
{
    Akonadi::Control::start();
    Akonadi::AttributeFactory::registerAttribute<TodoCategoriesAttribute>();

    Akonadi::CollectionView *collectionList = new Akonadi::CollectionView(this);
    connect(collectionList, SIGNAL(clicked(const Akonadi::Collection &)),
            this, SLOT(collectionClicked(const Akonadi::Collection &)));

    Akonadi::CollectionStatisticsDelegate *collectionDelegate
        = new Akonadi::CollectionStatisticsDelegate(collectionList);
    collectionDelegate->setUnreadCountShown(true);

    collectionList->setItemDelegate(collectionDelegate);

    QDockWidget *dock = new QDockWidget(i18n("Resources"), this);
    dock->setObjectName("ResourcesDock");
    dock->setWidget(collectionList);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    Akonadi::CollectionModel *collectionModel = new Akonadi::CollectionStatisticsModel(this);
    Akonadi::CollectionFilterProxyModel *collectionProxyModel = new Akonadi::CollectionFilterProxyModel(this);
    collectionProxyModel->setSourceModel(collectionModel);
    collectionProxyModel->addMimeTypeFilter("application/x-vnd.akonadi.calendar.todo");

    collectionList->setModel(collectionProxyModel);

    m_globalModel = new TodoFlatModel(this);

    m_treeModel = new TodoTreeModel(this);
    m_treeModel->setSourceModel(m_globalModel);

    m_categoriesModel = new TodoCategoriesModel(this);
    m_categoriesModel->setSourceModel(m_globalModel);

    KTabWidget *tab = new KTabWidget(this);
    setCentralWidget(tab);

    Akonadi::ItemView *globalList = new Akonadi::ItemView(tab);
    globalList->setModel(m_globalModel);
    tab->addTab(globalList, i18n("All Actions"));

    Akonadi::ItemView *todoTree = new Akonadi::ItemView(tab);
    todoTree->setRootIsDecorated(true);
    todoTree->setModel(m_treeModel);
    tab->addTab(todoTree, i18n("Todo Tree"));

    Akonadi::ItemView *categoriesTree = new Akonadi::ItemView(tab);
    categoriesTree->setRootIsDecorated(true);
    categoriesTree->setModel(m_categoriesModel);
    tab->addTab(categoriesTree, i18n("Categories Tree"));


    QTreeView *contextTree = new QTreeView(this);
    ContextsModel *contextsModel = new ContextsModel(this);
    contextsModel->setSourceModel(m_categoriesModel);
    contextTree->setModel(contextsModel);

    dock = new QDockWidget(i18n("Contexts"), this);
    dock->setObjectName("ContextsDock");
    dock->setWidget(contextTree);
    addDockWidget(Qt::LeftDockWidgetArea, dock);


    QTreeView *projectTree = new QTreeView(this);
    ProjectsModel *projectsModel = new ProjectsModel(this);
    projectsModel->setSourceModel(m_treeModel);
    projectTree->setModel(projectsModel);

    dock = new QDockWidget(i18n("Projects"), this);
    dock->setObjectName("ProjectsDock");
    dock->setWidget(projectTree);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    setupGUI();
}

void MainWindow::collectionClicked(const Akonadi::Collection &collection)
{
    m_currentCollection = collection;
    m_globalModel->setCollection(collection);
}

