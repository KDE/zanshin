/* This file is part of Zanshin Todo.

   Copyright 2009 Kevin Ottens <ervin@kde.org>

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

#include "quickselectdialog.h"

#include <KDE/KDebug>
#include <KDE/KLocale>

#include <QtGui/QLayout>
#include <QtGui/QTreeView>

#include "globalmodel.h"
#include "projectsmodel.h"
#include "contextsmodel.h"
#include "todocategoriesmodel.h"
#include "todoflatmodel.h"
#include "todotreemodel.h"

QuickSelectDialog::QuickSelectDialog(QWidget *parent, Mode mode, ActionType action)
    : KDialog(parent), m_tree(0), m_mode(mode)
{
    QString caption;

    if (mode==ContextMode) {
        switch (action) {
        case MoveAction:
            caption = i18n("Move Actions to Context");
            break;
        case CopyAction:
            caption = i18n("Copy Actions to Context");
            break;
        case JumpAction:
            caption = i18n("Jump to Context");
            break;
        }
    } else if (mode==ProjectMode) {
        switch (action) {
        case MoveAction:
            caption = i18n("Move Actions to Project or Folder");
            break;
        case CopyAction:
            caption = i18n("Copy Actions to Project or Folder");
            break;
        case JumpAction:
            caption = i18n("Jump to Project or Folder");
            break;
        }
    } else {
        kError() << "Shouldn't happen";
    }

    setCaption(caption);
    setButtons(Ok|Cancel);

    QWidget *page = mainWidget();
    page->setLayout(new QVBoxLayout(page));

    m_tree = new QTreeView(page);
    m_tree->setSortingEnabled(true);
    m_tree->sortByColumn(0, Qt::AscendingOrder);
    page->layout()->addWidget(m_tree);

    switch (mode) {
    case ProjectMode:
        m_tree->setModel(GlobalModel::projects());
        break;
    case ContextMode:
        m_tree->setModel(GlobalModel::contexts());
        break;
    }

    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tree->setCurrentIndex(m_tree->model()->index(0, 0));
    m_tree->expandAll();
    m_tree->setFocus(Qt::OtherFocusReason);
}

QString QuickSelectDialog::selectedId() const
{
    if (m_mode==ProjectMode) {
        return projectSelectedId();
    } else {
        return contextSelectedId();
    }
}

QString QuickSelectDialog::contextSelectedId() const
{
    QModelIndex index = m_tree->selectionModel()->currentIndex();
    QModelIndex sourceIndex = GlobalModel::contexts()->mapToSource(index);

    return GlobalModel::todoCategories()->data(sourceIndex.sibling(sourceIndex.row(), 0)).toString();
}

QString QuickSelectDialog::projectSelectedId() const
{
    QModelIndex index = m_tree->selectionModel()->currentIndex();
    QModelIndex sourceIndex = GlobalModel::projects()->mapToSource(index);

    return GlobalModel::todoTree()->data(sourceIndex.sibling(sourceIndex.row(), TodoFlatModel::RemoteId)).toString();
}
