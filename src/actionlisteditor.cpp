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

#include <KDE/Akonadi/EntityTreeView>

#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KConfigGroup>
#include <KDE/KDebug>
#include <kdescendantsproxymodel.h>
#include <kmodelindexproxymapper.h>
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

#include "actionlistcombobox.h"
#include "actionlistdelegate.h"
#include "actionlisteditorpage.h"
#include "modelstack.h"
#include "todohelpers.h"
#include "todomodel.h"
#if 0
#include "quickselectdialog.h"
#endif

ActionListEditor::ActionListEditor(ModelStack *models,
                                   QItemSelectionModel *projectSelection,
                                   QItemSelectionModel *categoriesSelection,
                                   KActionCollection *ac,
                                   QWidget *parent)
    : QWidget(parent),
      m_projectSelection(projectSelection),
      m_categoriesSelection(categoriesSelection)
{
    setLayout(new QVBoxLayout(this));

    m_stack = new QStackedWidget(this);
    layout()->addWidget(m_stack);
    layout()->setContentsMargins(0, 0, 0, 0);

    connect(projectSelection, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(onSideBarSelectionChanged(QModelIndex)));
    connect(categoriesSelection, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(onSideBarSelectionChanged(QModelIndex)));

    createPage(models->treeSelectionModel(projectSelection), models, Zanshin::ProjectMode);
    createPage(models->categoriesSelectionModel(categoriesSelection), models, Zanshin::CategoriesMode);

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

    m_comboBox = new ActionListComboBox(bottomBar);
    m_comboBox->view()->setTextElideMode(Qt::ElideLeft);
    m_comboBox->setMinimumContentsLength(20);
    m_comboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

    connect(m_comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onComboBoxChanged()));

    KDescendantsProxyModel *descendantProxyModel = new KDescendantsProxyModel(m_comboBox);
    descendantProxyModel->setSourceModel(models->collectionsModel());
    descendantProxyModel->setDisplayAncestorData(true);

    m_comboBox->setModel(descendantProxyModel);
    bottomBar->layout()->addWidget(m_comboBox);

    setupActions(ac);

    QToolBar *toolBar = new QToolBar(bottomBar);
    toolBar->setIconSize(QSize(16, 16));
    bottomBar->layout()->addWidget(toolBar);
    toolBar->addAction(m_cancelAdd);

    m_cancelAdd->setEnabled(false);

    updateActions(QModelIndex());
    setMode(Zanshin::ProjectMode);
    onComboBoxChanged();
}

void ActionListEditor::setMode(Zanshin::ApplicationMode mode)
{
    switch (mode) {
    case Zanshin::ProjectMode:
        m_stack->setCurrentIndex(0);
        onSideBarSelectionChanged(m_projectSelection->currentIndex());
        break;
    case Zanshin::CategoriesMode:
        m_stack->setCurrentIndex(1);
        onSideBarSelectionChanged(m_categoriesSelection->currentIndex());
        break;
    }
}

void ActionListEditor::onSideBarSelectionChanged(const QModelIndex &index)
{
    int type = index.data(TodoModel::ItemTypeRole).toInt();

    m_comboBox->setVisible(type == TodoModel::Inbox
                        || type == TodoModel::Category
                        || type == TodoModel::CategoryRoot);

    currentPage()->setCollectionColumnHidden(type!=TodoModel::Inbox);
}

void ActionListEditor::onComboBoxChanged()
{
    QModelIndex collectionIndex = m_comboBox->model()->index( m_comboBox->currentIndex(), 0 );
    Akonadi::Collection collection = collectionIndex.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();

    for (int i=0; i<m_stack->count(); i++) {
        page(i)->setDefaultCollection(collection);
    }
}

void ActionListEditor::createPage(QAbstractItemModel *model, ModelStack *models, Zanshin::ApplicationMode mode)
{
    ActionListEditorPage *page = new ActionListEditorPage(model, models, mode, m_stack);

    connect(page->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(updateActions(QModelIndex)));

    m_stack->addWidget(page);
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
    int type = index.data(TodoModel::ItemTypeRole).toInt();

    m_remove->setEnabled(index.isValid() 
                     && ((type==TodoModel::StandardTodo)
                       || type==TodoModel::ProjectTodo
                       || type==TodoModel::Category));
    m_move->setEnabled(index.isValid() && (type==TodoModel::StandardTodo));
}

void ActionListEditor::onAddActionRequested()
{
    QString summary = m_addActionEdit->text().trimmed();
    m_addActionEdit->setText(QString());

    currentPage()->addNewTodo(summary);
}

void ActionListEditor::onRemoveAction()
{
    QModelIndex currentIndex = currentPage()->selectionModel()->currentIndex();

    if (!currentIndex.isValid()) {
        return;
    }

    int type = currentIndex.data(TodoModel::ItemTypeRole).toInt();
    QModelIndex current;
    QModelIndex mapperIndex;

    if (type==TodoModel::ProjectTodo) {
        current = m_projectSelection->currentIndex();
    } else {
        current = m_categoriesSelection->currentIndex();
    }

    if (current.isValid()) {
        KModelIndexProxyMapper mapper(current.model(), currentIndex.model());
        mapperIndex = mapper.mapRightToLeft(currentIndex);
    }

    if (type==TodoModel::ProjectTodo) {
        if (TodoHelpers::removeProject(this, currentIndex)) {
            if (type==TodoModel::ProjectTodo && current==mapperIndex) {
                m_projectSelection->setCurrentIndex(current.parent(), QItemSelectionModel::Select);
            }
        }
    } else if (type==TodoModel::Category) {
        QString category = currentIndex.data().toString();
        if (TodoHelpers::removeCategory(this, category)) {
            m_categoriesSelection->setCurrentIndex(current.parent(), QItemSelectionModel::Select);
        }
    }
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
    page(0)->saveColumnsState(config, "ProjectHeaderState");
    page(1)->saveColumnsState(config, "CategoriesHeaderState");
}

void ActionListEditor::restoreColumnsState(const KConfigGroup &config)
{
    page(0)->restoreColumnsState(config, "ProjectHeaderState");
    page(1)->restoreColumnsState(config, "CategoriesHeaderState");
}

ActionListEditorPage *ActionListEditor::currentPage() const
{
    return static_cast<ActionListEditorPage*>(m_stack->currentWidget());
}

ActionListEditorPage *ActionListEditor::page(int idx) const
{
    return static_cast<ActionListEditorPage*>(m_stack->widget(idx));
}
