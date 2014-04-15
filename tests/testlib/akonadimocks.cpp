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

#include "akonadimocks.h"

#include <QTimer>

MockAkonadiJob::MockAkonadiJob(QObject *parent)
    : KJob(parent), m_done(false), m_launched(false)
{
}

void MockAkonadiJob::start()
{
    if (!m_launched) {
        m_launched = true;
        QTimer::singleShot(50, this, SLOT(onTimeout()));
    }
}

void MockAkonadiJob::onTimeout()
{
    m_done = true;
    emitResult();
}

bool MockAkonadiJob::isDone() const
{
    return m_done;
}


void MockCollectionFetchJob::setCollections(const Akonadi::Collection::List &collections)
{
    m_collections = collections;
}

Akonadi::Collection::List MockCollectionFetchJob::collections() const
{
    return isDone() ? m_collections : Akonadi::Collection::List();
}


void MockItemFetchJob::setItems(const Akonadi::Item::List &items)
{
    m_items = items;
}

Akonadi::Item::List MockItemFetchJob::items() const
{
    return isDone() ? m_items : Akonadi::Item::List();
}


MockMonitor::MockMonitor(QObject *parent)
    : Akonadi::MonitorInterface(parent)
{
}

void MockMonitor::addItem(const Akonadi::Item &item)
{
    emit itemAdded(item);
}

void MockMonitor::removeItem(const Akonadi::Item &item)
{
    emit itemRemoved(item);
}

void MockMonitor::changeItem(const Akonadi::Item &item)
{
    emit itemChanged(item);
}
