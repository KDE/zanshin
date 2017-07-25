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
#include "akonadi/akonadimessaging.h"
#include "akonadi/akonadimonitorimpl.h"
#include "akonadi/akonadiserializer.h"
#include "akonadi/akonadistorage.h"

#include "presentation/artifacteditormodel.h"
#include "presentation/availablesourcesmodel.h"
#include "presentation/availabletaskpagesmodel.h"
#include "presentation/runningtaskmodel.h"

#include "utils/dependencymanager.h"

void App::initializeDependencies()
{
    auto &deps = Utils::DependencyManager::globalInstance();

    deps.add<Akonadi::Cache,
             Akonadi::Cache(Akonadi::SerializerInterface*, Akonadi::MonitorInterface*),
             Utils::DependencyManager::UniqueInstance>();
    deps.add<Akonadi::MessagingInterface, Akonadi::Messaging, Utils::DependencyManager::UniqueInstance>();
    deps.add<Akonadi::MonitorInterface, Akonadi::MonitorImpl, Utils::DependencyManager::UniqueInstance>();
    deps.add<Akonadi::SerializerInterface, Akonadi::Serializer, Utils::DependencyManager::UniqueInstance>();
    deps.add<Akonadi::StorageInterface, Utils::DependencyManager::UniqueInstance>([] (Utils::DependencyManager *deps) {
        return new Akonadi::CachingStorage(deps->create<Akonadi::Cache>(),
                                           Akonadi::StorageInterface::Ptr(new Akonadi::Storage));
    });

    deps.add<Domain::ContextQueries,
             Akonadi::ContextQueries(Akonadi::StorageInterface*,
                                     Akonadi::SerializerInterface*,
                                     Akonadi::MonitorInterface*,
                                     Akonadi::Cache*)>();

    deps.add<Domain::ContextRepository,
             Akonadi::ContextRepository(Akonadi::StorageInterface*,
                                        Akonadi::SerializerInterface*)>();

    deps.add<Domain::DataSourceQueries>([] (Utils::DependencyManager *deps) {
        return new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Tasks,
                                              deps->create<Akonadi::StorageInterface>(),
                                              deps->create<Akonadi::SerializerInterface>(),
                                              deps->create<Akonadi::MonitorInterface>());
    });

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
                                  Akonadi::MonitorInterface*)>();

    deps.add<Domain::TaskRepository,
             Akonadi::TaskRepository(Akonadi::StorageInterface*,
                                     Akonadi::SerializerInterface*,
                                     Akonadi::MessagingInterface*)>();

    deps.add<Presentation::ArtifactEditorModel>([] (Utils::DependencyManager *deps) {
        auto model = new Presentation::ArtifactEditorModel;
        auto repository = deps->create<Domain::TaskRepository>();
        model->setSaveFunction([repository] (const Domain::Artifact::Ptr &artifact) {
            auto task = artifact.objectCast<Domain::Task>();
            Q_ASSERT(task);
            return repository->update(task);
        });
        model->setDelegateFunction([repository] (const Domain::Task::Ptr &task, const Domain::Task::Delegate &delegate) {
            return repository->delegate(task, delegate);
        });
        return model;
    });

    deps.add<Presentation::AvailablePagesModelInterface,
             Presentation::AvailableTaskPagesModel(Domain::ProjectQueries*,
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
