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

#ifndef DOMAIN_DATASOURCEQUERIES_H
#define DOMAIN_DATASOURCEQUERIES_H

#include <QObject>

#include "datasource.h"
#include "queryresult.h"

namespace Domain {

class DataSourceQueries;

class DataSourceQueriesNotifier : public QObject
{
    Q_OBJECT
signals:
    void defaultSourceChanged();

private:
    friend class DataSourceQueries;
};

class DataSourceQueries
{
public:
    typedef QSharedPointer<DataSourceQueries> Ptr;

    DataSourceQueries();
    virtual ~DataSourceQueries();

    DataSourceQueriesNotifier *notifier() const;

    virtual bool isDefaultSource(DataSource::Ptr source) const = 0;
    void setDefaultSource(DataSource::Ptr source);

// HACK: Ugly right? Find me another way to mock changeDefaultSource then...
#ifdef ZANSHIN_I_SWEAR_I_AM_IN_A_PRESENTATION_TEST
public:
#else
private:
#endif
    virtual void changeDefaultSource(DataSource::Ptr source) = 0;

public:
    virtual QueryResult<DataSource::Ptr>::Ptr findTopLevel() const = 0;
    virtual QueryResult<DataSource::Ptr>::Ptr findChildren(DataSource::Ptr source) const = 0;

    virtual QString searchTerm() const = 0;
    virtual void setSearchTerm(const QString &term) = 0;
    virtual QueryResult<DataSource::Ptr>::Ptr findSearchTopLevel() const = 0;
    virtual QueryResult<DataSource::Ptr>::Ptr findSearchChildren(DataSource::Ptr source) const = 0;

private:
    mutable QScopedPointer<DataSourceQueriesNotifier> m_notifier;
};

}

#endif // DOMAIN_DATASOURCEQUERIES_H
