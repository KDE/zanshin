/*
 * SPDX-FileCopyrightText: 2014 Franck Arrecot <franck.arrecot@gmail.com>
 * SPDX-FileCopyrightText: 2014 Rémi Benoit <r3m1.benoit@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "akonadicontextqueries.h"

using namespace Akonadi;

ContextQueries::ContextQueries(const StorageInterface::Ptr &storage,
                               const SerializerInterface::Ptr &serializer,
                               const MonitorInterface::Ptr &monitor,
                               const Cache::Ptr &cache)
    : m_serializer(serializer),
      m_cache(cache),
      m_helpers(new LiveQueryHelpers(serializer, storage)),
      m_integrator(new LiveQueryIntegrator(serializer, monitor))
{
    m_integrator->addRemoveHandler([this] (const Item &contextItem) {
        const auto uid = m_serializer->contextUid(contextItem);
        if (!uid.isEmpty())
            m_findToplevel.remove(uid);
    });
}

ContextQueries::ContextResult::Ptr ContextQueries::findAll() const
{
    auto fetch = m_helpers->fetchItems(const_cast<ContextQueries*>(this));
    auto predicate = [this] (const Akonadi::Item &item) {
        return m_serializer->isContext(item);
    };
    m_integrator->bind("ContextQueries::findAll", m_findAll, fetch, predicate);
    return m_findAll->result();
}

ContextQueries::TaskResult::Ptr ContextQueries::findTopLevelTasks(Domain::Context::Ptr context) const
{
    Q_ASSERT(context);
    auto fetch = m_helpers->fetchItemsForContext(context, const_cast<ContextQueries*>(this));
    auto predicate = [this, context] (const Akonadi::Item &item) {
        if (!m_serializer->isContextChild(context, item))
            return false;

        const auto items = m_cache->items(item.parentCollection());
        auto currentItem = item;
        auto parentUid = m_serializer->relatedUidFromItem(currentItem);
        while (!parentUid.isEmpty()) {
            const auto parent = std::find_if(items.cbegin(), items.cend(),
                                             [this, parentUid] (const Akonadi::Item &item) {
                                                 return m_serializer->itemUid(item) == parentUid;
                                             });
            if (parent == items.cend())
                break;

            if (m_serializer->isContextChild(context, *parent))
                return false;

            currentItem = *parent;
            parentUid = m_serializer->relatedUidFromItem(currentItem);
        }

        return true;
    };
    auto contextUid = context->property("todoUid").toString();
    auto &query = m_findToplevel[contextUid];
    m_integrator->bind("ContextQueries::findTopLevelTasks", query, fetch, predicate);
    return query->result();
}

#include "moc_akonadicontextqueries.cpp"
