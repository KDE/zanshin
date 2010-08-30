/* This file is part of Zanshin Todo.

   Copyright 2008-2010 Kevin Ottens <ervin@kde.org>

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

#include "actionlisteditor.h"

#if 0
#include <akonadi/item.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/itemdeletejob.h>

#include <boost/shared_ptr.hpp>

#include <kcal/todo.h>
#endif

#include <KDE/Akonadi/EntityTreeView>
#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KConfigGroup>
#include <KDE/KIcon>
#include <KDE/KLineEdit>
#include <KDE/KLocale>
#include <KDE/KPassivePopup>

#include <QtCore/QEvent>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QStackedWidget>

#include "actionlistdelegate.h"
#include "modelstack.h"
#if 0
#include "quickselectdialog.h"
#endif

ActionListEditor::ActionListEditor(ModelStack *models,
                                   QItemSelectionModel *projectSelection,
                                   QItemSelectionModel *categoriesSelection,
                                   KActionCollection *ac,
                                   QWidget *parent)
    : QWidget(parent)
{
    setLayout(new QVBoxLayout(this));

    m_stack = new QStackedWidget(this);
    layout()->addWidget(m_stack);
    layout()->setContentsMargins(0, 0, 0, 0);

    m_projectView = new Akonadi::EntityTreeView(m_stack);
    m_projectView->setModel(models->treeSelectionModel(projectSelection));
    m_projectView->setItemDelegate(new ActionListDelegate(models, m_projectView));

    m_projectView->header()->setSortIndicatorShown(true);
    m_projectView->setSortingEnabled(true);
    m_projectView->sortByColumn(0, Qt::AscendingOrder);

    m_projectView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_projectView->setItemsExpandable(false);
    m_projectView->setEditTriggers(m_projectView->editTriggers() | QAbstractItemView::SelectedClicked);

    connect(m_projectView->model(), SIGNAL(modelReset()),
            m_projectView, SLOT(expandAll()));
    connect(m_projectView->model(), SIGNAL(layoutChanged()),
            m_projectView, SLOT(expandAll()));
    connect(m_projectView->model(), SIGNAL(rowsInserted(QModelIndex, int, int)),
            m_projectView, SLOT(expandAll()));

    connect(m_projectView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(updateActions(QModelIndex)));

    m_stack->addWidget(m_projectView);

    m_categoriesView = new Akonadi::EntityTreeView(m_stack);
    m_categoriesView->setModel(models->categoriesSelectionModel(categoriesSelection));
    m_categoriesView->setItemDelegate(new ActionListDelegate(models, m_categoriesView));

    m_categoriesView->header()->setSortIndicatorShown(true);
    m_categoriesView->setSortingEnabled(true);
    m_categoriesView->sortByColumn(0, Qt::AscendingOrder);

    m_categoriesView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_categoriesView->setItemsExpandable(false);
    m_categoriesView->setEditTriggers(m_categoriesView->editTriggers() | QAbstractItemView::SelectedClicked);

    connect(m_categoriesView->model(), SIGNAL(modelReset()),
            m_categoriesView, SLOT(expandAll()));
    connect(m_categoriesView->model(), SIGNAL(layoutChanged()),
            m_categoriesView, SLOT(expandAll()));
    connect(m_categoriesView->model(), SIGNAL(rowsInserted(QModelIndex, int, int)),
            m_categoriesView, SLOT(expandAll()));

    connect(m_categoriesView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(updateActions(QModelIndex)));
    m_stack->addWidget(m_categoriesView);


    QWidget *bottomBar = new QWidget(this);
    layout()->addWidget(bottomBar);
    bottomBar->setLayout(new QHBoxLayout(bottomBar));
    bottomBar->layout()->setContentsMargins(0, 0, 0, 0);

    m_addActionEdit = new KLineEdit(bottomBar);
    m_addActionEdit->installEventFilter(this);
    bottomBar->layout()->addWidget(m_addActionEdit);
    m_addActionEdit->setClickMessage(i18n("Type and press enter to add an action"));
    m_addActionEdit->setClearButtonShown(true);
    connect(m_addActionEdit, SIGNAL(returnPressed()),
            this, SLOT(onAddActionRequested()));

    setupActions(ac);

    QToolBar *toolBar = new QToolBar(bottomBar);
    toolBar->setIconSize(QSize(16, 16));
    bottomBar->layout()->addWidget(toolBar);
    toolBar->addAction(m_cancelAdd);

    m_cancelAdd->setEnabled(false);
    updateActions(QModelIndex());
}

void ActionListEditor::setMode(Zanshin::ApplicationMode mode)
{
    switch (mode) {
    case Zanshin::ProjectMode:
        m_stack->setCurrentIndex(0);
        break;
    case Zanshin::CategoriesMode:
        m_stack->setCurrentIndex(1);
        break;
    }
}

void ActionListEditor::setupActions(KActionCollection *ac)
{
    m_add = ac->addAction("editor_add_action", this, SLOT(focusActionEdit()));
    m_add->setText(i18n("New Action"));
    m_add->setIcon(KIcon("list-add"));
    m_add->setShortcut(Qt::CTRL | Qt::Key_N);

    m_cancelAdd = ac->addAction("editor_cancel_action", m_stack, SLOT(setFocus()));
    connect(m_cancelAdd, SIGNAL(activated()), m_addActionEdit, SLOT(clear()));
    m_cancelAdd->setText(i18n("Cancel New Action"));
    m_cancelAdd->setIcon(KIcon("edit-undo"));
    m_cancelAdd->setShortcut(Qt::Key_Escape);

    m_remove = ac->addAction("editor_remove_action", this, SLOT(onRemoveAction()));
    m_remove->setText(i18n("Remove Action"));
    m_remove->setIcon(KIcon("list-remove"));
    m_remove->setShortcut(Qt::Key_Delete);

    m_move = ac->addAction("editor_move_action", this, SLOT(onMoveAction()));
    m_move->setText(i18n("Move Action..."));
    m_move->setShortcut(Qt::Key_M);
}

void ActionListEditor::updateActions(const QModelIndex &index)
{
    if (!index.isValid()) {
        m_remove->setEnabled(false);
        m_move->setEnabled(false);
    } else {
        m_remove->setEnabled(true);
        m_move->setEnabled(true);
    }
}

void ActionListEditor::onAddActionRequested()
{
    QString summary = m_addActionEdit->text().trimmed();
    m_addActionEdit->setText(QString());

    if (summary.isEmpty()) return;
#if 0
    QModelIndex current = m_model->mapToSource(m_view->currentIndex());
    int type = current.sibling(current.row(), TodoFlatModel::RowType).data().toInt();
    if (type==TodoFlatModel::StandardTodo) {
        current = current.parent();
    }

    QString parentRemoteId;
    if (m_model->sourceModel()==GlobalModel::todoTree()) {
        QModelIndex parentIndex = current.sibling(current.row(), TodoFlatModel::RemoteId);
        parentRemoteId = GlobalModel::todoTree()->data(parentIndex).toString();
    }

    QString category;
    if (m_model->sourceModel()==GlobalModel::todoCategories()) {
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
#endif
}

void ActionListEditor::onRemoveAction()
{
#if 0
    QModelIndex current = m_view->currentIndex();

    if (m_model->rowCount(current)>0) {
        return;
    }

    Akonadi::Item item;
    QAbstractItemModel *source = m_model->sourceModel();

    TodoFlatModel *flat = dynamic_cast<TodoFlatModel*>(source);
    if (flat != 0) {
        item = flat->itemForIndex(m_model->mapToSource(current));
    }

    TodoTreeModel *tree = dynamic_cast<TodoTreeModel*>(source);
    if (tree != 0) {
        item = tree->itemForIndex(m_model->mapToSource(current));
    }

    TodoCategoriesModel *categories = dynamic_cast<TodoCategoriesModel*>(source);
    if (categories != 0) {
        item = categories->itemForIndex(m_model->mapToSource(current));
    }

    if (!item.isValid()) {
        return;
    }

    new Akonadi::ItemDeleteJob(item, this);
#endif
}

void ActionListEditor::onMoveAction()
{
#if 0
    QModelIndex current = m_view->currentIndex();

    if (m_model->rowCount(current)>0) {
        return;
    }

    QAbstractItemModel *source = m_model->sourceModel();
    QModelIndex movedIndex;

    TodoFlatModel *flat = dynamic_cast<TodoFlatModel*>(source);
    if (flat != 0) {
        movedIndex = m_model->mapToSource(current);
    }

    TodoTreeModel *tree = dynamic_cast<TodoTreeModel*>(source);
    if (tree != 0) {
        movedIndex = m_model->mapToSource(current);
    }

    TodoCategoriesModel *categories = dynamic_cast<TodoCategoriesModel*>(source);
    if (categories != 0) {
        movedIndex = m_model->mapToSource(current);
    }

    if (!movedIndex.isValid()) {
        return;
    }

    QuickSelectDialog::Mode mode = QuickSelectDialog::ProjectMode;
    if (m_model->mode()==ActionListModel::NoContextMode
     || m_model->mode()==ActionListModel::ContextMode) {
        mode = QuickSelectDialog::ContextMode;
    }

    QuickSelectDialog dlg(this, mode,
                          QuickSelectDialog::MoveAction);
    if (dlg.exec()==QDialog::Accepted) {
        QString selectedId = dlg.selectedId();
        if (mode==QuickSelectDialog::ProjectMode) {
            QModelIndex index = movedIndex.sibling(movedIndex.row(), TodoFlatModel::ParentRemoteId);
            source->setData(index, selectedId);
        } else {
            QModelIndex index = movedIndex.sibling(movedIndex.row(), TodoFlatModel::Categories);
            source->setData(index, selectedId);
        }
    }
#endif
}

void ActionListEditor::focusActionEdit()
{
    QPoint pos = m_addActionEdit->geometry().topLeft();
    pos = m_addActionEdit->parentWidget()->mapToGlobal(pos);

    KPassivePopup *popup = KPassivePopup::message(i18n("Type and press enter to add an action"), m_addActionEdit);
    popup->move(pos-QPoint(0, popup->height()));
    m_addActionEdit->setFocus();
}

bool ActionListEditor::eventFilter(QObject *watched, QEvent *event)
{
    if (watched==m_addActionEdit) {
        if (event->type()==QEvent::FocusIn) {
            m_cancelAdd->setEnabled(true);
        } else  if (event->type()==QEvent::FocusOut) {
            m_cancelAdd->setEnabled(false);
        }
    }

    return QWidget::eventFilter(watched, event);
}

void ActionListEditor::saveColumnsState(KConfigGroup &config) const
{
    QByteArray state = m_projectView->header()->saveState();
    config.writeEntry("ProjectHeaderState", state.toBase64());

    state = m_categoriesView->header()->saveState();
    config.writeEntry("CategoriesHeaderState", state.toBase64());
}

void ActionListEditor::restoreColumnsState(const KConfigGroup &config)
{
    QByteArray state;

    if (config.hasKey("ProjectHeaderState")) {
        state = config.readEntry("ProjectHeaderState", state);
        m_projectView->header()->restoreState(QByteArray::fromBase64(state));
    }

    if (config.hasKey("CategoriesHeaderState")) {
        state = config.readEntry("CategoriesHeaderState", state);
        m_categoriesView->header()->restoreState(QByteArray::fromBase64(state));
    }
}
