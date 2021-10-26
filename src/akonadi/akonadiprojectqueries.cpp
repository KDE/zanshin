/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "akonadiprojectqueries.h"

using namespace Akonadi;

ProjectQueries::ProjectQueries(const StorageInterface::Ptr &storage, const SerializerInterface::Ptr &serializer, const MonitorInterface::Ptr &monitor)
    : m_serializer(serializer),
      m_helpers(new LiveQueryHelpers(serializer, storage)),
      m_integrator(new LiveQueryIntegrator(serializer, monitor))
{
    m_integrator->addRemoveHandler([this] (const Item &item) {
        m_findTopLevel.remove(item.id());
    });
}

ProjectQueries::ProjectResult::Ptr ProjectQueries::findAll() const
{
    auto fetch = m_helpers->fetchItems(const_cast<ProjectQueries*>(this));
    auto predicate = [this] (const Akonadi::Item &item) {
        return m_serializer->isProjectItem(item);
    };
    m_integrator->bind("ProjectQueries::findAll", m_findAll, fetch, predicate);
    return m_findAll->result();
}

ProjectQueries::TaskResult::Ptr ProjectQueries::findTopLevel(Domain::Project::Ptr project) const
{
    Akonadi::Item item = m_serializer->createItemFromProject(project);
    auto &query = m_findTopLevel[item.id()];
    auto fetch = m_helpers->fetchSiblings(item, const_cast<ProjectQueries*>(this));
    auto predicate = [this, project] (const Akonadi::Item &item) {
        return m_serializer->isProjectChild(project, item);
    };
    m_integrator->bind("ProjectQueries::findTopLevel", query, fetch, predicate);
    return query->result();
}
