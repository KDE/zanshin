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

#include "akonadinotequeries.h"

#include "akonadicollectionfetchjobinterface.h"
#include "akonadiitemfetchjobinterface.h"

#include "utils/jobhandler.h"

#include <functional>

using namespace std::placeholders;

using namespace Akonadi;

NoteQueries::NoteQueries(const StorageInterface::Ptr &storage,
                         const SerializerInterface::Ptr &serializer,
                         const MonitorInterface::Ptr &monitor)
    : m_storage(storage),
      m_serializer(serializer),
      m_monitor(monitor)
{
    connect(m_monitor.data(), SIGNAL(itemAdded(Akonadi::Item)), this, SLOT(onItemAdded(Akonadi::Item)));
    connect(m_monitor.data(), SIGNAL(itemRemoved(Akonadi::Item)), this, SLOT(onItemRemoved(Akonadi::Item)));
    connect(m_monitor.data(), SIGNAL(itemChanged(Akonadi::Item)), this, SLOT(onItemChanged(Akonadi::Item)));
}

NoteQueries::NoteResult::Ptr NoteQueries::findAll() const
{
    if (!m_findAll) {
        {
            NoteQueries *self = const_cast<NoteQueries*>(this);
            self->m_findAll = self->createNoteQuery();
        }

        m_findAll->setFetchFunction([this] (const NoteQuery::AddFunction &add) {
            CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(),
                                                                           StorageInterface::Recursive,
                                                                           StorageInterface::Notes);
            Utils::JobHandler::install(job->kjob(), [this, job, add] {
                if (job->kjob()->error() != KJob::NoError)
                    return;

                for (auto collection : job->collections()) {
                    ItemFetchJobInterface *job = m_storage->fetchItems(collection);
                    Utils::JobHandler::install(job->kjob(), [this, job, add] {
                        if (job->kjob()->error() != KJob::NoError)
                            return;

                        for (auto item : job->items()) {
                            add(item);
                        }
                    });
                }
            });
        });
        m_findAll->setPredicateFunction([this] (const Akonadi::Item &item) {
            return m_serializer->isNoteItem(item);
        });
    }

    return m_findAll->result();
}

void NoteQueries::onItemAdded(const Item &item)
{
    foreach (const NoteQuery::Ptr &query, m_noteQueries)
        query->onAdded(item);
}

void NoteQueries::onItemRemoved(const Item &item)
{
    foreach (const NoteQuery::Ptr &query, m_noteQueries)
        query->onRemoved(item);
}

void NoteQueries::onItemChanged(const Item &item)
{
    foreach (const NoteQuery::Ptr &query, m_noteQueries)
        query->onChanged(item);
}

NoteQueries::NoteQuery::Ptr NoteQueries::createNoteQuery()
{
    auto query = NoteQueries::NoteQuery::Ptr::create();

    query->setConvertFunction(std::bind(&SerializerInterface::createNoteFromItem, m_serializer, _1));
    query->setUpdateFunction(std::bind(&SerializerInterface::updateNoteFromItem, m_serializer, _2, _1));
    query->setRepresentsFunction(std::bind(&SerializerInterface::representsItem, m_serializer, _2, _1));

    m_noteQueries << query;
    return query;
}
