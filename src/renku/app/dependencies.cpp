/* This file is part of Renku Notes

   Copyright 2015 Kevin Ottens <ervin@kde.org>

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

#include "akonadi/akonadidatasourcequeries.h"
#include "akonadi/akonadidatasourcerepository.h"
#include "akonadi/akonadinotequeries.h"
#include "akonadi/akonadinoterepository.h"
#include "akonadi/akonaditagqueries.h"
#include "akonadi/akonaditagrepository.h"

#include "akonadi/akonadicache.h"
#include "akonadi/akonadicachingstorage.h"
#include "akonadi/akonadimonitorimpl.h"
#include "akonadi/akonadiserializer.h"
#include "akonadi/akonadistorage.h"

#include "presentation/artifacteditormodel.h"
#include "presentation/availablenotepagesmodel.h"
#include "presentation/availablesourcesmodel.h"

#include "utils/dependencymanager.h"

void App::initializeDependencies()
{
    auto &deps = Utils::DependencyManager::globalInstance();

    deps.add<Akonadi::Cache,
             Akonadi::Cache(Akonadi::SerializerInterface*, Akonadi::MonitorInterface*),
             Utils::DependencyManager::UniqueInstance>();
    deps.add<Akonadi::MonitorInterface, Akonadi::MonitorImpl, Utils::DependencyManager::UniqueInstance>();
    deps.add<Akonadi::SerializerInterface, Akonadi::Serializer, Utils::DependencyManager::UniqueInstance>();
    deps.add<Akonadi::StorageInterface, Utils::DependencyManager::UniqueInstance>([] (Utils::DependencyManager *deps) {
        return new Akonadi::CachingStorage(deps->create<Akonadi::Cache>(),
                                           Akonadi::StorageInterface::Ptr(new Akonadi::Storage));
    });


    deps.add<Domain::DataSourceQueries>([] (Utils::DependencyManager *deps) {
        return new Akonadi::DataSourceQueries(Akonadi::StorageInterface::Notes,
                                              deps->create<Akonadi::StorageInterface>(),
                                              deps->create<Akonadi::SerializerInterface>(),
                                              deps->create<Akonadi::MonitorInterface>());
    });

    deps.add<Domain::DataSourceRepository>([] (Utils::DependencyManager *deps) {
        return new Akonadi::DataSourceRepository(Akonadi::StorageInterface::Notes,
                                                 deps->create<Akonadi::StorageInterface>(),
                                                 deps->create<Akonadi::SerializerInterface>());
    });

    deps.add<Domain::NoteQueries,
             Akonadi::NoteQueries(Akonadi::StorageInterface*,
                                  Akonadi::SerializerInterface*,
                                  Akonadi::MonitorInterface*)>();

    deps.add<Domain::NoteRepository,
             Akonadi::NoteRepository(Akonadi::StorageInterface*,
                                     Akonadi::SerializerInterface*)>();

    deps.add<Domain::TagQueries,
             Akonadi::TagQueries(Akonadi::StorageInterface*,
                                 Akonadi::SerializerInterface*,
                                 Akonadi::MonitorInterface*)>();

    deps.add<Domain::TagRepository,
             Akonadi::TagRepository(Akonadi::StorageInterface*,
                                    Akonadi::SerializerInterface*)>();

    deps.add<Presentation::ArtifactEditorModel>([] (Utils::DependencyManager *deps) {
        auto model = new Presentation::ArtifactEditorModel;
        auto repository = deps->create<Domain::NoteRepository>();
        model->setSaveFunction([repository] (const Domain::Artifact::Ptr &artifact) {
            auto note = artifact.objectCast<Domain::Note>();
            Q_ASSERT(note);
            return repository->update(note);
        });
        return model;
    });

    deps.add<Presentation::AvailablePagesModelInterface,
             Presentation::AvailableNotePagesModel(Domain::NoteQueries*,
                                                   Domain::NoteRepository*,
                                                   Domain::TagQueries*,
                                                   Domain::TagRepository*)>();

    deps.add<Presentation::AvailableSourcesModel,
             Presentation::AvailableSourcesModel(Domain::DataSourceQueries*,
                                                 Domain::DataSourceRepository*)>();
}
