/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
   Copyright 2014 Remi Benoit <r3m1.benoit@gmail.com>

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

#ifndef AKONADI_NOTEQUERIES_H
#define AKONADI_NOTEQUERIES_H

#include <QObject>

#include "domain/notequeries.h"

namespace Akonadi {

class Item;
class MonitorInterface;
class SerializerInterface;
class StorageInterface;

class NoteQueries : public QObject, public Domain::NoteQueries
{
    Q_OBJECT
public:
    typedef Domain::QueryResultProvider<Domain::Note::Ptr> NoteProvider;
    typedef Domain::QueryResult<Domain::Note::Ptr> NoteResult;

    typedef Domain::QueryResultProvider<Domain::Topic::Ptr> TopicProvider;
    typedef Domain::QueryResult<Domain::Topic::Ptr> TopicResult;

    NoteQueries();
    NoteQueries(StorageInterface *storage, SerializerInterface *serializer, MonitorInterface *monitor);
    virtual ~NoteQueries();

    NoteResult::Ptr findAll() const;
    TopicResult::Ptr findTopics(Domain::Note::Ptr note) const;

private slots:
    void onItemAdded(const Akonadi::Item &item);
    void onItemRemoved(const Akonadi::Item &item);
    void onItemChanged(const Akonadi::Item &item);

private:
    bool isNoteItem(const Domain::Note::Ptr &note, const Item &item) const;
    Domain::Note::Ptr deserializeNote(const Item &item) const;

    StorageInterface *m_storage;
    SerializerInterface *m_serializer;
    MonitorInterface *m_monitor;
    bool m_ownInterfaces;

    mutable NoteProvider::WeakPtr m_noteProvider;
};

}

#endif // AKONADI_NOTEQUERIES_H
