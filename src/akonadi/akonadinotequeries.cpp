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
#include "akonadimonitorimpl.h"
#include "akonadiserializer.h"
#include "akonadistorage.h"

#include "utils/jobhandler.h"

using namespace Akonadi;

NoteQueries::NoteQueries()
    : m_storage(new Storage),
      m_serializer(new Serializer),
      m_monitor(new MonitorImpl),
      m_ownInterfaces(true)
{
    connect(m_monitor, SIGNAL(itemAdded(Akonadi::Item)), this, SLOT(onItemAdded(Akonadi::Item)));
    connect(m_monitor, SIGNAL(itemRemoved(Akonadi::Item)), this, SLOT(onItemRemoved(Akonadi::Item)));
    connect(m_monitor, SIGNAL(itemChanged(Akonadi::Item)), this, SLOT(onItemChanged(Akonadi::Item)));
}

NoteQueries::NoteQueries(StorageInterface *storage, SerializerInterface *serializer, MonitorInterface *monitor)
    : m_storage(storage),
      m_serializer(serializer),
      m_monitor(monitor),
      m_ownInterfaces(false)
{
    connect(monitor, SIGNAL(itemAdded(Akonadi::Item)), this, SLOT(onItemAdded(Akonadi::Item)));
    connect(monitor, SIGNAL(itemRemoved(Akonadi::Item)), this, SLOT(onItemRemoved(Akonadi::Item)));
    connect(monitor, SIGNAL(itemChanged(Akonadi::Item)), this, SLOT(onItemChanged(Akonadi::Item)));
}

NoteQueries::~NoteQueries()
{
    if (m_ownInterfaces) {
        delete m_storage;
        delete m_serializer;
        delete m_monitor;
    }
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

        m_findAll->setConvertFunction([this] (const Akonadi::Item &item) {
            return m_serializer->createNoteFromItem(item);
        });
        m_findAll->setUpdateFunction([this] (const Akonadi::Item &item, Domain::Note::Ptr &note) {
            m_serializer->updateNoteFromItem(note, item);
        });
        m_findAll->setPredicateFunction([this] (const Akonadi::Item &item) {
            return m_serializer->isNoteItem(item);
        });
        m_findAll->setRepresentsFunction([this] (const Akonadi::Item &item, const Domain::Note::Ptr &note) {
            return m_serializer->representsItem(note, item);
        });
    }

    return m_findAll->result();
}

NoteQueries::TopicResult::Ptr NoteQueries::findTopics(Domain::Note::Ptr note) const
{
    Q_UNUSED(note);
    qFatal("Not implemented yet");
    return TopicResult::Ptr();
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
    m_noteQueries << query;
    return query;
}
