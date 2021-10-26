/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef AKONADI_MONITORINTERFACE_H
#define AKONADI_MONITORINTERFACE_H

#include <QObject>
#include <QSharedPointer>

namespace Akonadi {

class Collection;
class Item;

class MonitorInterface : public QObject
{
    Q_OBJECT
public:
    typedef QSharedPointer<MonitorInterface> Ptr;

    explicit MonitorInterface(QObject *parent = nullptr);
    virtual ~MonitorInterface();

signals:
    void collectionAdded(const Akonadi::Collection &collection);
    void collectionRemoved(const Akonadi::Collection &collection);
    void collectionChanged(const Akonadi::Collection &collection);
    void collectionSelectionChanged(const Akonadi::Collection &collection);

    void itemAdded(const Akonadi::Item &item);
    void itemRemoved(const Akonadi::Item &item);
    void itemChanged(const Akonadi::Item &items);
    void itemMoved(const Akonadi::Item &item);
};

}

#endif // AKONADI_MONITORINTERFACE_H
