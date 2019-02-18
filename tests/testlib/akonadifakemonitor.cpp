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

#include "akonadifakemonitor.h"

using namespace Testlib;

AkonadiFakeMonitor::AkonadiFakeMonitor(QObject *parent)
    : Akonadi::MonitorInterface(parent)
{
}

void AkonadiFakeMonitor::addCollection(const Akonadi::Collection &collection)
{
    emit collectionAdded(collection);
}

void AkonadiFakeMonitor::removeCollection(const Akonadi::Collection &collection)
{
    emit collectionRemoved(collection);
}

void AkonadiFakeMonitor::changeCollection(const Akonadi::Collection &collection)
{
    emit collectionChanged(collection);
}

void AkonadiFakeMonitor::changeCollectionSelection(const Akonadi::Collection &collection)
{
    emit collectionSelectionChanged(collection);
}

void AkonadiFakeMonitor::addItem(const Akonadi::Item &item)
{
    emit itemAdded(item);
}

void AkonadiFakeMonitor::removeItem(const Akonadi::Item &item)
{
    emit itemRemoved(item);
}

void AkonadiFakeMonitor::changeItem(const Akonadi::Item &item)
{
    emit itemChanged(item);
}

void AkonadiFakeMonitor::moveItem(const Akonadi::Item &item)
{
    emit itemMoved(item);
}


