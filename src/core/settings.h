/* This file is part of Zanshin Todo.

   Copyright 2012 Christian Mollekopf <chrigi_1@fastmail.fm>

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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H
#include <Akonadi/Collection>

class Settings: public QObject
{
    Q_OBJECT
private:
    Settings();
    Settings(const Settings &);
public:
    static Settings &instance() {
        static Settings i;
        return i;
    }
    
    void setDefaultTodoCollection(const Akonadi::Collection &collection);
    Akonadi::Collection defaultTodoCollection();

    void setDefaultNoteCollection(const Akonadi::Collection &collection);
    Akonadi::Collection defaultNoteCollection();
    
    void setActiveCollections(const QSet<Akonadi::Collection::Id> &);
    QSet<Akonadi::Collection::Id> activeCollections();
signals:
    void defaultTodoCollectionChanged(Akonadi::Collection);
    void defaultNoteCollectionChanged(Akonadi::Collection);
    void activeCollectionsChanged(QSet<Akonadi::Collection::Id>);
};

#endif // CONFIGURATION_H
