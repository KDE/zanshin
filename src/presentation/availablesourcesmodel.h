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


#ifndef PRESENTATION_AVAILABLESOURCESMODEL_H
#define PRESENTATION_AVAILABLESOURCESMODEL_H

#include <QObject>

#include "domain/datasource.h"

#include "presentation/metatypes.h"

class QModelIndex;

namespace Domain {
    class DataSourceQueries;
    class DataSourceRepository;
}

namespace Presentation {

class AvailableSourcesModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* sourceListModel READ sourceListModel)
    Q_PROPERTY(QAbstractItemModel* searchListModel READ searchListModel)
    Q_PROPERTY(QString searchTerm READ searchTerm WRITE setSearchTerm NOTIFY searchTermChanged)
public:
    explicit AvailableSourcesModel(Domain::DataSourceQueries *dataSourceQueries,
                                   Domain::DataSourceRepository *dataSourceRepository,
                                   QObject *parent = 0);
    ~AvailableSourcesModel();

    QAbstractItemModel *sourceListModel();
    QAbstractItemModel *searchListModel();

    QString searchTerm() const;
    void setSearchTerm(const QString &term);

signals:
    void searchTermChanged(const QString &term);

public slots:
    void listSource(const Domain::DataSource::Ptr &source);
    void unlistSource(const Domain::DataSource::Ptr &source);
    void bookmarkSource(const Domain::DataSource::Ptr &source);

private:
    QAbstractItemModel *createSourceListModel();
    QAbstractItemModel *createSearchListModel();

    QAbstractItemModel *m_sourceListModel;
    QAbstractItemModel *m_searchListModel;

    Domain::DataSourceQueries *m_dataSourceQueries;
    Domain::DataSourceRepository *m_dataSourceRepository;
};

}

#endif // PRESENTATION_AVAILABLESOURCESMODEL_H
