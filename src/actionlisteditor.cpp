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
#include "categorymanager.h"
#include "globaldefs.h"
#include "modelstack.h"
#include "quickselectdialog.h"
#include "todohelpers.h"
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
      m_categoriesSelection(categoriesSelection),
      m_models(models)
{
    setLayout(new QVBoxLayout(this));

    m_stack = new QStackedWidget(this);
    layout()->addWidget(m_stack);
    layout()->setContentsMargins(0, 0, 0, 0);

    connect(projectSelection, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(onSideBarSelectionChanged(QModelIndex)));
    connect(categoriesSelection, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(onSideBarSelectionChanged(QModelIndex)));

    models->setItemTreeSelectionModel(projectSelection);
    models->setItemCategorySelectionModel(categoriesSelection);
    createPage(models->treeSelectionModel(), models, Zanshin::ProjectMode);
    createPage(models->categoriesSelectionModel(), models, Zanshin::CategoriesMode);

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
    int type = index.data(Zanshin::ItemTypeRole).toInt();

    m_comboBox->setVisible(type == Zanshin::Inbox
                        || type == Zanshin::Category
                        || type == Zanshin::CategoryRoot);

    currentPage()->setCollectionColumnHidden(type!=Zanshin::Inbox);
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
    int type = index.data(Zanshin::ItemTypeRole).toInt();

    m_remove->setEnabled(index.isValid()
                     && ((type==Zanshin::StandardTodo)
                       || type==Zanshin::ProjectTodo
                       || type==Zanshin::Category));
    m_move->setEnabled(index.isValid()
                   && (type==Zanshin::StandardTodo
                    || type==Zanshin::Category
                    || type==Zanshin::ProjectTodo));
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

    int type = currentIndex.data(Zanshin::ItemTypeRole).toInt();
    QModelIndex current;
    QModelIndex mapperIndex;

    if (type==Zanshin::ProjectTodo) {
        current = m_projectSelection->currentIndex();
    } else {
        current = m_categoriesSelection->currentIndex();
    }

    if (current.isValid()) {
        KModelIndexProxyMapper mapper(current.model(), currentIndex.model());
        mapperIndex = mapper.mapRightToLeft(currentIndex);
    }

    if (type==Zanshin::ProjectTodo) {
        if (TodoHelpers::removeProject(this, currentIndex)) {
            if (type==Zanshin::ProjectTodo && current==mapperIndex) {
                m_projectSelection->setCurrentIndex(current.parent(), QItemSelectionModel::Select);
            }
        }
    } else if (type==Zanshin::Category) {
        if (TodoHelpers::removeCategory(this, currentIndex)) {
            m_categoriesSelection->setCurrentIndex(current.parent(), QItemSelectionModel::Select);
        }
    } else {
        currentPage()->removeCurrentTodo();
    }
}

void ActionListEditor::onMoveAction()
{
    QModelIndex current = currentPage()->selectionModel()->currentIndex();

    if (!current.isValid()) {
        return;
    }

    QAbstractItemModel *model;
    QModelIndex currentSelection;
    if (currentPage()->mode()==Zanshin::ProjectMode) {
        model = m_models->treeSideBarModel();
        currentSelection = m_projectSelection->currentIndex();
    } else {
        model = m_models->categoriesSideBarModel();
        currentSelection = m_categoriesSelection->currentIndex();
    }

    QModelIndex mapperIndex;
    if (currentSelection.isValid()) {
        KModelIndexProxyMapper mapper(currentSelection.model(), current.model());
        mapperIndex = mapper.mapRightToLeft(current);
    }

    QuickSelectDialog dlg(this, model, currentPage()->mode(),
                          QuickSelectDialog::MoveAction);
    if (dlg.exec()==QDialog::Accepted) {
        QString selectedId = dlg.selectedId();
        QModelIndex index = dlg.selectedIndex();
        if (currentPage()->mode()==Zanshin::ProjectMode) {
            TodoHelpers::moveTodoToProject(current, selectedId, dlg.selectedType(), dlg.collection());
            if (dlg.selectedType()==Zanshin::ProjectTodo && currentSelection==mapperIndex) {
                m_projectSelection->setCurrentIndex(index, QItemSelectionModel::Select);
            } else {
                currentPage()->selectSiblingIndex(current);
            }
        } else {
            int type = current.data(Zanshin::ItemTypeRole).toInt();
            QString categoryPath = current.data(Zanshin::CategoryPathRole).toString();
            if (type==Zanshin::Category) {
                TodoHelpers::moveCategory(categoryPath, selectedId, dlg.selectedType());
            } else {
                TodoHelpers::moveTodoToCategory(current, selectedId, dlg.selectedType());
            }
            if (dlg.selectedType()==Zanshin::Category && currentSelection==mapperIndex) {
                m_categoriesSelection->setCurrentIndex(index, QItemSelectionModel::Select);
            } else {
                currentPage()->selectSiblingIndex(current);
            }
        }
    }
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
