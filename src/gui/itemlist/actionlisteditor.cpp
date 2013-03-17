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
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QStackedWidget>

#include "actionlistdelegate.h"
#include "actionlisteditorpage.h"
#include "globaldefs.h"
#include "core/modelstack.h"
#include "quickselectdialog.h"
#include "todohelpers.h"
#include "core/configuration.h"
#include "gui/itemeditor/itemviewer.h"
#include "itemselectorproxy.h"
#include "core/pimitemrelationinterface.h"



ActionListEditor::ActionListEditor(ModelStack *models,
                                   KActionCollection *ac,
                                   QWidget *parent, KXMLGUIClient *client, ItemViewer *itemViewer)
    : QWidget(parent),
      m_models(models),
      m_selectorProxy(new ItemSelectorProxy(this))
{
    setLayout(new QVBoxLayout(this));

    m_stack = new QStackedWidget(this);
    layout()->addWidget(m_stack);
    layout()->setContentsMargins(0, 0, 0, 0);

    connect(models->treeSelection(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(onSideBarSelectionChanged(QModelIndex)));
    connect(models->categoriesSelection(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(onSideBarSelectionChanged(QModelIndex)));
    connect(models->knowledgeSelection(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(onSideBarSelectionChanged(QModelIndex)));

    setupActions(ac);

    createPage(models->treeSelectionModel(), models, Zanshin::ProjectMode, client);
    createPage(models->categoriesSelectionModel(), models, Zanshin::CategoriesMode, client);
    createPage(models->knowledgeSelectionModel(), models, Zanshin::KnowledgeMode, client);

    connect(m_selectorProxy, SIGNAL(itemSelected(Akonadi::Item)), itemViewer, SLOT(setItem(const Akonadi::Item &)));
    //connect(&AkonadiCollection::instance(), SIGNAL(itemCreated(const Akonadi::Item &)), m_selectorProxy, SLOT(selectItem(const Akonadi::Item &)));
    updateActions();
    setMode(Zanshin::ProjectMode);
    m_cancelAdd->setEnabled(false);
}

QAbstractItemModel *ActionListEditor::currentSidebarModel(Zanshin::ApplicationMode mode) const
{
    switch (mode) {
    case Zanshin::ProjectMode:
        return m_models->treeSideBarModel();
    case Zanshin::CategoriesMode:
        return m_models->categoriesSideBarModel();
    case Zanshin::KnowledgeMode:
        return m_models->knowledgeSideBarModel();
    }
    Q_ASSERT(0);
    return 0;
}

QItemSelectionModel *ActionListEditor::currentSelection(Zanshin::ApplicationMode mode) const
{
    switch (mode) {
    case Zanshin::ProjectMode:
        return m_models->treeSelection();
    case Zanshin::CategoriesMode:
        return m_models->categoriesSelection();
    case Zanshin::KnowledgeMode:
        return m_models->knowledgeSelection();
    }
    Q_ASSERT(0);
    return 0;
}

void ActionListEditor::setMode(Zanshin::ApplicationMode mode)
{
    kDebug() << mode;
    switch (mode) {
    case Zanshin::ProjectMode:
        m_stack->setCurrentIndex(0);
        break;
    case Zanshin::CategoriesMode:
        m_stack->setCurrentIndex(1);
        break;
    case Zanshin::KnowledgeMode:
        m_stack->setCurrentIndex(2);
        break;
        break;
    }
    onSideBarSelectionChanged(currentSelection(mode)->currentIndex());
    m_selectorProxy->setView(currentPage()->treeView());
}

static Akonadi::Collection getCollection(const QModelIndex &index)
{
    const Zanshin::ItemType type = (Zanshin::ItemType) index.data(Zanshin::ItemTypeRole).toInt();

    Akonadi::Collection col;
    if ( type==Zanshin::Collection ) {
        col = index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
    } else {
        // We use ParentCollectionRole instead of Akonadi::Item::parentCollection() because the
        // information about the rights is not valid on retrieved items.
        col = index.data(Akonadi::EntityTreeModel::ParentCollectionRole).value<Akonadi::Collection>();
    }
    return col;
}

void ActionListEditor::onSideBarSelectionChanged(const QModelIndex &index)
{
    int type = index.data(Zanshin::ItemTypeRole).toInt();

    currentPage()->setCollectionSelectorVisible(type == Zanshin::Inbox
                        || type == Zanshin::Category
                        || type == Zanshin::CategoryRoot
                        || type == Zanshin::Topic
                        || type == Zanshin::TopicRoot);

    currentPage()->selectFirstIndex();
    currentPage()->setCurrentCollection(getCollection(index));
}


void ActionListEditor::createPage(QAbstractItemModel *model, ModelStack *models, Zanshin::ApplicationMode mode, KXMLGUIClient *client)
{
    QList<QAction*> contextActions;
    if (mode == Zanshin::CategoriesMode || mode == Zanshin::ProjectMode) {
        contextActions << m_add
                      << m_remove
                      << m_move
                      << m_promote;
    }

    if (mode==Zanshin::CategoriesMode) {
        contextActions << m_dissociate;
    }
    ActionListEditorPage *page = new ActionListEditorPage(model, models, mode, contextActions, QList<QAction*>() << m_cancelAdd, m_stack, client);

    connect(page->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(updateActions()));
    //connect(page->treeView(), SIGNAL(currentChanged(Akonadi::Item)), this, SIGNAL(currentChanged(Akonadi::Item)));

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
    connect(m_cancelAdd, SIGNAL(triggered()), this, SLOT(clearActionEdit()));
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

    m_dissociate = ac->addAction("editor_dissociate_action", this, SLOT(onDissociateAction()));
    m_dissociate->setText(i18n("Dissociate Action from Context"));
    m_dissociate->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_D);
}

void ActionListEditor::updateActions()
{
    const QItemSelectionModel * const itemSelectionModel = currentPage()->selectionModel();
    const QModelIndex index = itemSelectionModel->currentIndex();
    int type = index.data(Zanshin::ItemTypeRole).toInt();

    Akonadi::Collection collection;
    if ( type==Zanshin::Collection ) {
        collection = index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
    } else if (type==Zanshin::Category) {
        collection = Configuration::instance().defaultTodoCollection();
    } else if (type==Zanshin::Topic) {
        collection = Configuration::instance().defaultNoteCollection();
    } else if (type==Zanshin::StandardTodo) {
        QModelIndex parent = index;
        int parentType = type;
        while (parent.isValid() && parentType==Zanshin::StandardTodo) {
            parent = parent.sibling(parent.row()-1, parent.column());
            parentType = parent.data(Zanshin::ItemTypeRole).toInt();
        }

        if (parentType!=Zanshin::ProjectTodo) {
            collection = Configuration::instance().defaultTodoCollection();
        } else {
            collection = index.data(Akonadi::EntityTreeModel::ParentCollectionRole).value<Akonadi::Collection>();
        }
    } else {
        // We use ParentCollectionRole instead of Akonadi::Item::parentCollection() because the
        // information about the rights is not valid on retrieved items.
        collection = index.data(Akonadi::EntityTreeModel::ParentCollectionRole).value<Akonadi::Collection>();
    }

    m_add->setEnabled(index.isValid()
                  && (collection.rights() & Akonadi::Collection::CanCreateItem)
                  && (type==Zanshin::ProjectTodo
                   || type==Zanshin::Category
                   || type==Zanshin::Topic
                   || type==Zanshin::Inbox
                   || type==Zanshin::StandardTodo));

    currentPage()->setActionEditEnabled(m_add->isEnabled());

    m_remove->setEnabled(index.isValid()
                     && (collection.rights() & Akonadi::Collection::CanDeleteItem)
                     && ((type==Zanshin::StandardTodo)
                       || type==Zanshin::ProjectTodo
                       || type==Zanshin::Category
                       || type==Zanshin::Topic));

    m_move->setEnabled(index.isValid()
                   && (collection.rights() & Akonadi::Collection::CanDeleteItem)
                   && (type==Zanshin::StandardTodo
                    || type==Zanshin::Category
                    || type==Zanshin::Topic
                    || type==Zanshin::ProjectTodo));

    m_promote->setEnabled(index.isValid()
                       && (collection.rights() & Akonadi::Collection::CanChangeItem)
                       && type==Zanshin::StandardTodo
                       && itemSelectionModel->selectedRows().size() == 1);

    m_dissociate->setEnabled(index.isValid()
                          && (collection.rights() & Akonadi::Collection::CanDeleteItem)
                          && type==Zanshin::StandardTodo);
}

void ActionListEditor::onRemoveAction()
{
    QModelIndexList currentIndexes = currentPage()->selectionModel()->selectedRows();
    foreach (QModelIndex index, currentIndexes) {
        PimItemStructureInterface::remove(PimItemStructureInterface::fromIndex(index), this);
    }
}

void ActionListEditor::onMoveAction()
{
    const Zanshin::ApplicationMode mode = currentPage()->mode();
    QAbstractItemModel *model = currentSidebarModel(mode);
    QModelIndex currentIndex = currentSelection(mode)->currentIndex();

    QuickSelectDialog dlg(this, model, mode,
                          QuickSelectDialog::MoveAction);
    if (dlg.exec()==QDialog::Accepted) {
        QModelIndexList list = currentPage()->selectionModel()->selectedRows();
        if (currentIndex.isValid() && !list.isEmpty()) {
            KModelIndexProxyMapper mapper(currentIndex.model(), list.first().model());
            foreach (QModelIndex current, list) {
                if (!current.isValid()) {
                    return;
                }

                if (mode==Zanshin::ProjectMode) {
                    PimItemStructureInterface::moveTo(PimItemStructureInterface::fromIndex(current), PimItemStructureInterface::fromIndex(dlg.selectedIndex()));
                } else if (mode==Zanshin::CategoriesMode){
                    PimItemStructureInterface::linkTo(PimItemStructureInterface::fromIndex(current), PimItemStructureInterface::fromIndex(dlg.selectedIndex()));
                } else {
                    qWarning() << "not implemented";
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

void ActionListEditor::onDissociateAction()
{
    QModelIndexList currentIndexes = currentPage()->selectionModel()->selectedRows();
    foreach (QModelIndex index, currentIndexes) {
        currentPage()->dissociateTodo(index);
    }
}

void ActionListEditor::focusActionEdit()
{
    currentPage()->focusActionEdit();
}

void ActionListEditor::clearActionEdit()
{
    currentPage()->clearActionEdit();
}


bool ActionListEditor::eventFilter(QObject *watched, QEvent *event)
{
    if (watched==currentPage()->m_addActionEdit) {
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
    page(2)->saveColumnsState(config, "TopicsHeaderState");
}

void ActionListEditor::restoreColumnsState(const KConfigGroup &config)
{
    page(0)->restoreColumnsState(config, "ProjectHeaderState");
    page(1)->restoreColumnsState(config, "CategoriesHeaderState");
    page(2)->restoreColumnsState(config, "TopicsHeaderState");
}

ActionListEditorPage *ActionListEditor::currentPage() const
{
    return static_cast<ActionListEditorPage*>(m_stack->currentWidget());
}

ActionListEditorPage *ActionListEditor::page(int idx) const
{
    return static_cast<ActionListEditorPage*>(m_stack->widget(idx));
}
