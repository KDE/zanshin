/* This file is part of Zanshin Todo.

   Copyright 2008-2009 Kevin Ottens <ervin@kde.org>

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

#include "sidebar.h"

#include <akonadi/item.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/itemdeletejob.h>
#include <akonadi/transactionsequence.h>

#include <boost/shared_ptr.hpp>

#include <kcal/todo.h>

#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KIcon>
#include <KDE/KInputDialog>
#include <KDE/KLocale>
#include <KDE/KMessageBox>

#include <QtCore/QTimer>
#include <QtGui/QStackedWidget>
#include <QtGui/QToolBar>
#include <QtGui/QTreeView>
#include <QtGui/QVBoxLayout>

#include "contextsmodel.h"
#include "globalmodel.h"
#include "librarymodel.h"
#include "projectsmodel.h"
#include "todocategoriesmodel.h"
#include "todoflatmodel.h"
#include "todotreemodel.h"

typedef boost::shared_ptr<KCal::Incidence> IncidencePtr;

SideBar::SideBar(QWidget *parent, KActionCollection *ac)
    : QWidget(parent)
{
    setupActions(ac);

    setLayout(new QVBoxLayout(this));
    m_stack = new QStackedWidget(this);
    layout()->addWidget(m_stack);

    setupProjectPage();
    setupContextPage();
}

void SideBar::setupProjectPage()
{
    QWidget *projectPage = new QWidget(m_stack);
    projectPage->setLayout(new QVBoxLayout(projectPage));

    m_projectTree = new QTreeView(projectPage);
    projectPage->layout()->addWidget(m_projectTree);
    m_projectTree->setAnimated(true);
    m_projectTree->setModel(GlobalModel::projectsLibrary());
    m_projectTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_projectTree->setDragEnabled(true);
    m_projectTree->viewport()->setAcceptDrops(true);
    m_projectTree->setDropIndicatorShown(true);
    m_projectTree->setCurrentIndex(m_projectTree->model()->index(0, 0));
    connect(m_projectTree->model(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            m_projectTree, SLOT(expand(const QModelIndex&)));
    connect(m_projectTree->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(onCurrentProjectChangeDetected()));
    connect(m_projectTree->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(updateActions(QModelIndex)));

    QToolBar *projectBar = new QToolBar(projectPage);
    projectPage->layout()->addWidget(projectBar);
    projectBar->addAction(m_add);
    projectBar->addAction(m_remove);
    projectBar->addAction(m_addFolder);

    m_stack->addWidget(projectPage);
}

void SideBar::setupContextPage()
{
    QWidget *contextPage = new QWidget(m_stack);
    contextPage->setLayout(new QVBoxLayout(contextPage));

    m_contextTree = new QTreeView(contextPage);
    contextPage->layout()->addWidget(m_contextTree);
    m_contextTree->setAnimated(true);
    m_contextTree->setModel(GlobalModel::contextsLibrary());
    m_contextTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_contextTree->setDragEnabled(true);
    m_contextTree->viewport()->setAcceptDrops(true);
    m_contextTree->setDropIndicatorShown(true);
    m_contextTree->setCurrentIndex(m_contextTree->model()->index(0, 0));
    connect(m_contextTree->model(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            m_contextTree, SLOT(expand(const QModelIndex&)));
    connect(m_contextTree->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(onCurrentContextChangeDetected()));
    connect(m_contextTree->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(updateActions(QModelIndex)));

    QToolBar *contextBar = new QToolBar(contextPage);
    contextPage->layout()->addWidget(contextBar);
    contextBar->addAction(m_add);
    contextBar->addAction(m_remove);

    m_stack->addWidget(contextPage);
}

void SideBar::setupActions(KActionCollection *ac)
{
    m_addFolder = ac->addAction("sidebar_new_folder", this, SLOT(onAddFolder()));
    m_addFolder->setText(i18n("New Folder"));
    m_addFolder->setIcon(KIcon("folder-new"));

    m_add = ac->addAction("sidebar_new", this, SLOT(onAddItem()));
    m_add->setText(i18n("New"));
    m_add->setIcon(KIcon("list-add"));

    m_remove = ac->addAction("sidebar_remove", this, SLOT(onRemoveItem()));
    m_remove->setText(i18n("Remove"));
    m_remove->setIcon(KIcon("list-remove"));

    m_previous = ac->addAction("sidebar_go_previous", this, SLOT(onPreviousItem()));
    m_previous->setText(i18n("Previous"));
    m_previous->setIcon(KIcon("go-previous"));

    m_next = ac->addAction("sidebar_go_next", this, SLOT(onNextItem()));
    m_next->setText(i18n("Next"));
    m_next->setIcon(KIcon("go-next"));
}

void SideBar::switchToProjectMode()
{
    m_stack->setCurrentIndex(ProjectPageIndex);
    m_add->setText("New Project");
    m_remove->setText("Remove Project/Folder");
    updateActions(m_projectTree->currentIndex());

    applyCurrentProjectChange();
}

void SideBar::switchToContextMode()
{
    m_stack->setCurrentIndex(ContextPageIndex);
    m_add->setText("New Context");
    m_remove->setText("Remove Context");
    updateActions(m_contextTree->currentIndex());

    applyCurrentProjectChange();
}

void SideBar::updateActions(const QModelIndex &index)
{
    const LibraryModel *model = qobject_cast<const LibraryModel*>(index.model());;

    if (model==GlobalModel::projectsLibrary()) {
        m_addFolder->setEnabled(true);
    } else {
        m_addFolder->setEnabled(false);
    }

    if (model->isInbox(index)) {
        m_addFolder->setEnabled(false);
        m_add->setEnabled(false);
        m_remove->setEnabled(false);
    } else {
        m_add->setEnabled(true);
        m_remove->setEnabled(!model->isLibraryRoot(index));

        QModelIndex sourceIndex = model->mapToSource(index); // into "projects"
        if (m_addFolder->isEnabled() && sourceIndex.isValid()) { // Shouldn't be enabled on projects
            sourceIndex = GlobalModel::projects()->mapToSource(sourceIndex); // into "todoTree"
            sourceIndex = sourceIndex.sibling(sourceIndex.row(), TodoFlatModel::RowType); // we want the row type
            if (GlobalModel::todoTree()->data(sourceIndex).toInt()!=TodoFlatModel::FolderTodo) {
                m_addFolder->setEnabled(false);
                m_add->setEnabled(false);
            }
        }
    }
}

void SideBar::onAddFolder()
{
    bool ok;
    QString summary = KInputDialog::getText(i18n("New Folder"),
                                            i18n("Enter folder name:"),
                                            QString(), &ok, this);
    summary = summary.trimmed();

    if (!ok || summary.isEmpty()) return;

    QModelIndex parent = m_projectTree->currentIndex();
    parent = GlobalModel::projectsLibrary()->mapToSource(parent);
    parent = GlobalModel::projects()->mapToSource(parent);
    parent = parent.sibling(parent.row(), TodoFlatModel::RemoteId);

    QString parentRemoteId = GlobalModel::todoTree()->data(parent).toString();

    KCal::Todo *todo = new KCal::Todo();
    todo->setSummary(summary);
    todo->addComment("X-Zanshin-Folder");

    if (!parentRemoteId.isEmpty()) {
        todo->setRelatedToUid(parentRemoteId);
    }

    IncidencePtr incidence(todo);
    Akonadi::Item item;
    item.setMimeType("application/x-vnd.akonadi.calendar.todo");
    item.setPayload<IncidencePtr>(incidence);

    Akonadi::Collection collection = GlobalModel::todoFlat()->collection();
    Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob(item, collection);
    job->start();
}

void SideBar::onAddItem()
{
    switch (m_stack->currentIndex()) {
    case ProjectPageIndex:
        addNewProject();
        break;
    case ContextPageIndex:
        addNewContext();
        break;
    default:
        Q_ASSERT(false);
    }
}

void SideBar::onRemoveItem()
{
    switch (m_stack->currentIndex()) {
    case ProjectPageIndex:
        removeCurrentProject();
        break;
    case ContextPageIndex:
        removeCurrentContext();
        break;
    default:
        Q_ASSERT(false);
    }
}

void SideBar::addNewProject()
{
    bool ok;
    QString summary = KInputDialog::getText(i18n("New Project"),
                                            i18n("Enter project name:"),
                                            QString(), &ok, this);
    summary = summary.trimmed();

    if (!ok || summary.isEmpty()) return;

    QModelIndex parent = m_projectTree->currentIndex();
    parent = GlobalModel::projectsLibrary()->mapToSource(parent);
    parent = GlobalModel::projects()->mapToSource(parent);
    parent = parent.sibling(parent.row(), TodoFlatModel::RemoteId);

    QString parentRemoteId = GlobalModel::todoTree()->data(parent).toString();

    KCal::Todo *todo = new KCal::Todo();
    todo->setSummary(summary);
    todo->addComment("X-Zanshin-Project");

    if (!parentRemoteId.isEmpty()) {
        todo->setRelatedToUid(parentRemoteId);
    }

    IncidencePtr incidence(todo);
    Akonadi::Item item;
    item.setMimeType("application/x-vnd.akonadi.calendar.todo");
    item.setPayload<IncidencePtr>(incidence);

    Akonadi::Collection collection = GlobalModel::todoFlat()->collection();
    Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob(item, collection);
    job->start();
}

void addDeleteTodoJobsHelper(const QModelIndex &index, Akonadi::TransactionSequence *transaction)
{
    for (int i=0; i<GlobalModel::todoTree()->rowCount(index); i++) {
        addDeleteTodoJobsHelper(index.child(i, 0), transaction);
    }

    Akonadi::Item item = GlobalModel::todoTree()->itemForIndex(index);
    new Akonadi::ItemDeleteJob(item, transaction);
}

void SideBar::removeCurrentProject()
{
    QModelIndex current = m_projectTree->currentIndex();
    current = GlobalModel::projectsLibrary()->mapToSource(current);
    current = GlobalModel::projects()->mapToSource(current);
    current = current.sibling(current.row(), 0);

    bool canRemove = true;

    if (GlobalModel::todoTree()->rowCount(current)>0) {
        QString summary = GlobalModel::todoTree()->data(current).toString();
        int rowType = GlobalModel::todoTree()->data(current.sibling(current.row(), TodoFlatModel::RowType)).toInt();
        QString message;
        QString caption;
        if (rowType==TodoFlatModel::FolderTodo) {
            message = i18n("Do you really want to delete the folder '%1', with all its projects and actions?", summary);
            caption = i18n("Delete Folder");
        } else {
            message = i18n("Do you really want to delete the project '%1', with all its actions?", summary);
            caption = i18n("Delete Project");
        }
        int button = KMessageBox::questionYesNo(this, message, caption);

        canRemove = (button==KMessageBox::Yes);
    }

    if (!canRemove) return;

    Akonadi::TransactionSequence *transaction = new Akonadi::TransactionSequence();
    addDeleteTodoJobsHelper(current, transaction);
    transaction->start();
}

void SideBar::addNewContext()
{
    bool ok;
    QString summary = KInputDialog::getText(i18n("New Context"),
                                            i18n("Enter context name:"),
                                            QString(), &ok, this);
    summary = summary.trimmed();

    if (!ok || summary.isEmpty()) return;

    QModelIndex parent = m_contextTree->currentIndex();
    parent = GlobalModel::contextsLibrary()->mapToSource(parent);
    parent = GlobalModel::contexts()->mapToSource(parent);
    parent = parent.sibling(parent.row(), TodoFlatModel::Summary);

    GlobalModel::todoCategories()->addCategory(summary, parent);
}

void SideBar::removeCurrentContext()
{
    QModelIndex current = m_contextTree->currentIndex();
    current = GlobalModel::contextsLibrary()->mapToSource(current);
    current = GlobalModel::contexts()->mapToSource(current);
    current = current.sibling(current.row(), 0);

    bool canRemove = true;

    QString summary = GlobalModel::todoCategories()->data(current).toString();

    if (GlobalModel::todoCategories()->rowCount(current)>0) {
        QString message = i18n("Do you really want to delete the context '%1', with all its sub-contexts? All actions won't be associated to those contexts anymore.", summary);
        QString caption = i18n("Delete Context");
        int button = KMessageBox::questionYesNo(this, message, caption);

        canRemove = (button==KMessageBox::Yes);
    }

    if (!canRemove) return;

    GlobalModel::todoCategories()->removeCategory(summary);
}

void SideBar::onCurrentProjectChangeDetected()
{
    QTimer::singleShot(0, this, SLOT(applyCurrentProjectChange()));
}

void SideBar::applyCurrentProjectChange()
{
    QModelIndex index = m_projectTree->currentIndex();
    const LibraryModel *model = qobject_cast<const LibraryModel*>(index.model());

    if (model->isInbox(index)) {
        emit noProjectInboxActivated();
    } else {
        emit projectActivated(model->mapToSource(index));
    }
}

void SideBar::onCurrentContextChangeDetected()
{
    QTimer::singleShot(0, this, SLOT(applyCurrentContextChange()));
}

void SideBar::applyCurrentContextChange()
{
    QModelIndex index = m_contextTree->currentIndex();
    const LibraryModel *model = qobject_cast<const LibraryModel*>(index.model());

    if (model->isInbox(index)) {
        emit noContextInboxActivated();
    } else {
        emit contextActivated(model->mapToSource(index));
    }
}
