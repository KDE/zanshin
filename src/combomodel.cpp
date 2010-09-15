/* This file is part of Zanshin Todo.

   Copyright 2008-2010 Kevin Ottens <ervin@kde.org>
   Copyright 2008, 2009 Mario Bensi <nef@ipsquad.net>

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

#include "combomodel.h"

#include <kcal/todo.h>

#include <KDebug>
#include <KIcon>

#include "todomodel.h"

ComboModel::ComboModel(bool isCheckable, QObject *parent)
    : QSortFilterProxyModel(parent), m_isCheckable(isCheckable)
{
    setDynamicSortFilter(true);
}

ComboModel::~ComboModel()
{
}

bool ComboModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex sourceChild = sourceModel()->index(sourceRow, 0, sourceParent);
    return sourceChild.data(TodoModel::ItemTypeRole).toInt() != TodoModel::Inbox
        && sourceChild.data(TodoModel::ItemTypeRole).toInt() != TodoModel::Collection
        && sourceChild.data(TodoModel::ItemTypeRole).toInt() != TodoModel::CategoryRoot;
}

Qt::ItemFlags ComboModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || !m_isCheckable) {
        return QSortFilterProxyModel::flags(index);
    }

    return QSortFilterProxyModel::flags(index) | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
}

QVariant ComboModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::EditRole) {
        return m_selectedItems.join(", ");
    }
    if (role == Qt::CheckStateRole && m_isCheckable) {
        if (!m_selectedItems.isEmpty()) {
            foreach (QString item, m_selectedItems) {
                if (index.data(Qt::DisplayRole).toString().contains(item)) {
                    return Qt::Checked;
                }
            }
        }
        return Qt::Unchecked;
    }
    if (role == LastPathPartRole) {
        if (m_selectedItems.isEmpty()) {
            QStringList path = index.data(Qt::DisplayRole).toString().split(" / ");
            return path.last();
        } else {
            foreach (QString item, m_selectedItems) {
                if (index.data(Qt::DisplayRole).toString().contains(item)) {
                    return QVariant();
                }
            }
            QStringList path = index.data(Qt::DisplayRole).toString().split(" / ");
            return  m_selectedItems.join(", ") + ", " + path.last();
        }
    }
    return QSortFilterProxyModel::data(index, role);
}

bool ComboModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::CheckStateRole) {
        QStringList path = index.data(Qt::DisplayRole).toString().split(" / ");
        if (value.toInt() == Qt::Checked) {
            m_selectedItems << path.last();
        } else {
            m_selectedItems.removeOne(path.last());
        }
        QModelIndex idx = this->index(0, 0);

        emit(dataChanged(index, index));

        return true;
    }
    return QSortFilterProxyModel::setData(index, value, role);
}

void ComboModel::setSelectedItems(const QStringList &selectedItems)
{
    m_selectedItems = selectedItems;
}

QStringList ComboModel::selectedItems() const
{
    return m_selectedItems;
}

void ComboModel::checkItem(int row)
{
    QModelIndex idx = index(row, 0);
    checkItem(idx);
}

void ComboModel::checkItem(const QModelIndex &index)
{
    int checkState = index.data(Qt::CheckStateRole).toInt();
    if (checkState == Qt::Checked) {
        setData(index, Qt::Unchecked, Qt::CheckStateRole);
    } else {
        setData(index, Qt::Checked, Qt::CheckStateRole);
    }
}
