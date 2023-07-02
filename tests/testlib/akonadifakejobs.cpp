/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include "akonadifakejobs.h"

#include <KCalendarCore/Todo>

#include <QTimer>

using namespace Testlib;

void AkonadiFakeCollectionFetchJob::setCollections(const Akonadi::Collection::List &collections)
{
    m_collections = collections;
}

Akonadi::Collection::List AkonadiFakeCollectionFetchJob::collections() const
{
    auto result = isDone() ? m_collections : Akonadi::Collection::List();

    if (m_resource.isEmpty()) {
        auto it = std::remove_if(result.begin(), result.end(),
                                 [] (const Akonadi::Collection &col) {
                                     const auto mime = col.contentMimeTypes();
                                     return !mime.contains(KCalendarCore::Todo::todoMimeType());
                                 });
        result.erase(it, std::end(result));
    }

    return result;
}

QString AkonadiFakeCollectionFetchJob::resource() const
{
    return m_resource;
}

void AkonadiFakeCollectionFetchJob::setResource(const QString &resource)
{
    m_resource = resource;
}

void AkonadiFakeItemFetchJob::setItems(const Akonadi::Item::List &items)
{
    m_items = items;
}

Akonadi::Item::List AkonadiFakeItemFetchJob::items() const
{
    if (expectedError() != KJob::NoError)
        return m_items;

    return isDone() ? m_items : Akonadi::Item::List();
}

Akonadi::Collection AkonadiFakeItemFetchJob::collection() const
{
    return m_collection;
}

void AkonadiFakeItemFetchJob::setCollection(const Akonadi::Collection &collection)
{
    m_collection = collection;
}


#include "moc_akonadifakejobs.cpp"
