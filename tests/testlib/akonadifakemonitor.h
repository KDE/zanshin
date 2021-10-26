/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef TESTLIB_AKONADIFAKEMONITOR_H
#define TESTLIB_AKONADIFAKEMONITOR_H

#include "akonadi/akonadimonitorinterface.h"

namespace Testlib {

class AkonadiFakeMonitor : public Akonadi::MonitorInterface
{
    Q_OBJECT
public:
    typedef QSharedPointer<AkonadiFakeMonitor> Ptr;

    explicit AkonadiFakeMonitor(QObject *parent = nullptr);

public slots:
    void addCollection(const Akonadi::Collection &collection);
    void removeCollection(const Akonadi::Collection &collection);
    void changeCollection(const Akonadi::Collection &collection);
    void changeCollectionSelection(const Akonadi::Collection &collection);

    void addItem(const Akonadi::Item &item);
    void removeItem(const Akonadi::Item &item);
    void changeItem(const Akonadi::Item &item);
    void moveItem(const Akonadi::Item &item);
};

}

#endif // TESTLIB_AKONADIFAKEMONITOR_H
