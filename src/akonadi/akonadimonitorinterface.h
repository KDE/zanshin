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

#ifndef AKONADI_MONITORINTERFACE_H
#define AKONADI_MONITORINTERFACE_H

#include <QObject>
#include <QSharedPointer>

namespace Akonadi {

class Collection;
class Item;
class Tag;

class MonitorInterface : public QObject
{
    Q_OBJECT
public:
    typedef QSharedPointer<MonitorInterface> Ptr;

    explicit MonitorInterface(QObject *parent = 0);
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

    void tagAdded(const Akonadi::Tag &tag);
    void tagRemoved(const Akonadi::Tag &tag);
    void tagChanged(const Akonadi::Tag &tag);
};

}

#endif // AKONADI_MONITORINTERFACE_H
