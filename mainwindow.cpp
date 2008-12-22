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

#include "todocategoriesattribute.h"
#include "todocategoriesmodel.h"
#include "todoflatmodel.h"
#include "todotreemodel.h"

MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent), m_globalModel(0), m_projectModel(0), m_contextModel(0)
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
    dock->setWidget(collectionList);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    Akonadi::CollectionModel *collectionModel = new Akonadi::CollectionStatisticsModel(this);
    Akonadi::CollectionFilterProxyModel *collectionProxyModel = new Akonadi::CollectionFilterProxyModel(this);
    collectionProxyModel->setSourceModel(collectionModel);
    collectionProxyModel->addMimeTypeFilter("application/x-vnd.akonadi.calendar.todo");

    collectionList->setModel(collectionProxyModel);

    m_globalModel = new TodoFlatModel(this);

    m_projectModel = new TodoTreeModel(this);
    m_projectModel->setSourceModel(m_globalModel);

    m_contextModel = new TodoCategoriesModel(this);
    m_contextModel->setSourceModel(m_globalModel);

    KTabWidget *tab = new KTabWidget(this);
    setCentralWidget(tab);

    Akonadi::ItemView *globalList = new Akonadi::ItemView(tab);
    globalList->setModel(m_globalModel);
    tab->addTab(globalList, i18n("All Actions"));

    Akonadi::ItemView *projectTree = new Akonadi::ItemView(tab);
    projectTree->setRootIsDecorated(true);
    projectTree->setModel(m_projectModel);
    tab->addTab(projectTree, i18n("Project Tree"));

    Akonadi::ItemView *contextTree = new Akonadi::ItemView(tab);
    contextTree->setRootIsDecorated(true);
    contextTree->setModel(m_contextModel);
    tab->addTab(contextTree, i18n("Context Tree"));

    setupGUI();
}

void MainWindow::collectionClicked(const Akonadi::Collection &collection)
{
    m_currentCollection = collection;
    m_globalModel->setCollection(collection);
}

