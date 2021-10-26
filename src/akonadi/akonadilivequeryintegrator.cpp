/*
 * SPDX-FileCopyrightText: 2015 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "akonadilivequeryintegrator.h"

using namespace Akonadi;

LiveQueryIntegrator::LiveQueryIntegrator(const SerializerInterface::Ptr &serializer,
                                         const MonitorInterface::Ptr &monitor,
                                         QObject *parent)
    : QObject(parent),
      m_serializer(serializer),
      m_monitor(monitor)
{
    connect(m_monitor.data(), &MonitorInterface::collectionSelectionChanged,
            this, &LiveQueryIntegrator::onCollectionSelectionChanged);

    connect(m_monitor.data(), &MonitorInterface::collectionAdded, this, &LiveQueryIntegrator::onCollectionAdded);
    connect(m_monitor.data(), &MonitorInterface::collectionRemoved, this, &LiveQueryIntegrator::onCollectionRemoved);
    connect(m_monitor.data(), &MonitorInterface::collectionChanged, this, &LiveQueryIntegrator::onCollectionChanged);

    connect(m_monitor.data(), &MonitorInterface::itemAdded, this, &LiveQueryIntegrator::onItemAdded);
    connect(m_monitor.data(), &MonitorInterface::itemRemoved, this, &LiveQueryIntegrator::onItemRemoved);
    connect(m_monitor.data(), &MonitorInterface::itemChanged, this, &LiveQueryIntegrator::onItemChanged);
}

void LiveQueryIntegrator::addRemoveHandler(const LiveQueryIntegrator::CollectionRemoveHandler &handler)
{
    m_collectionRemoveHandlers << handler;
}

void LiveQueryIntegrator::addRemoveHandler(const LiveQueryIntegrator::ItemRemoveHandler &handler)
{
    m_itemRemoveHandlers << handler;
}

void LiveQueryIntegrator::onCollectionSelectionChanged()
{
    foreach (const auto &weak, m_itemInputQueries) {
        auto query = weak.toStrongRef();
        if (query)
            query->reset();
    }
}

void LiveQueryIntegrator::onCollectionAdded(const Collection &collection)
{
    foreach (const auto &weak, m_collectionInputQueries) {
        auto query = weak.toStrongRef();
        if (query)
            query->onAdded(collection);
    }
}

void LiveQueryIntegrator::onCollectionRemoved(const Collection &collection)
{
    foreach (const auto &weak, m_collectionInputQueries) {
        auto query = weak.toStrongRef();
        if (query)
            query->onRemoved(collection);
    }

    foreach (const auto &handler, m_collectionRemoveHandlers)
        handler(collection);

    cleanupQueries();
}

void LiveQueryIntegrator::onCollectionChanged(const Collection &collection)
{
    foreach (const auto &weak, m_collectionInputQueries) {
        auto query = weak.toStrongRef();
        if (query)
            query->onChanged(collection);
    }
}

void LiveQueryIntegrator::onItemAdded(const Item &item)
{
    foreach (const auto &weak, m_itemInputQueries) {
        auto query = weak.toStrongRef();
        if (query)
            query->onAdded(item);
    }
}

void LiveQueryIntegrator::onItemRemoved(const Item &item)
{
    foreach (const auto &weak, m_itemInputQueries) {
        auto query = weak.toStrongRef();
        if (query)
            query->onRemoved(item);
    }

    foreach (const auto &handler, m_itemRemoveHandlers)
        handler(item);

    cleanupQueries();
}

void LiveQueryIntegrator::onItemChanged(const Item &item)
{
    foreach (const auto &weak, m_itemInputQueries) {
        auto query = weak.toStrongRef();
        if (query)
            query->onChanged(item);
    }
}

void LiveQueryIntegrator::cleanupQueries()
{
    m_collectionInputQueries.removeAll(Domain::LiveQueryInput<Collection>::WeakPtr());
    m_itemInputQueries.removeAll(Domain::LiveQueryInput<Item>::WeakPtr());
}
