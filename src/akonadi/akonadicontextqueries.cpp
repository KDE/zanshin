/* This file is part of Zanshin

   Copyright 2014 Franck Arrecot <franck.arrecot@gmail.com>

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

#include "akonadicontextqueries.h"

#include "akonadicollectionfetchjobinterface.h"
#include "akonadiitemfetchjobinterface.h"
#include "akonaditagfetchjobinterface.h"
#include "akonadimonitorimpl.h"
#include "akonadiserializer.h"
#include "akonadistorage.h"

#include "utils/jobhandler.h"
#include <QByteArray>
#include <QDebug>

using namespace Akonadi;

ContextQueries::ContextQueries()
    : m_storage(new Storage),
      m_serializer(new Serializer),
      m_monitor(new MonitorImpl),
      m_ownInterfaces(true)
{
}

ContextQueries::ContextQueries(StorageInterface *storage, SerializerInterface *serializer, MonitorInterface *monitor)
    : m_storage(storage),
      m_serializer(serializer),
      m_monitor(monitor),
      m_ownInterfaces(false)
{
    connect(m_monitor, SIGNAL(tagAdded(Akonadi::Tag)), this, SLOT(onTagAdded(Akonadi::Tag)));
    connect(m_monitor, SIGNAL(tagRemoved(Akonadi::Tag)), this, SLOT(onTagRemoved(Akonadi::Tag)));
    connect(m_monitor, SIGNAL(tagChanged(Akonadi::Tag)), this, SLOT(onTagChanged(Akonadi::Tag)));
}

ContextQueries::~ContextQueries()
{
}

ContextQueries::ContextResult::Ptr ContextQueries::findAll() const
{
    ContextProvider::Ptr provider(m_contextProvider.toStrongRef());

    if (!provider) {
        provider = ContextProvider::Ptr::create();
        m_contextProvider = provider.toWeakRef();
    }

    auto result = ContextResult::create(provider);
    TagFetchJobInterface *job = m_storage->fetchTags();
    Utils::JobHandler::install(job->kjob(), [provider, job, this] {
        for (Akonadi::Tag tag : job->tags()) {
            auto context = m_serializer->createContextFromTag(tag);
            if (context) {
                provider->append(context);
            }
        }
    });
    return result;
}

ContextQueries::ContextResult::Ptr ContextQueries::findChildren(Domain::Context::Ptr context) const
{
    qFatal("Not implemented yet");
    // TODO : cache system to store context children
    Q_UNUSED(context);
    return ContextResult::Ptr();
    // NOTE : will be done when tag parent support exists
}

ContextQueries::TaskResult::Ptr ContextQueries::findTasks(Domain::Context::Ptr context) const
{
    qFatal("Not implemented yet");
    Q_UNUSED(context);
    return TaskResult::Ptr();
}

void ContextQueries::onTagAdded(const Tag &tag)
{
    ContextProvider::Ptr provider(m_contextProvider.toStrongRef());
    auto context = m_serializer->createContextFromTag(tag);

    if (!context)
        return;

    if (provider) {
        provider->append(context);
    }
}

void ContextQueries::onTagRemoved(const Tag &tag)
{
    if (tag.type() != Akonadi::Serializer::contextTagType())
        return;

    ContextProvider::Ptr provider(m_contextProvider.toStrongRef());

    if (provider) {
        for (int i = 0; i < provider->data().size(); i++) {
            auto context = provider->data().at(i);
            if (m_serializer->isContextTag(context, tag)) {
                provider->removeAt(i);
                i--;
            }
        }
    }
}

void ContextQueries::onTagChanged(const Tag &tag)
{
    if (tag.type() != Akonadi::Serializer::contextTagType())
        return;

    ContextProvider::Ptr provider(m_contextProvider.toStrongRef());

    if (provider) {
        for (int i = 0; i < provider->data().size(); i++) {
            auto context = provider->data().at(i);
            if (m_serializer->isContextTag(context, tag)) {
                m_serializer->updateContextFromTag(context, tag);
                provider->replace(i, context);
            }
        }
    }
}
