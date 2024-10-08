/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef AKONADI_TASKREPOSITORY_H
#define AKONADI_TASKREPOSITORY_H

#include "domain/taskrepository.h"

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

namespace Akonadi {

class TaskRepository : public QObject, public Domain::TaskRepository
{
    Q_OBJECT
public:
    typedef QSharedPointer<TaskRepository> Ptr;

    TaskRepository(const StorageInterface::Ptr &storage,
                   const SerializerInterface::Ptr &serializer);

    KJob *create(Domain::Task::Ptr task) override;
    KJob *createChild(Domain::Task::Ptr task, Domain::Task::Ptr parent) override;
    KJob *createInProject(Domain::Task::Ptr task, Domain::Project::Ptr project) override;
    KJob *createInContext(Domain::Task::Ptr task, Domain::Context::Ptr context) override;

    KJob *update(Domain::Task::Ptr task) override;
    KJob *remove(Domain::Task::Ptr task) override;

    KJob *promoteToProject(Domain::Task::Ptr task) override;

    KJob *associate(Domain::Task::Ptr parent, Domain::Task::Ptr child) override;
    KJob *dissociate(Domain::Task::Ptr child) override;
    KJob *dissociateAll(Domain::Task::Ptr child) override;

private:
    StorageInterface::Ptr m_storage;
    SerializerInterface::Ptr m_serializer;

    KJob *createItem(const Akonadi::Item &item);
};

}

#endif // AKONADI_TASKREPOSITORY_H
