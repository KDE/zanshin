/* This file is part of Zanshin

   Copyright 2015 Kevin Ottens <ervin@kde.org>

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

#ifndef TESTLIB_AKONADIFAKESTORAGEBEHAVIOR_H
#define TESTLIB_AKONADIFAKESTORAGEBEHAVIOR_H

#include <QObject>

#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <Akonadi/Tag>

namespace Testlib {

class AkonadiFakeStorageBehavior
{
public:
    enum FetchBehavior {
        NormalFetch = 0x0,
        EmptyFetch
    };

    AkonadiFakeStorageBehavior();
    ~AkonadiFakeStorageBehavior();

    void setFetchCollectionsErrorCode(Akonadi::Collection::Id id, int errorCode);
    int fetchCollectionsErrorCode(Akonadi::Collection::Id id) const;
    void setFetchCollectionsBehavior(Akonadi::Collection::Id id, FetchBehavior behavior);
    FetchBehavior fetchCollectionsBehavior(Akonadi::Collection::Id id) const;

    void setSearchCollectionsErrorCode(const QString &name, int errorCode);
    int searchCollectionsErrorCode(const QString &name) const;
    void setSearchCollectionsBehavior(const QString &name, FetchBehavior behavior);
    FetchBehavior searchCollectionsBehavior(const QString &name) const;

    void setFetchItemsErrorCode(Akonadi::Collection::Id id, int errorCode);
    int fetchItemsErrorCode(Akonadi::Collection::Id id) const;
    void setFetchItemsBehavior(Akonadi::Collection::Id id, FetchBehavior behavior);
    FetchBehavior fetchItemsBehavior(Akonadi::Collection::Id id) const;

    void setFetchItemErrorCode(Akonadi::Item::Id id, int errorCode);
    int fetchItemErrorCode(Akonadi::Item::Id id) const;
    void setFetchItemBehavior(Akonadi::Item::Id id, FetchBehavior behavior);
    FetchBehavior fetchItemBehavior(Akonadi::Item::Id id) const;

    void setFetchTagItemsErrorCode(Akonadi::Tag::Id id, int errorCode);
    int fetchTagItemsErrorCode(Akonadi::Tag::Id id) const;
    void setFetchTagItemsBehavior(Akonadi::Tag::Id id, FetchBehavior behavior);
    FetchBehavior fetchTagItemsBehavior(Akonadi::Tag::Id id) const;

    void setFetchTagsErrorCode(int errorCode);
    int fetchTagsErrorCode() const;
    void setFetchTagsBehavior(FetchBehavior behavior);
    FetchBehavior fetchTagsBehavior() const;

private:
    QHash<Akonadi::Collection::Id, int> m_fetchCollectionsErrorCode;
    QHash<Akonadi::Collection::Id, FetchBehavior> m_fetchCollectionsBehavior;

    QHash<QString, int> m_searchCollectionsErrorCode;
    QHash<QString, FetchBehavior> m_searchCollectionsBehavior;

    QHash<Akonadi::Collection::Id, int> m_fetchItemsErrorCode;
    QHash<Akonadi::Collection::Id, FetchBehavior> m_fetchItemsBehavior;

    QHash<Akonadi::Item::Id, int> m_fetchItemErrorCode;
    QHash<Akonadi::Item::Id, FetchBehavior> m_fetchItemBehavior;

    QHash<Akonadi::Tag::Id, int> m_fetchTagItemsErrorCode;
    QHash<Akonadi::Tag::Id, FetchBehavior> m_fetchTagItemsBehavior;

    int m_fetchTagsErrorCode;
    FetchBehavior m_fetchTagsBehavior;
};

}

#endif // TESTLIB_AKONADIFAKESTORAGEBEHAVIOR_H
