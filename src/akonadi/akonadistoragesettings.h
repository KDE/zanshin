/* This file is part of Zanshin

   Copyright 2012 Christian Mollekopf <chrigi_1@fastmail.fm>
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

#ifndef AKONADI_STORAGESETTINGS_H
#define AKONADI_STORAGESETTINGS_H

#include <AkonadiCore/Akonadi/Collection>

namespace Akonadi
{

class StorageSettings : public QObject
{
    Q_OBJECT
private:
    StorageSettings();

public:
    static StorageSettings &instance();
    
    Akonadi::Collection defaultCollection();

public slots:
    void setDefaultCollection(const Akonadi::Collection &collection);

signals:
    void defaultCollectionChanged(const Akonadi::Collection &collection);
};

}

#endif // AKONADI_STORAGESETTINGS_H
