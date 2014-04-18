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

#ifndef ZANSHIN_TESTLIB_AKONADIMOCKS_H
#define ZANSHIN_TESTLIB_AKONADIMOCKS_H

#include <KJob>

#include "akonadi/akonadimonitorinterface.h"
#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"

class MockAkonadiJob : public KJob
{
    Q_OBJECT
public:
    explicit MockAkonadiJob(QObject *parent = 0);

    void setExpectedError(int errorCode);

private:
    void start();

private slots:
    void onTimeout();

protected:
    bool isDone() const;

private:
    bool m_done;
    bool m_launched;
    int m_errorCode;
};

class MockCollectionFetchJob : public MockAkonadiJob, public Akonadi::CollectionFetchJobInterface
{
    Q_OBJECT
public:
    using MockAkonadiJob::MockAkonadiJob;

    void setCollections(const Akonadi::Collection::List &collections);
    Akonadi::Collection::List collections() const;

private:
    Akonadi::Collection::List m_collections;
};

class MockItemFetchJob : public MockAkonadiJob, public Akonadi::ItemFetchJobInterface
{
    Q_OBJECT
public:
    using MockAkonadiJob::MockAkonadiJob;

    void setItems(const Akonadi::Item::List &items);
    Akonadi::Item::List items() const;

private:
    Akonadi::Item::List m_items;
};

class MockMonitor : public Akonadi::MonitorInterface
{
    Q_OBJECT
public:
    explicit MockMonitor(QObject *parent = 0);

    void addItem(const Akonadi::Item &item);
    void removeItem(const Akonadi::Item &item);
    void changeItem(const Akonadi::Item &item);
};

#endif // ZANSHIN_TESTLIB_AKONADIMOCKS_H
