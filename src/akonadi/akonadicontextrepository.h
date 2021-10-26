/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2014 Franck Arrecot<franck.arrecot@kgmail.com>
   * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
   */


#ifndef AKONADICONTEXTREPOSITORY_H
#define AKONADICONTEXTREPOSITORY_H

#include "domain/contextrepository.h"

#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

namespace Akonadi {

class ContextRepository : public QObject, public Domain::ContextRepository
{
    Q_OBJECT
public:
    typedef QSharedPointer<ContextRepository> Ptr;

    ContextRepository(const StorageInterface::Ptr &storage,
                      const SerializerInterface::Ptr &serializer);

    KJob *create(Domain::Context::Ptr context, Domain::DataSource::Ptr source) override;
    KJob *update(Domain::Context::Ptr context) override;
    KJob *remove(Domain::Context::Ptr context) override;

    KJob *associate(Domain::Context::Ptr context, Domain::Task::Ptr child) override;
    KJob *dissociate(Domain::Context::Ptr context, Domain::Task::Ptr child) override;
    KJob *dissociateAll(Domain::Task::Ptr child) override;

private:
    StorageInterface::Ptr m_storage;
    SerializerInterface::Ptr m_serializer;
};

}

#endif // AKONADICONTEXTREPOSITORY_H
