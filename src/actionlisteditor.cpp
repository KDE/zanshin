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


ActionListEditor::ActionListEditor(ModelStack *models,
                                   QItemSelectionModel *projectSelection,
                                   QItemSelectionModel *categoriesSelection,
                                   KActionCollection *ac,
                                   QWidget *parent)
    : QWidget(parent),
      m_projectSelection(projectSelection),
      m_categoriesSelection(categoriesSelection),
      m_models(models),
      m_defaultCollectionId(-1)
{
    setLayout(new QVBoxLayout(this));

    m_stack = new QStackedWidget(this);
    layout()->addWidget(m_stack);
    layout()->setContentsMargins(0, 0, 0, 0);

    connect(projectSelection, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(onSideBarSelectionChanged(QModelIndex)));
    connect(categoriesSelection, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(onSideBarSelectionChanged(QModelIndex)));

    models->setItemTreeSelectionModel(projectSelection);
    models->setItemCategorySelectionModel(categoriesSelection);

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

    createPage(models->treeSelectionModel(), models, Zanshin::ProjectMode);
    createPage(models->categoriesSelectionModel(), models, Zanshin::CategoriesMode);

    m_comboBox = new ActionListComboBox(bottomBar);
    m_comboBox->view()->setTextElideMode(Qt::ElideLeft);
    m_comboBox->setMinimumContentsLength(20);
    m_comboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

    connect(m_comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onComboBoxChanged()));

    KDescendantsProxyModel *descendantProxyModel = new KDescendantsProxyModel(m_comboBox);
    descendantProxyModel->setSourceModel(models->collectionsModel());
    descendantProxyModel->setDisplayAncestorData(true);

    KConfigGroup config(KGlobal::config(), "General");
    m_defaultCollectionId = config.readEntry("defaultCollection", -1);

    if (m_defaultCollectionId > 0) {
        if (!selectDefaultCollection(descendantProxyModel, QModelIndex(),
                                     0, descendantProxyModel->rowCount()-1)) {
            connect(descendantProxyModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                    this, SLOT(onRowInsertedInComboBox(QModelIndex,int,int)));
        }
    }

    m_comboBox->setModel(descendantProxyModel);
    bottomBar->layout()->addWidget(m_comboBox);

    QToolBar *toolBar = new QToolBar(bottomBar);
    toolBar->setIconSize(QSize(16, 16));
    bottomBar->layout()->addWidget(toolBar);
    toolBar->addAction(m_cancelAdd);

    m_cancelAdd->setEnabled(false);

    updateActions();
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

    currentPage()->selectFirstIndex();
}

void ActionListEditor::onComboBoxChanged()
{
    QModelIndex collectionIndex = m_comboBox->model()->index( m_comboBox->currentIndex(), 0 );
    Akonadi::Collection collection = collectionIndex.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();

    KConfigGroup config(KGlobal::config(), "General");
    config.writeEntry("defaultCollection", QString::number(collection.id()));
    config.sync();

    for (int i=0; i<m_stack->count(); i++) {
        page(i)->setDefaultCollection(collection);
    }
}

bool ActionListEditor::selectDefaultCollection(QAbstractItemModel *model, const QModelIndex &parent, int begin, int end)
{
    for (int i = begin; i <= end; i++) {
        QModelIndex collectionIndex = model->index(i, 0, parent);
        Akonadi::Collection collection = collectionIndex.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
        if (collection.id() == m_defaultCollectionId) {
            m_comboBox->setCurrentIndex(i);
            m_defaultCollectionId = -1;
            return true;
        }
    }
    return false;
}

void ActionListEditor::onRowInsertedInComboBox(const QModelIndex &parent, int begin, int end)
{
    QAbstractItemModel *model = static_cast<QAbstractItemModel*>(sender());
    if (selectDefaultCollection(model, parent, begin, end)) {
        disconnect(this, SLOT(onRowInsertedInComboBox(QModelIndex,int,int)));
    }
}

void ActionListEditor::createPage(QAbstractItemModel *model, ModelStack *models, Zanshin::ApplicationMode mode)
{
    QList<QAction*> contextActions;
    contextActions << m_add
                   << m_remove
                   << m_move
                   << m_promote;

    ActionListEditorPage *page = new ActionListEditorPage(model, models, mode, contextActions, m_stack);

    connect(page->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(updateActions()));

    m_stack->addWidget(page);
}

void ActionListEditor::setupActions(KActionCollection *ac)
{
    m_add = ac->addAction("editor_add_action", this, SLOT(focusActionEdit()));
    m_add->setText(i18n("New Action"));
    m_add->setIcon(KIcon("list-add"));
    if (qgetenv("ZANSHIN_KONTACT_PLUGIN").isEmpty()) {
        m_add->setShortcut(Qt::CTRL | Qt::Key_N);
    }

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

    m_promote = ac->addAction("editor_promote_action", this, SLOT(onPromoteAction()));
    m_promote->setText(i18n("Promote Action as Project"));
    m_promote->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_P);
}

void ActionListEditor::updateActions()
{
    const QItemSelectionModel * const itemSelectionModel = currentPage()->selectionModel();
    const QModelIndex index = itemSelectionModel->currentIndex();
    int type = index.data(Zanshin::ItemTypeRole).toInt();

    m_remove->setEnabled(index.isValid()
                     && ((type==Zanshin::StandardTodo)
                       || type==Zanshin::ProjectTodo
                       || type==Zanshin::Category));
    m_move->setEnabled(index.isValid()
                   && (type==Zanshin::StandardTodo
                    || type==Zanshin::Category
                    || type==Zanshin::ProjectTodo));

    m_promote->setEnabled(index.isValid()
                       && type==Zanshin::StandardTodo
                       && itemSelectionModel->selectedRows().size() == 1);
}

void ActionListEditor::onAddActionRequested()
{
    QString summary = m_addActionEdit->text().trimmed();
    m_addActionEdit->setText(QString());

    currentPage()->addNewTodo(summary);
}

void ActionListEditor::onRemoveAction()
{
    QModelIndexList currentIndexes = currentPage()->selectionModel()->selectedRows();

    if (currentIndexes.isEmpty()) {
        return;
    }

    QModelIndexList currentProjects;
    QModelIndexList currentCategories;
    QModelIndexList currentTodos;

    foreach (const QModelIndex &index, currentIndexes) {
        const int type = index.data(Zanshin::ItemTypeRole).toInt();
        if (type==Zanshin::ProjectTodo) {
            currentProjects << index;
        } else if (type==Zanshin::Category) {
            currentCategories << index;
        } else if (type==Zanshin::StandardTodo) {
            currentTodos << index;
        }
    }

    // Remove todos and projects already present in selected projects
    if (!currentProjects.isEmpty()) {
        QStringList projectUidList;
        foreach (const QModelIndex project, currentProjects) {
            projectUidList << project.data(Zanshin::UidRole).toString();
        }

        QSet<QString> projects = QSet<QString>::fromList(projectUidList);

        foreach (const QModelIndex project, currentProjects) {
            QSet<QString> ancestors = QSet<QString>::fromList(project.data(Zanshin::AncestorsUidRole).toStringList());
            if (!ancestors.intersect(projects).isEmpty()) {
                currentProjects.removeOne(project);
            }
        }
        foreach (const QModelIndex todo, currentTodos) {
            QSet<QString> ancestors = QSet<QString>::fromList(todo.data(Zanshin::AncestorsUidRole).toStringList());
            if (!ancestors.intersect(projects).isEmpty()) {
                currentTodos.removeOne(todo);
            }
        }
    }

    // Remove categories if the parent is also in the list
    if (!currentCategories.isEmpty()) {
        QStringList categoryList;
        foreach (const QModelIndex project, currentCategories) {
            categoryList << project.data(Qt::EditRole).toString();
        }

        QSet<QString> categories = QSet<QString>::fromList(categoryList);

        foreach (const QModelIndex category, currentCategories) {
            QStringList pathList = category.data(Zanshin::CategoryPathRole).toString().split(CategoryManager::pathSeparator());
            pathList.removeLast();
            QSet<QString> ancestors = QSet<QString>::fromList(pathList);
            if (!ancestors.intersect(categories).isEmpty()) {
                currentCategories.removeOne(category);
            }
        }
    }

    QModelIndex current;
    QModelIndex mapperIndex;

    if (!currentProjects.isEmpty()) {
        current = m_projectSelection->currentIndex();
        if (current.isValid()) {
            KModelIndexProxyMapper mapper(current.model(), currentProjects[0].model());
            mapperIndex = mapper.mapRightToLeft(currentProjects[0]);
        }

        if (TodoHelpers::removeProjects(this, currentProjects)) {
            if (current==mapperIndex) {
               m_projectSelection->setCurrentIndex(current.parent(), QItemSelectionModel::Select);
            }
        }
    }

    if (!currentCategories.isEmpty()) {
        current = m_categoriesSelection->currentIndex();
        if (current.isValid()) {
            KModelIndexProxyMapper mapper(current.model(), currentCategories[0].model());
            mapperIndex = mapper.mapRightToLeft(currentCategories[0]);
        }
        if (CategoryManager::instance().removeCategories(this, currentCategories)) {
            m_categoriesSelection->setCurrentIndex(current.parent(), QItemSelectionModel::Select);
        }
    }

    if (!currentTodos.isEmpty()) {
        foreach (QModelIndex index, currentTodos) {
            currentPage()->removeTodo(index);
        }
    }
}

void ActionListEditor::onMoveAction()
{
    QAbstractItemModel *model;
    QModelIndex currentSelection;
    if (currentPage()->mode()==Zanshin::ProjectMode) {
        model = m_models->treeSideBarModel();
        currentSelection = m_projectSelection->currentIndex();
    } else {
        model = m_models->categoriesSideBarModel();
        currentSelection = m_categoriesSelection->currentIndex();
    }

    QuickSelectDialog dlg(this, model, currentPage()->mode(),
                          QuickSelectDialog::MoveAction);
    if (dlg.exec()==QDialog::Accepted) {
        QString selectedId = dlg.selectedId();
        QModelIndex index = dlg.selectedIndex();

        QModelIndexList list = currentPage()->selectionModel()->selectedIndexes();
        if (currentSelection.isValid() && !list.isEmpty()) {
            KModelIndexProxyMapper mapper(currentSelection.model(), list.first().model());
            foreach (QModelIndex current, list) {
                if (!current.isValid()) {
                    return;
                }

                QModelIndex mapperIndex = mapper.mapRightToLeft(current);

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
                        CategoryManager::instance().moveCategory(categoryPath, selectedId, dlg.selectedType());
                    } else {
                        CategoryManager::instance().moveTodoToCategory(current, selectedId, dlg.selectedType());
                    }
                    if (dlg.selectedType()==Zanshin::Category && currentSelection==mapperIndex) {
                        m_categoriesSelection->setCurrentIndex(index, QItemSelectionModel::Select);
                    } else {
                        currentPage()->selectSiblingIndex(current);
                    }
                }
            }
        }
    }
}

void ActionListEditor::onPromoteAction()
{
    QModelIndex currentIndex = currentPage()->selectionModel()->currentIndex();

    if (!currentIndex.isValid()) {
        return;
    }

    int type = currentIndex.data(Zanshin::ItemTypeRole).toInt();

    if (type!=Zanshin::StandardTodo) {
        return;
    }

    TodoHelpers::promoteTodo(currentIndex);
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
