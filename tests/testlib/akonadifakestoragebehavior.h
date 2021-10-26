/*
 * SPDX-FileCopyrightText: 2015 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef TESTLIB_AKONADIFAKESTORAGEBEHAVIOR_H
#define TESTLIB_AKONADIFAKESTORAGEBEHAVIOR_H

#include <QObject>

#include <AkonadiCore/Akonadi/Collection>
#include <AkonadiCore/Akonadi/Item>

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

    void setCreateNextItemError(int errorCode, const QString &errorText);
    int createNextItemErrorCode();
    QString createNextItemErrorText();

    void setDeleteNextItemError(int errorCode, const QString &errorText);
    int deleteNextItemErrorCode();
    QString deleteNextItemErrorText();

    void setUpdateNextItemError(int errorCode, const QString &errorText);
    int updateNextItemErrorCode();
    QString updateNextItemErrorText();

private:
    QHash<Akonadi::Collection::Id, int> m_fetchCollectionsErrorCode;
    QHash<Akonadi::Collection::Id, FetchBehavior> m_fetchCollectionsBehavior;

    QHash<QString, int> m_searchCollectionsErrorCode;
    QHash<QString, FetchBehavior> m_searchCollectionsBehavior;

    QHash<Akonadi::Collection::Id, int> m_fetchItemsErrorCode;
    QHash<Akonadi::Collection::Id, FetchBehavior> m_fetchItemsBehavior;

    QHash<Akonadi::Item::Id, int> m_fetchItemErrorCode;
    QHash<Akonadi::Item::Id, FetchBehavior> m_fetchItemBehavior;

    int m_createNextItemErrorCode = 0;
    QString m_createNextItemErrorText;

    int m_deleteNextItemErrorCode = 0;
    QString m_deleteNextItemErrorText;

    int m_updateNextItemErrorCode = 0;
    QString m_updateNextItemErrorText;
};

}

#endif // TESTLIB_AKONADIFAKESTORAGEBEHAVIOR_H
