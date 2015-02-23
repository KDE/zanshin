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

#include <QIcon>

using namespace Presentation;

DataSourceListModel::DataSourceListModel(const Query &query, QObject *parent)
    : QueryTreeModel<Domain::DataSource::Ptr>(
          [query] (const Domain::DataSource::Ptr &source) {
              if (source)
                  return Domain::QueryResultInterface<Domain::DataSource::Ptr>::Ptr();
              else
                  return query();
          },

          // flags
          [] (const Domain::DataSource::Ptr &) {
              return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
          },

          // data
          [] (const Domain::DataSource::Ptr &source, int role) -> QVariant {
              switch (role) {
              case Qt::DisplayRole:
                  return source->name();
              case Qt::DecorationRole:
                  return QIcon::fromTheme(source->iconName().isEmpty() ? "folder" : source->iconName());
              case IconNameRole:
                  return source->iconName().isEmpty() ? "folder" : source->iconName();
              default:
                  return QVariant();
              }
          },

          // setData
          [] (const Domain::DataSource::Ptr &, const QVariant &, int) {
              return false;
          },

          parent
      )
{
}
