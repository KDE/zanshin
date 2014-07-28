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


#ifndef PRESENTATION_DATASOURCELISTMODEL_H
#define PRESENTATION_DATASOURCELISTMODEL_H

#include <QAbstractListModel>

#include "domain/queryresult.h"
#include "domain/datasource.h"

namespace Presentation {

class DataSourceListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum {
        IconNameRole = Qt::UserRole + 1
    };

    typedef Domain::QueryResult<Domain::DataSource::Ptr> DataSourceList;

    explicit DataSourceListModel(const DataSourceList::Ptr &dataSourceList,
                                 QObject *parent = 0);
    ~DataSourceListModel();

    Qt::ItemFlags flags(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

private:
    Domain::DataSource::Ptr dataSourceForIndex(const QModelIndex &index) const;
    bool isModelIndexValid(const QModelIndex &index) const;

    DataSourceList::Ptr m_dataSourceList;
};

}

#endif // PRESENTATION_DATASOURCELISTMODEL_H
