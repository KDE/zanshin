/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "dependencies.h"

#include "akonadi/akonadicontextqueries.h"
#include "akonadi/akonadicontextrepository.h"
#include "akonadi/akonadidatasourcequeries.h"
#include "akonadi/akonadidatasourcerepository.h"
#include "akonadi/akonadiprojectqueries.h"
#include "akonadi/akonadiprojectrepository.h"
#include "akonadi/akonaditaskqueries.h"
#include "akonadi/akonaditaskrepository.h"

#include "akonadi/akonadicache.h"
#include "akonadi/akonadicachingstorage.h"
#include "akonadi/akonadimonitorimpl.h"
#include "akonadi/akonadiserializer.h"
#include "akonadi/akonadistorage.h"

#include "presentation/availablepagesmodel.h"
#include "presentation/availablesourcesmodel.h"
#include "presentation/editormodel.h"
#include "presentation/runningtaskmodel.h"

#include "utils/dependencymanager.h"

void Integration::initializeGlobalAppDependencies()
{
    auto &deps = Utils::DependencyManager::globalInstance();
    initializeDefaultAkonadiDependencies(deps);
    initializeDefaultDomainDependencies(deps);
    initializeDefaultPresentationDependencies(deps);
}

void Integration::initializeDefaultAkonadiDependencies(Utils::DependencyManager &deps)
{
    deps.add<Akonadi::Cache,
             Akonadi::Cache(Akonadi::SerializerInterface*, Akonadi::MonitorInterface*),
             Utils::DependencyManager::UniqueInstance>();
    deps.add<Akonadi::MonitorInterface, Akonadi::MonitorImpl, Utils::DependencyManager::UniqueInstance>();
    deps.add<Akonadi::SerializerInterface, Akonadi::Serializer, Utils::DependencyManager::UniqueInstance>();
    deps.add<Akonadi::StorageInterface, Utils::DependencyManager::UniqueInstance>([] (Utils::DependencyManager *deps) {
        return new Akonadi::CachingStorage(deps->create<Akonadi::Cache>(),
                                           Akonadi::StorageInterface::Ptr(new Akonadi::Storage));
    });
}

void Integration::initializeDefaultDomainDependencies(Utils::DependencyManager &deps)
{
    deps.add<Domain::ContextQueries,
             Akonadi::ContextQueries(Akonadi::StorageInterface*,
                                     Akonadi::SerializerInterface*,
                                     Akonadi::MonitorInterface*,
                                     Akonadi::Cache*)>();

    deps.add<Domain::ContextRepository,
             Akonadi::ContextRepository(Akonadi::StorageInterface*,
                                        Akonadi::SerializerInterface*)>();

    deps.add<Domain::DataSourceQueries,
             Akonadi::DataSourceQueries(Akonadi::StorageInterface*,
                                        Akonadi::SerializerInterface*,
                                        Akonadi::MonitorInterface*)>();

    deps.add<Domain::DataSourceRepository,
             Akonadi::DataSourceRepository(Akonadi::StorageInterface*,
                                           Akonadi::SerializerInterface*)>();

    deps.add<Domain::ProjectQueries,
             Akonadi::ProjectQueries(Akonadi::StorageInterface*,
                                     Akonadi::SerializerInterface*,
                                     Akonadi::MonitorInterface*)>();

    deps.add<Domain::ProjectRepository,
             Akonadi::ProjectRepository(Akonadi::StorageInterface*,
                                        Akonadi::SerializerInterface*)>();

    deps.add<Domain::TaskQueries,
             Akonadi::TaskQueries(Akonadi::StorageInterface*,
                                  Akonadi::SerializerInterface*,
                                  Akonadi::MonitorInterface*,
                                  Akonadi::Cache*)>();

    deps.add<Domain::TaskRepository,
             Akonadi::TaskRepository(Akonadi::StorageInterface*,
                                     Akonadi::SerializerInterface*)>();
}

void Integration::initializeDefaultPresentationDependencies(Utils::DependencyManager &deps)
{
    deps.add<Presentation::EditorModel>([] (Utils::DependencyManager *deps) {
        auto model = new Presentation::EditorModel;
        auto repository = deps->create<Domain::TaskRepository>();
        model->setSaveFunction([repository] (const Domain::Task::Ptr &task) {
            Q_ASSERT(task);
            return repository->update(task);
        });
        return model;
    });

    deps.add<Presentation::AvailablePagesModel,
             Presentation::AvailablePagesModel(Domain::DataSourceQueries*,
                                               Domain::ProjectQueries*,
                                               Domain::ProjectRepository*,
                                               Domain::ContextQueries*,
                                               Domain::ContextRepository*,
                                               Domain::TaskQueries*,
                                               Domain::TaskRepository*)>();

    deps.add<Presentation::AvailableSourcesModel,
             Presentation::AvailableSourcesModel(Domain::DataSourceQueries*,
                                                 Domain::DataSourceRepository*)>();

    deps.add<Presentation::RunningTaskModel,
             Presentation::RunningTaskModel(Domain::TaskQueries*,
                                            Domain::TaskRepository*)>();
}
