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

#include "todocategoriesmodel.h"
#include "todotreemodel.h"
#include "todomodel.h"

QuickSelectDialog::QuickSelectDialog(QWidget *parent, QAbstractItemModel *model, Zanshin::ApplicationMode mode, ActionType action)
    : KDialog(parent), m_tree(0), m_model(model), m_mode(mode)
{
    QString caption;

    if (mode==Zanshin::CategoriesMode) {
        switch (action) {
        case MoveAction:
            caption = i18n("Move Actions to Category");
            break;
        case CopyAction:
            caption = i18n("Copy Actions to Category");
            break;
        case JumpAction:
            caption = i18n("Jump to Category");
            break;
        }
    } else if (mode==Zanshin::ProjectMode) {
        switch (action) {
        case MoveAction:
            caption = i18n("Move Actions to Project");
            break;
        case CopyAction:
            caption = i18n("Copy Actions to Project");
            break;
        case JumpAction:
            caption = i18n("Jump to Project");
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

    m_tree->setModel(m_model);
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tree->setCurrentIndex(m_model->index(0, 0));
    m_tree->expandAll();
    m_tree->setFocus(Qt::OtherFocusReason);
}

QString QuickSelectDialog::selectedId() const
{
    if (m_mode==Zanshin::ProjectMode) {
        return projectSelectedId();
    } else {
        return categorySelectedId();
    }
}

TodoModel::ItemType QuickSelectDialog::selectedType() const
{
    QModelIndex index = m_tree->selectionModel()->currentIndex();
    return (TodoModel::ItemType)index.data(TodoModel::ItemTypeRole).toInt();
}

QString QuickSelectDialog::categorySelectedId() const
{
    QModelIndex index = m_tree->selectionModel()->currentIndex();
    return index.data().toString();
}

QString QuickSelectDialog::projectSelectedId() const
{
    QModelIndex index = m_tree->selectionModel()->currentIndex();
    return index.data(TodoModel::UidRole).toString();
}

Akonadi::Collection QuickSelectDialog::collection() const
{
    QModelIndex index = m_tree->selectionModel()->currentIndex();
    Akonadi::Collection collection;
    TodoModel::ItemType type = (TodoModel::ItemType)index.data(TodoModel::ItemTypeRole).toInt();
    if (type == TodoModel::Collection) {
        collection = index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
    } else {
        const Akonadi::Item item = index.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
        collection = item.parentCollection();
    }
    return collection;
}
