/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef DOMAIN_DATASOURCEQUERIES_H
#define DOMAIN_DATASOURCEQUERIES_H

#include <QObject>

#include "datasource.h"
#include "project.h"
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
    virtual QueryResult<DataSource::Ptr>::Ptr findAllSelected() const = 0;
    virtual QueryResult<Project::Ptr>::Ptr findProjects(DataSource::Ptr source) const = 0;
};

}

#endif // DOMAIN_DATASOURCEQUERIES_H
