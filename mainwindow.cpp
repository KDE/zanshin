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
#include "globalmodel.h"
#include "projectsmodel.h"
#include "todocategoriesmodel.h"
#include "todoflatmodel.h"
#include "todotreemodel.h"

MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent)
{
    Akonadi::Control::start();

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

    m_view = new Akonadi::ItemView(this);
    m_view->setIconSize(QSize(24, 24));
    m_view->setModel(GlobalModel::todoFlat());
    setCentralWidget(m_view);

    QTreeView *contextTree = new QTreeView(this);
    contextTree->setModel(GlobalModel::contexts());
    connect(contextTree->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(onContextChanged(QModelIndex)));

    dock = new QDockWidget(i18n("Contexts"), this);
    dock->setObjectName("ContextsDock");
    dock->setWidget(contextTree);
    addDockWidget(Qt::LeftDockWidgetArea, dock);


    QTreeView *projectTree = new QTreeView(this);
    projectTree->setModel(GlobalModel::projects());
    connect(projectTree->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(onProjectChanged(QModelIndex)));

    dock = new QDockWidget(i18n("Projects"), this);
    dock->setObjectName("ProjectsDock");
    dock->setWidget(projectTree);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    setupGUI();
}

void MainWindow::collectionClicked(const Akonadi::Collection &collection)
{
    GlobalModel::todoFlat()->setCollection(collection);
}

void MainWindow::onProjectChanged(const QModelIndex &current)
{
    m_view->setModel(GlobalModel::todoTree());
    QModelIndex projIndex = GlobalModel::projects()->mapToSource(current);
    m_view->setRootIsDecorated(true);
    m_view->setRootIndex(projIndex);
}

void MainWindow::onContextChanged(const QModelIndex &current)
{
    m_view->setModel(GlobalModel::todoCategories());
    QModelIndex catIndex = GlobalModel::contexts()->mapToSource(current);
    m_view->setRootIsDecorated(true);
    m_view->setRootIndex(catIndex);
}

