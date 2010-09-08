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

#include "actionlisteditorpage.h"

#include <KDE/Akonadi/EntityTreeView>
#include <KDE/KConfigGroup>

#include <QtGui/QHeaderView>
#include <QtGui/QVBoxLayout>

#include "actionlistdelegate.h"

ActionListEditorPage::ActionListEditorPage(QAbstractItemModel *model,
                                           ModelStack *models,
                                           QWidget *parent)
    : QWidget(parent)
{
    setLayout(new QVBoxLayout(this));
    layout()->setContentsMargins(0, 0, 0, 0);

    m_treeView = new Akonadi::EntityTreeView(this);
    m_treeView->setModel(model);
    m_treeView->setItemDelegate(new ActionListDelegate(models, m_treeView));

    m_treeView->header()->setSortIndicatorShown(true);
    m_treeView->setSortingEnabled(true);
    m_treeView->sortByColumn(0, Qt::AscendingOrder);

    m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_treeView->setItemsExpandable(false);
    m_treeView->setEditTriggers(m_treeView->editTriggers() | QAbstractItemView::SelectedClicked);

    connect(m_treeView->model(), SIGNAL(modelReset()),
            m_treeView, SLOT(expandAll()));
    connect(m_treeView->model(), SIGNAL(layoutChanged()),
            m_treeView, SLOT(expandAll()));
    connect(m_treeView->model(), SIGNAL(rowsInserted(QModelIndex, int, int)),
            m_treeView, SLOT(expandAll()));

    layout()->addWidget(m_treeView);
}

QItemSelectionModel *ActionListEditorPage::selectionModel() const
{
    return m_treeView->selectionModel();
}

void ActionListEditorPage::saveColumnsState(KConfigGroup &config, const QString &key) const
{
    QByteArray state = m_treeView->header()->saveState();
    config.writeEntry(key, state.toBase64());
}

void ActionListEditorPage::restoreColumnsState(const KConfigGroup &config, const QString &key)
{
    QByteArray state;

    if (config.hasKey(key)) {
        state = config.readEntry(key, state);
        m_treeView->header()->restoreState(QByteArray::fromBase64(state));
    }
}

void ActionListEditorPage::addNewTodo(const QString &summary)
{

}

void ActionListEditorPage::removeCurrentTodo()
{

}
