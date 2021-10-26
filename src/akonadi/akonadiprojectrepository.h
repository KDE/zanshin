/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef AKONADI_PROJECTREPOSITORY_H
#define AKONADI_PROJECTREPOSITORY_H

#include "domain/projectrepository.h"

#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

namespace Akonadi {

class ProjectRepository : public QObject, public Domain::ProjectRepository
{
    Q_OBJECT
public:
    typedef QSharedPointer<ProjectRepository> Ptr;

    ProjectRepository(const StorageInterface::Ptr &storage,
                      const SerializerInterface::Ptr &serializer);

    KJob *create(Domain::Project::Ptr project, Domain::DataSource::Ptr source) override;
    KJob *update(Domain::Project::Ptr project) override;
    KJob *remove(Domain::Project::Ptr project) override;

    KJob *associate(Domain::Project::Ptr parent, Domain::Task::Ptr child) override;
    KJob *dissociate(Domain::Task::Ptr child) override;

private:
    StorageInterface::Ptr m_storage;
    SerializerInterface::Ptr m_serializer;
};

}

#endif // AKONADI_PROJECTREPOSITORY_H
