/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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


