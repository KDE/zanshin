/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>

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


#include "datasourcelistmodel.h"

using namespace Presentation;

DataSourceListModel::DataSourceListModel(const DataSourceList::Ptr &dataSourceList, QObject *parent)
    : QAbstractListModel(parent),
      m_dataSourceList(dataSourceList)
{
    m_dataSourceList->addPreInsertHandler([this](const Domain::DataSource::Ptr &, int index) {
                                        beginInsertRows(QModelIndex(), index, index);
                                    });
    m_dataSourceList->addPostInsertHandler([this](const Domain::DataSource::Ptr &, int) {
                                         endInsertRows();
                                     });
    m_dataSourceList->addPreRemoveHandler([this](const Domain::DataSource::Ptr &, int index) {
                                        beginRemoveRows(QModelIndex(), index, index);
                                    });
    m_dataSourceList->addPostRemoveHandler([this](const Domain::DataSource::Ptr &, int) {
                                         endRemoveRows();
                                     });
    m_dataSourceList->addPostReplaceHandler([this](const Domain::DataSource::Ptr &, int idx) {
                                         emit dataChanged(index(idx), index(idx));
                                     });
}

DataSourceListModel::~DataSourceListModel()
{
}

Qt::ItemFlags DataSourceListModel::flags(const QModelIndex &index) const
{
    if (!isModelIndexValid(index)) {
        return Qt::NoItemFlags;
    }

    return QAbstractListModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
}

int DataSourceListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return m_dataSourceList->data().size();
}

QVariant DataSourceListModel::data(const QModelIndex &index, int role) const
{
    if (!isModelIndexValid(index)) {
        return QVariant();
    }

    if (role != Qt::DisplayRole && role != Qt::CheckStateRole) {
        return QVariant();
    }

    const auto dataSource = dataSourceForIndex(index);
    if (role == Qt::DisplayRole)
        return dataSource->name();
    else
        return QVariant();
}

Domain::DataSource::Ptr DataSourceListModel::dataSourceForIndex(const QModelIndex &index) const
{
    return m_dataSourceList->data().at(index.row());
}

bool DataSourceListModel::isModelIndexValid(const QModelIndex &index) const
{
    return index.isValid()
        && index.column() == 0
        && index.row() >= 0
        && index.row() < m_dataSourceList->data().size();
}
