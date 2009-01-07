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

#include <KDE/KDebug>
#include <KDE/KLocale>
#include <KDE/KTabWidget>

#include <QtGui/QDockWidget>

#include "actionlistmodel.h"
#include "actionlistview.h"
#include "contextsmodel.h"
#include "globalmodel.h"
#include "librarymodel.h"
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

    QDockWidget *dock = new QDockWidget(i18n("Resources"), this);
    dock->setObjectName("ResourcesDock");
    dock->setWidget(collectionList);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    Akonadi::CollectionModel *collectionModel = new Akonadi::CollectionModel(this);
    Akonadi::CollectionFilterProxyModel *collectionProxyModel = new Akonadi::CollectionFilterProxyModel(this);
    collectionProxyModel->setSourceModel(collectionModel);
    collectionProxyModel->addMimeTypeFilter("application/x-vnd.akonadi.calendar.todo");

    collectionList->setModel(collectionProxyModel);

    m_view = new ActionListView(this);
    m_actionList = new ActionListModel(this);
    m_view->setModel(m_actionList);
    m_actionList->setSourceModel(GlobalModel::todoFlat());
    m_view->setRootIsDecorated(false);
    setCentralWidget(m_view);

    QTreeView *contextTree = new QTreeView(this);
    contextTree->setAnimated(true);
    contextTree->setModel(GlobalModel::contextsLibrary());
    connect(contextTree->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(onContextChanged(QModelIndex)));

    dock = new QDockWidget(i18n("Contexts"), this);
    dock->setObjectName("ContextsDock");
    dock->setWidget(contextTree);
    addDockWidget(Qt::LeftDockWidgetArea, dock);


    QTreeView *projectTree = new QTreeView(this);
    projectTree->setAnimated(true);
    projectTree->setModel(GlobalModel::projectsLibrary());
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
    delete m_actionList;
    m_actionList = new ActionListModel(this);
    m_view->setModel(m_actionList);
    if (GlobalModel::projectsLibrary()->isInbox(current)) {
        m_actionList->setSourceModel(GlobalModel::todoFlat());
        m_actionList->setMode(ActionListModel::NoProjectMode);
        m_view->setRootIsDecorated(false);
    } else {
        m_actionList->setSourceModel(GlobalModel::todoTree());
        QModelIndex projIndex = GlobalModel::projectsLibrary()->mapToSource(current);
        QModelIndex focusIndex = GlobalModel::projects()->mapToSource(projIndex);
        m_actionList->setSourceFocusIndex(focusIndex);
        m_view->setRootIsDecorated(true);
    }
}

void MainWindow::onContextChanged(const QModelIndex &current)
{
    delete m_actionList;
    m_actionList = new ActionListModel(this);
    m_view->setModel(m_actionList);
    if (GlobalModel::contextsLibrary()->isInbox(current)) {
        m_actionList->setSourceModel(GlobalModel::todoFlat());
        m_actionList->setMode(ActionListModel::NoContextMode);
        m_view->setRootIsDecorated(false);
    } else {
        m_actionList->setSourceModel(GlobalModel::todoCategories());
        QModelIndex catIndex = GlobalModel::contextsLibrary()->mapToSource(current);
        QModelIndex focusIndex = GlobalModel::contexts()->mapToSource(catIndex);
        m_actionList->setSourceFocusIndex(focusIndex);
        m_view->setRootIsDecorated(true);
    }
}

