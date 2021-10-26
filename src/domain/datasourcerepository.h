/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef DOMAIN_DATASOURCEREPOSITORY_H
#define DOMAIN_DATASOURCEREPOSITORY_H

#include "datasource.h"

class KJob;

namespace Domain {

class DataSourceRepository
{
public:
    typedef QSharedPointer<DataSourceRepository> Ptr;

    DataSourceRepository();
    virtual ~DataSourceRepository();

    virtual KJob *update(DataSource::Ptr source) = 0;

    virtual void showConfigDialog() = 0;
    virtual void windowNeedsDataBackend(QWidget *window) = 0;
};

}

#endif // DOMAIN_DATASOURCEREPOSITORY_H
