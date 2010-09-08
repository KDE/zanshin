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

#include "sidebarpage.h"

#include <KDE/Akonadi/EntityTreeView>
#include <KDE/KInputDialog>
#include <KDE/KLocale>
#include <KDE/KMessageBox>

#include <QtGui/QVBoxLayout>
#include <QtGui/QHeaderView>

SideBarPage::SideBarPage(QAbstractItemModel *model,
                         const QList<QAction*> &contextActions,
                         QWidget *parent)
    : QWidget(parent)
{
    setLayout(new QVBoxLayout(this));
    m_treeView = new Akonadi::EntityTreeView(this);
    layout()->addWidget(m_treeView);
    layout()->setContentsMargins(0, 0, 0, 0);

    m_treeView->setFocusPolicy(Qt::NoFocus);
    m_treeView->header()->hide();
    m_treeView->setSortingEnabled(true);
    m_treeView->sortByColumn(0, Qt::AscendingOrder);
    m_treeView->setAnimated(true);
    m_treeView->setModel(model);
    m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_treeView->setDragEnabled(true);
    m_treeView->viewport()->setAcceptDrops(true);
    m_treeView->setDropIndicatorShown(true);
    m_treeView->setRootIsDecorated(false);
    m_treeView->setStyleSheet("QTreeView { background: transparent; border-style: none; }");

    m_treeView->setCurrentIndex(m_treeView->model()->index(0, 0));

    connect(model, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            m_treeView, SLOT(expand(const QModelIndex&)));

    m_treeView->setContextMenuPolicy(Qt::ActionsContextMenu);
    m_treeView->addActions(contextActions);
}

QItemSelectionModel *SideBarPage::selectionModel() const
{
    return m_treeView->selectionModel();
}

void SideBarPage::addNewItem()
{

}

void SideBarPage::removeCurrentItem()
{

}

void SideBarPage::renameCurrentItem()
{

}

void SideBarPage::selectPreviousItem()
{
    QModelIndex index = m_treeView->currentIndex();
    index = m_treeView->indexAbove(index);

    if (index.isValid()) {
        m_treeView->setCurrentIndex(index);
    }
}

void SideBarPage::selectNextItem()
{
    QModelIndex index = m_treeView->currentIndex();
    m_treeView->expand(index);
    index = m_treeView->indexBelow(index);

    if (index.isValid()) {
        m_treeView->setCurrentIndex(index);
    }
}
