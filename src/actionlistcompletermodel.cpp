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

#include "actionlistcompletermodel.h"

#include <KDE/KCalCore/Todo>
#include <kmodelindexproxymapper.h>
#include <QItemSelectionModel>

ActionListCompleterModel::ActionListCompleterModel(QItemSelectionModel *selectionModel, QObject *parent)
    : QSortFilterProxyModel(parent), m_selectionModel(selectionModel), m_mapper(0)
{
    setDynamicSortFilter(true);
}

ActionListCompleterModel::~ActionListCompleterModel()
{
}

bool ActionListCompleterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (!m_mapper) {
        return false;
    }
    QModelIndex sourceChild = sourceModel()->index(sourceRow, 0, sourceParent);
    QModelIndex selectionIndex = m_mapper->mapRightToLeft(sourceChild);
    return !m_selectionModel->selectedIndexes().contains(selectionIndex);
}

QVariant ActionListCompleterModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::EditRole) {
        QStringList indexList;
        QModelIndexList indexes = m_selectionModel->selectedIndexes();
        foreach (const QModelIndex &index, indexes) {
            indexList << index.data(Qt::DisplayRole).toString().split(" / ").last();
        }
        if (indexList.isEmpty()) {
            return QSortFilterProxyModel::data(index).toString().split(" / ").last();
        } else {
            return QString(indexList.join(", ") + ", " + QSortFilterProxyModel::data(index).toString().split(" / ").last());
        }
    }
    return QSortFilterProxyModel::data(index, role);
}

void ActionListCompleterModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    delete m_mapper;
    m_mapper = new KModelIndexProxyMapper(m_selectionModel->model(), sourceModel, this);
    QSortFilterProxyModel::setSourceModel(sourceModel);
}
