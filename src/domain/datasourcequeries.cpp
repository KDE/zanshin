/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "datasourcequeries.h"

using namespace Domain;

Q_GLOBAL_STATIC(DataSourceQueriesNotifier, s_notifier)

DataSourceQueries::DataSourceQueries()
{
}

DataSourceQueries::~DataSourceQueries()
{
}

DataSourceQueriesNotifier *DataSourceQueries::notifier() const
{
    return s_notifier();
}

void DataSourceQueries::setDefaultSource(DataSource::Ptr source)
{
    if (isDefaultSource(source))
        return;

    changeDefaultSource(source);
    emit notifier()->defaultSourceChanged();
}

#include "moc_datasourcequeries.cpp"
