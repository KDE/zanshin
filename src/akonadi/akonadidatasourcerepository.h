/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef AKONADI_DATASOURCEREPOSITORY_H
#define AKONADI_DATASOURCEREPOSITORY_H

#include "domain/datasourcerepository.h"

#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

namespace Akonadi {

class SerializerInterface;
class StorageInterface;

class DataSourceRepository : public QObject, public Domain::DataSourceRepository
{
    Q_OBJECT
public:
    typedef QSharedPointer<DataSourceRepository> Ptr;

    DataSourceRepository(const StorageInterface::Ptr &storage,
                         const SerializerInterface::Ptr &serializer);

    KJob *update(Domain::DataSource::Ptr source) override;

    void showConfigDialog() override;
    void windowNeedsDataBackend(QWidget *window) override;

private:
    StorageInterface::Ptr m_storage;
    SerializerInterface::Ptr m_serializer;
};

}

#endif // AKONADI_DATASOURCEREPOSITORY_H
