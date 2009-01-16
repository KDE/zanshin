/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>
   Copyright 2008, 2009 Mario Bensi <nef@ipsquad.net>

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

#include <akonadi/control.h>

#include <akonadi/itemcreatejob.h>
#include <akonadi/itemmodel.h>

#include <boost/shared_ptr.hpp>

#include <kcal/todo.h>

#include <KDE/KActionCollection>
#include <KDE/KConfigGroup>
#include <KDE/KDebug>
#include <KDE/KLineEdit>
#include <KDE/KLocale>
#include <KDE/KTabWidget>

#include <QtGui/QDockWidget>
#include <QtGui/QHeaderView>
#include <QtGui/QVBoxLayout>

#include "actionlistmodel.h"
#include "actionlistview.h"
#include "configdialog.h"
#include "contextsmodel.h"
#include "globalmodel.h"
#include "globalsettings.h"
#include "librarymodel.h"
#include "projectsmodel.h"
#include "todocategoriesmodel.h"
#include "todoflatmodel.h"
#include "todotreemodel.h"

typedef boost::shared_ptr<KCal::Incidence> IncidencePtr;

MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent)
{
    Akonadi::Control::start();

    setupCentralWidget();

    QTreeView *contextTree = new QTreeView(this);
    contextTree->setAnimated(true);
    contextTree->setModel(GlobalModel::contextsLibrary());
    contextTree->setSelectionMode(QAbstractItemView::SingleSelection);
    contextTree->setDragEnabled(true);
    contextTree->viewport()->setAcceptDrops(true);
    contextTree->setDropIndicatorShown(true);
    connect(contextTree->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(onContextChanged(QModelIndex)));

    QDockWidget *dock = new QDockWidget(i18n("Contexts"), this);
    dock->setObjectName("ContextsDock");
    dock->setWidget(contextTree);
    addDockWidget(Qt::LeftDockWidgetArea, dock);


    QTreeView *projectTree = new QTreeView(this);
    projectTree->setAnimated(true);
    projectTree->setModel(GlobalModel::projectsLibrary());
    projectTree->setSelectionMode(QAbstractItemView::SingleSelection);
    projectTree->setDragEnabled(true);
    projectTree->viewport()->setAcceptDrops(true);
    projectTree->setDropIndicatorShown(true);
    connect(projectTree->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(onProjectChanged(QModelIndex)));

    dock = new QDockWidget(i18n("Projects"), this);
    dock->setObjectName("ProjectsDock");
    dock->setWidget(projectTree);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    setupActions();
    setupGUI();
    restoreColumnState();
    applySettings();
}

void MainWindow::setupCentralWidget()
{
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(new QVBoxLayout(centralWidget));

    m_view = new ActionListView(centralWidget);
    centralWidget->layout()->addWidget(m_view);
    m_actionList = new ActionListModel(this);
    m_view->setModel(m_actionList);
    m_actionList->setSourceModel(GlobalModel::todoFlat());

    m_addActionEdit = new KLineEdit(centralWidget);
    centralWidget->layout()->addWidget(m_addActionEdit);
    m_addActionEdit->setClickMessage(i18n("Type and press enter to add an action"));
    m_addActionEdit->setClearButtonShown(true);
    connect(m_addActionEdit, SIGNAL(returnPressed()),
            this, SLOT(onAddActionRequested()));

    setCentralWidget(centralWidget);
}

void MainWindow::setupActions()
{
    KActionCollection *ac = actionCollection();
    ac->addAction(KStandardAction::Preferences, this, SLOT(showConfigDialog()));
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
    } else {
        m_actionList->setSourceModel(GlobalModel::todoTree());
        m_actionList->setMode(ActionListModel::ProjectMode);
        QModelIndex projIndex = GlobalModel::projectsLibrary()->mapToSource(current);
        QModelIndex focusIndex = GlobalModel::projects()->mapToSource(projIndex);
        m_actionList->setSourceFocusIndex(focusIndex);
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
    } else {
        m_actionList->setSourceModel(GlobalModel::todoCategories());
        m_actionList->setMode(ActionListModel::ContextMode);
        QModelIndex catIndex = GlobalModel::contextsLibrary()->mapToSource(current);
        QModelIndex focusIndex = GlobalModel::contexts()->mapToSource(catIndex);
        m_actionList->setSourceFocusIndex(focusIndex);
    }
}

void MainWindow::onAddActionRequested()
{
    QString summary = m_addActionEdit->text().trimmed();
    m_addActionEdit->setText(QString());

    if (summary.isEmpty()) return;

    QModelIndex current = m_actionList->mapToSource(m_view->currentIndex());
    int type = current.sibling(current.row(), TodoFlatModel::RowType).data().toInt();
    if (type==TodoFlatModel::StandardTodo) {
        current = current.parent();
    }

    QString parentRemoteId;
    if (m_actionList->sourceModel()==GlobalModel::todoTree()) {
        QModelIndex parentIndex = current.sibling(current.row(), TodoFlatModel::RemoteId);
        parentRemoteId = GlobalModel::todoTree()->data(parentIndex).toString();
    }

    QString category;
    if (m_actionList->sourceModel()==GlobalModel::todoCategories()) {
        QModelIndex categoryIndex = current.sibling(current.row(), TodoFlatModel::Summary);
        category = GlobalModel::todoCategories()->data(categoryIndex).toString();
    }

    KCal::Todo *todo = new KCal::Todo();
    todo->setSummary(summary);
    if (!parentRemoteId.isEmpty()) {
        todo->setRelatedToUid(parentRemoteId);
    }
    if (!category.isEmpty()) {
        todo->setCategories(category);
    }

    IncidencePtr incidence(todo);
    Akonadi::Item item;
    item.setMimeType("application/x-vnd.akonadi.calendar.todo");
    item.setPayload<IncidencePtr>(incidence);

    Akonadi::Collection collection = GlobalModel::todoFlat()->collection();
    Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob(item, collection);
    job->start();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveColumnsState();
    KXmlGuiWindow::closeEvent(event);
}

void MainWindow::saveAutoSaveSettings()
{
    saveColumnsState();
    KXmlGuiWindow::saveAutoSaveSettings();
}

void MainWindow::saveColumnsState()
{
    KConfigGroup cg = autoSaveConfigGroup();
    QByteArray state = m_view->header()->saveState();
    cg.writeEntry("MainHeaderState", state.toBase64());
}

void MainWindow::restoreColumnState()
{
    KConfigGroup cg = autoSaveConfigGroup();
    QByteArray state;
    if (cg.hasKey("MainHeaderState")) {
        state = cg.readEntry("MainHeaderState", state);
        m_view->header()->restoreState(QByteArray::fromBase64(state));
    }
}

void MainWindow::showConfigDialog()
{
    if (KConfigDialog::showDialog("settings")) {
        return;
    }

    ConfigDialog *dialog = new ConfigDialog(this, "settings",
                                            GlobalSettings::self());

    connect(dialog, SIGNAL(settingsChanged(const QString&)),
            this, SLOT(applySettings()));

    dialog->show();
}

void MainWindow::onSettingsChanged()
{
    Akonadi::Collection collection(GlobalSettings::collectionId());
    GlobalModel::todoFlat()->setCollection(collection);
}
