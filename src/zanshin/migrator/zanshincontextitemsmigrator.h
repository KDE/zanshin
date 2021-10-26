/*
 * SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef ZANSHINCONTEXTITEMSMIGRATOR_H
#define ZANSHINCONTEXTITEMSMIGRATOR_H

#include <AkonadiCore/Akonadi/Item>
#include <akonadi/akonadistorage.h>
#include <akonadi/akonadiserializer.h>

enum class WhichItems { TasksToConvert, OnlyContexts, AllTasks };

class ZanshinContextItemsMigrator
{
public:
    ZanshinContextItemsMigrator(bool forceMigration = false);

    struct FetchResult
    {
        Akonadi::Item::List items;
        Akonadi::Collection pickedCollection;
    };
    FetchResult fetchAllItems(WhichItems which);
    Akonadi::Tag::List fetchAllTags();

    void createContexts(const Akonadi::Tag::List &contextTags, const Akonadi::Collection &collection);

    void associateContexts(Akonadi::Item::List &items);

    bool migrateTags();

private:
    Akonadi::Storage m_storage;
    Akonadi::Serializer m_serializer;
    bool m_forceMigration;

    QHash<Akonadi::Tag::Id, QString> m_tagUids;
};

#endif // ZANSHINCONTEXTITEMSMIGRATOR_H
