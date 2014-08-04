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
    NoteProvider::Ptr provider(m_noteProvider.toStrongRef());

    if (!provider) {
        provider = NoteProvider::Ptr(new NoteProvider);
        m_noteProvider = provider.toWeakRef();
    }

    auto result = NoteResult::create(provider);

    CollectionFetchJobInterface *job = m_storage->fetchCollections(Akonadi::Collection::root(),
                                                                   StorageInterface::Recursive,
                                                                   StorageInterface::Notes);
    Utils::JobHandler::install(job->kjob(), [provider, job, this] {
        for (auto collection : job->collections()) {
            ItemFetchJobInterface *job = m_storage->fetchItems(collection);
            Utils::JobHandler::install(job->kjob(), [provider, job, this] {
                for (auto item : job->items()) {
                    auto note = deserializeNote(item);
                    if (note)
                        provider->append(note);
                }
            });
        }
    });

    return result;
}

NoteQueries::TopicResult::Ptr NoteQueries::findTopics(Domain::Note::Ptr note) const
{
    Q_UNUSED(note);
    qFatal("Not implemented yet");
    return TopicResult::Ptr();
}

void NoteQueries::onItemAdded(const Item &item)
{
    NoteProvider::Ptr provider(m_noteProvider.toStrongRef());

    if (provider) {
        auto note = deserializeNote(item);
        if (note)
            provider->append(note);
    }
}

void NoteQueries::onItemRemoved(const Item &item)
{
    NoteProvider::Ptr provider(m_noteProvider.toStrongRef());

    if (provider) {
        for (int i = 0; i < provider->data().size(); i++) {
            auto note = provider->data().at(i);
            if (m_serializer->represents(note, item)) {
                provider->removeAt(i);
                i--;
            }
        }
    }
}

void NoteQueries::onItemChanged(const Item &item)
{
    NoteProvider::Ptr provider(m_noteProvider.toStrongRef());

    if (provider) {
        for (int i = 0; i < provider->data().size(); i++) {
            auto note = provider->data().at(i);
            if (m_serializer->represents(note, item)) {
                m_serializer->updateNoteFromItem(note, item);
                provider->replace(i, note);
            }
        }
    }
}

Domain::Note::Ptr NoteQueries::deserializeNote(const Item &item) const
{
    return m_serializer->createNoteFromItem(item);
}
