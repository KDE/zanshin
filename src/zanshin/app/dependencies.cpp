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
#include "akonadi/akonadinotequeries.h"
#include "akonadi/akonadinoterepository.h"
#include "akonadi/akonadiprojectqueries.h"
#include "akonadi/akonadiprojectrepository.h"
#include "akonadi/akonaditagqueries.h"
#include "akonadi/akonaditagrepository.h"
#include "akonadi/akonaditaskqueries.h"
#include "akonadi/akonaditaskrepository.h"

#include "akonadi/akonadimessaging.h"
#include "akonadi/akonadimonitorimpl.h"
#include "akonadi/akonadiserializer.h"
#include "akonadi/akonadistorage.h"

#include "presentation/applicationmodel.h"
#include "presentation/availabletaskpagesmodel.h"

#include "scripting/scripthandler.h"

#include "utils/dependencymanager.h"

void App::initializeDependencies()
{
    auto &deps = Utils::DependencyManager::globalInstance();

    deps.add<Akonadi::MessagingInterface, Akonadi::Messaging, Utils::DependencyManager::UniqueInstance>();
    deps.add<Akonadi::MonitorInterface, Akonadi::MonitorImpl, Utils::DependencyManager::UniqueInstance>();
    deps.add<Akonadi::SerializerInterface, Akonadi::Serializer, Utils::DependencyManager::UniqueInstance>();
    deps.add<Akonadi::StorageInterface, Akonadi::Storage, Utils::DependencyManager::UniqueInstance>();


    deps.add<Domain::ContextQueries,
             Akonadi::ContextQueries(Akonadi::StorageInterface*,
                                     Akonadi::SerializerInterface*,
                                     Akonadi::MonitorInterface*)>();

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

    deps.add<Domain::NoteQueries,
             Akonadi::NoteQueries(Akonadi::StorageInterface*,
                                  Akonadi::SerializerInterface*,
                                  Akonadi::MonitorInterface*)>();

    deps.add<Domain::NoteRepository,
             Akonadi::NoteRepository(Akonadi::StorageInterface*,
                                     Akonadi::SerializerInterface*)>();

    deps.add<Domain::ProjectQueries,
             Akonadi::ProjectQueries(Akonadi::StorageInterface*,
                                     Akonadi::SerializerInterface*,
                                     Akonadi::MonitorInterface*)>();

    deps.add<Domain::ProjectRepository,
             Akonadi::ProjectRepository(Akonadi::StorageInterface*,
                                        Akonadi::SerializerInterface*)>();

    deps.add<Domain::TagQueries,
             Akonadi::TagQueries(Akonadi::StorageInterface*,
                                 Akonadi::SerializerInterface*,
                                 Akonadi::MonitorInterface*)>();

    deps.add<Domain::TagRepository,
             Akonadi::TagRepository(Akonadi::StorageInterface*,
                                    Akonadi::SerializerInterface*)>();

    deps.add<Domain::TaskQueries,
             Akonadi::TaskQueries(Akonadi::StorageInterface*,
                                  Akonadi::SerializerInterface*,
                                  Akonadi::MonitorInterface*)>();

    deps.add<Domain::TaskRepository,
             Akonadi::TaskRepository(Akonadi::StorageInterface*,
                                     Akonadi::SerializerInterface*,
                                     Akonadi::MessagingInterface*)>();

    deps.add<Presentation::AvailablePagesModelInterface,
             Presentation::AvailableTaskPagesModel(Domain::ProjectQueries*,
                                                   Domain::ProjectRepository*,
                                                   Domain::ContextQueries*,
                                                   Domain::ContextRepository*,
                                                   Domain::TaskQueries*,
                                                   Domain::TaskRepository*,
                                                   Domain::NoteRepository*)>();

    deps.add<Presentation::ApplicationModel,
             Presentation::ApplicationModel(Domain::ProjectQueries*,
                                            Domain::ProjectRepository*,
                                            Domain::ContextQueries*,
                                            Domain::ContextRepository*,
                                            Domain::DataSourceQueries*,
                                            Domain::DataSourceRepository*,
                                            Domain::TaskQueries*,
                                            Domain::TaskRepository*,
                                            Domain::NoteRepository*,
                                            Domain::TagQueries*,
                                            Domain::TagRepository*)>();

    deps.add<Scripting::ScriptHandler,
            Scripting::ScriptHandler(Domain::TaskRepository*)>();
}
