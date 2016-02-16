/* This file is part of Zanshin

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

    connect(m_monitor.data(), &MonitorInterface::tagAdded, this, &LiveQueryIntegrator::onTagAdded);
    connect(m_monitor.data(), &MonitorInterface::tagRemoved, this, &LiveQueryIntegrator::onTagRemoved);
    connect(m_monitor.data(), &MonitorInterface::tagChanged, this, &LiveQueryIntegrator::onTagChanged);
}

void LiveQueryIntegrator::addRemoveHandler(const LiveQueryIntegrator::CollectionRemoveHandler &handler)
{
    m_collectionRemoveHandlers << handler;
}

void LiveQueryIntegrator::addRemoveHandler(const LiveQueryIntegrator::ItemRemoveHandler &handler)
{
    m_itemRemoveHandlers << handler;
}

void LiveQueryIntegrator::addRemoveHandler(const LiveQueryIntegrator::TagRemoveHandler &handler)
{
    m_tagRemoveHandlers << handler;
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

void LiveQueryIntegrator::onTagAdded(const Tag &tag)
{
    foreach (const auto &weak, m_tagInputQueries) {
        auto query = weak.toStrongRef();
        if (query)
            query->onAdded(tag);
    }
}

void LiveQueryIntegrator::onTagRemoved(const Tag &tag)
{
    foreach (const auto &weak, m_tagInputQueries) {
        auto query = weak.toStrongRef();
        if (query)
            query->onRemoved(tag);
    }

    foreach (const auto &handler, m_tagRemoveHandlers)
        handler(tag);

    cleanupQueries();
}

void LiveQueryIntegrator::onTagChanged(const Tag &tag)
{
    foreach (const auto &weak, m_tagInputQueries) {
        auto query = weak.toStrongRef();
        if (query)
            query->onChanged(tag);
    }
}

void LiveQueryIntegrator::cleanupQueries()
{
    m_collectionInputQueries.removeAll(Domain::LiveQueryInput<Collection>::WeakPtr());
    m_itemInputQueries.removeAll(Domain::LiveQueryInput<Item>::WeakPtr());
    m_tagInputQueries.removeAll(Domain::LiveQueryInput<Tag>::WeakPtr());
}
