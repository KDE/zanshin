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

#include "actionlisteditor.h"

#include <akonadi/item.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/itemdeletejob.h>

#include <boost/shared_ptr.hpp>

#include <kcal/todo.h>

#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KIcon>
#include <KDE/KLineEdit>
#include <KDE/KLocale>
#include <KDE/KPassivePopup>

#include <QtGui/QVBoxLayout>

#include "actionlistmodel.h"
#include "actionlistview.h"
#include "contextsmodel.h"
#include "globalmodel.h"
#include "projectsmodel.h"
#include "todocategoriesmodel.h"
#include "todoflatmodel.h"
#include "todotreemodel.h"

typedef boost::shared_ptr<KCal::Incidence> IncidencePtr;

ActionListEditor::ActionListEditor(QWidget *parent, KActionCollection *ac)
    : QWidget(parent)
{
    setLayout(new QVBoxLayout(this));

    m_view = new ActionListView(this);
    layout()->addWidget(m_view);
    m_model = new ActionListModel(this);
    m_view->setModel(m_model);
    m_model->setSourceModel(GlobalModel::todoFlat());

    connect(m_view->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(updateActions(QModelIndex)));

    m_addActionEdit = new KLineEdit(this);
    layout()->addWidget(m_addActionEdit);
    m_addActionEdit->setClickMessage(i18n("Type and press enter to add an action"));
    m_addActionEdit->setClearButtonShown(true);
    connect(m_addActionEdit, SIGNAL(returnPressed()),
            this, SLOT(onAddActionRequested()));

    setupActions(ac);
    updateActions(QModelIndex());
}

ActionListView *ActionListEditor::view() const
{
    return m_view;
}

void ActionListEditor::setupActions(KActionCollection *ac)
{
    m_add = ac->addAction("editor_add_action", this, SLOT(focusActionEdit()));
    m_add->setText(i18n("New Action"));
    m_add->setIcon(KIcon("list-add"));

    m_remove = ac->addAction("editor_remove_action", this, SLOT(onRemoveAction()));
    m_remove->setText(i18n("Remove Action"));
    m_remove->setIcon(KIcon("list-remove"));

    m_previous = ac->addAction("editor_go_previous", this, SLOT(onPreviousAction()));
    m_previous->setText(i18n("Previous Action"));
    m_previous->setIcon(KIcon("go-previous"));

    m_next = ac->addAction("editor_go_next", this, SLOT(onNextAction()));
    m_next->setText(i18n("Next Action"));
    m_next->setIcon(KIcon("go-next"));
}

void ActionListEditor::updateActions(const QModelIndex &index)
{
    if (!index.isValid()) {
        m_add->setEnabled(false);
        m_remove->setEnabled(false);
    } else {
        m_add->setEnabled(true);
        m_remove->setEnabled(true);
    }
}

void ActionListEditor::onAddActionRequested()
{
    QString summary = m_addActionEdit->text().trimmed();
    m_addActionEdit->setText(QString());

    if (summary.isEmpty()) return;

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
}

void ActionListEditor::onRemoveAction()
{
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
}

void ActionListEditor::showNoProjectInbox()
{
    delete m_model;
    m_model = new ActionListModel(this);

    m_model->setSourceModel(GlobalModel::todoFlat());
    m_model->setMode(ActionListModel::NoProjectMode);

    m_view->setModel(m_model);
    updateActions(QModelIndex());
    connect(m_view->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(updateActions(QModelIndex)));
}

void ActionListEditor::focusOnProject(const QModelIndex &index)
{
    delete m_model;
    m_model = new ActionListModel(this);

    m_model->setSourceModel(GlobalModel::todoTree());
    m_model->setMode(ActionListModel::ProjectMode);
    QModelIndex focusIndex = GlobalModel::projects()->mapToSource(index);
    m_model->setSourceFocusIndex(focusIndex);

    m_view->setModel(m_model);
    updateActions(QModelIndex());
    connect(m_view->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(updateActions(QModelIndex)));
}

void ActionListEditor::showNoContextInbox()
{
    delete m_model;
    m_model = new ActionListModel(this);

    m_model->setSourceModel(GlobalModel::todoFlat());
    m_model->setMode(ActionListModel::NoContextMode);

    m_view->setModel(m_model);
    updateActions(QModelIndex());
    connect(m_view->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(updateActions(QModelIndex)));
}

void ActionListEditor::focusOnContext(const QModelIndex &index)
{
    delete m_model;
    m_model = new ActionListModel(this);

    m_model->setSourceModel(GlobalModel::todoCategories());
    m_model->setMode(ActionListModel::ContextMode);
    QModelIndex focusIndex = GlobalModel::contexts()->mapToSource(index);
    m_model->setSourceFocusIndex(focusIndex);

    m_view->setModel(m_model);
    updateActions(QModelIndex());
    connect(m_view->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(updateActions(QModelIndex)));
}

void ActionListEditor::focusActionEdit()
{
    QPoint pos = m_addActionEdit->geometry().topLeft();
    pos = m_addActionEdit->parentWidget()->mapToGlobal(pos);

    KPassivePopup *popup = KPassivePopup::message(i18n("Type and press enter to add an action"), m_addActionEdit);
    popup->move(pos-QPoint(0, popup->height()));
    m_addActionEdit->setFocus();
}
