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


#include "availablesourcesmodel.h"

#include <QIcon>

#include "domain/datasourcequeries.h"

#include "presentation/querytreemodel.h"

using namespace Presentation;

AvailableSourcesModel::AvailableSourcesModel(Domain::DataSourceQueries *dataSourceQueries,
                                             QObject *parent)
    : QObject(parent),
      m_sourceListModel(0),
      m_dataSourceQueries(dataSourceQueries)
{
}

AvailableSourcesModel::~AvailableSourcesModel()
{
}

QAbstractItemModel *AvailableSourcesModel::sourceListModel()
{
    if (!m_sourceListModel)
        m_sourceListModel = createSourceListModel();
    return m_sourceListModel;
}

QAbstractItemModel *AvailableSourcesModel::createSourceListModel()
{
    auto query = [this] (const Domain::DataSource::Ptr &source) {
        if (!source)
            return m_dataSourceQueries->findTopLevel();
        else
            return m_dataSourceQueries->findChildren(source);
    };

    auto flags = [] (const Domain::DataSource::Ptr &source) {
        Q_UNUSED(source)
        return Qt::ItemIsSelectable
             | Qt::ItemIsEnabled;
    };

    auto data = [] (const Domain::DataSource::Ptr &source, int role) -> QVariant {
        if (role != Qt::DisplayRole
         && role != Qt::EditRole
         && role != Qt::DecorationRole
         && role != QueryTreeModelBase::IconNameRole) {
            return QVariant();
        }

        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return source->name();
        } else if (role == Qt::DecorationRole || role == QueryTreeModelBase::IconNameRole) {
            const QString iconName = source->iconName();

            if (role == Qt::DecorationRole)
                return QVariant::fromValue(QIcon::fromTheme(iconName));
            else
                return iconName;
        } else {
            return QVariant();
        }
    };

    auto setData = [] (const Domain::DataSource::Ptr &source, const QVariant &value, int role) {
        Q_UNUSED(source)
        Q_UNUSED(value)
        Q_UNUSED(role)
        return false;
    };

    auto drop = [] (const QMimeData *mimeData, Qt::DropAction, const Domain::DataSource::Ptr &source) {
        Q_UNUSED(mimeData)
        Q_UNUSED(source)
        return false;
    };

    auto drag = [](const Domain::DataSource::List &) -> QMimeData* {
        return 0;
    };

    return new QueryTreeModel<Domain::DataSource::Ptr>(query, flags, data, setData, drop, drag, this);
}
