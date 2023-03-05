/*
 * SPDX-FileCopyrightText: 2015 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include "akonadifakedataxmlloader.h"
#include "akonadifakedata.h"

#include <QDateTime>
#include <Akonadi/XmlDocument>

#include <KCalendarCore/Todo>

using namespace Testlib;

AkonadiFakeDataXmlLoader::AkonadiFakeDataXmlLoader(AkonadiFakeData *data)
    : m_data(data)
{
}

void AkonadiFakeDataXmlLoader::load(const QString &fileName) const
{
    Akonadi::XmlDocument doc(fileName);
    Q_ASSERT(doc.isValid());

    Akonadi::Collection::Id collectionId = m_data->maxCollectionId() + 1;
    Akonadi::Item::Id itemId = m_data->maxItemId() + 1;

    QHash<QString, Akonadi::Collection> collectionByRid;

    foreach (const Akonadi::Collection &c, doc.collections()) {
        collectionByRid[c.remoteId()] = c;
    }

    std::function<int(const Akonadi::Collection &collection)> depth
            = [&depth, &collectionByRid] (const Akonadi::Collection &c) {
                  if (c.parentCollection().remoteId().isEmpty()) {
                      return 0;
                  }

                  auto parent = collectionByRid.value(c.parentCollection().remoteId());
                  return depth(parent) + 1;
              };

    auto depthLessThan = [&depth] (const Akonadi::Collection &left, const Akonadi::Collection &right) {
        return depth(left) < depth(right);
    };

    auto collectionsByDepth = doc.collections();
    std::sort(collectionsByDepth.begin(), collectionsByDepth.end(), depthLessThan);

    foreach (const Akonadi::Collection &collection, collectionsByDepth) {
        auto c = collection;
        c.setId(collectionId++);
        collectionByRid[c.remoteId()] = c;

        c.setParentCollection(collectionByRid.value(c.parentCollection().remoteId()));
        m_data->createCollection(c);

        foreach (const Akonadi::Item &item, doc.items(collection)) {
            auto i = item;
            i.setId(itemId++);
            i.setParentCollection(c);
            i.setModificationTime(QDateTime::currentDateTime());
            m_data->createItem(i);
        }
    }
}

