/*
 * SPDX-FileCopyrightText: 2015 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include "akonadifakestoragebehavior.h"

using namespace Testlib;

AkonadiFakeStorageBehavior::AkonadiFakeStorageBehavior()
{
}

AkonadiFakeStorageBehavior::~AkonadiFakeStorageBehavior()
{
}

void AkonadiFakeStorageBehavior::setFetchCollectionsErrorCode(Akonadi::Collection::Id id, int errorCode)
{
    m_fetchCollectionsErrorCode[id] = errorCode;
}

int AkonadiFakeStorageBehavior::fetchCollectionsErrorCode(Akonadi::Collection::Id id) const
{
    return m_fetchCollectionsErrorCode.value(id, KJob::NoError);
}

void AkonadiFakeStorageBehavior::setFetchCollectionsBehavior(Akonadi::Collection::Id id, FetchBehavior behavior)
{
    m_fetchCollectionsBehavior[id] = behavior;
}

AkonadiFakeStorageBehavior::FetchBehavior AkonadiFakeStorageBehavior::fetchCollectionsBehavior(Akonadi::Collection::Id id) const
{
    return m_fetchCollectionsBehavior.value(id, NormalFetch);
}

void AkonadiFakeStorageBehavior::setSearchCollectionsErrorCode(const QString &name, int errorCode)
{
    m_searchCollectionsErrorCode[name] = errorCode;
}

int AkonadiFakeStorageBehavior::searchCollectionsErrorCode(const QString &name) const
{
    return m_searchCollectionsErrorCode.value(name, KJob::NoError);
}

void AkonadiFakeStorageBehavior::setSearchCollectionsBehavior(const QString &name, AkonadiFakeStorageBehavior::FetchBehavior behavior)
{
    m_searchCollectionsBehavior[name] = behavior;
}

AkonadiFakeStorageBehavior::FetchBehavior AkonadiFakeStorageBehavior::searchCollectionsBehavior(const QString &name) const
{
    return m_searchCollectionsBehavior.value(name, NormalFetch);
}

void AkonadiFakeStorageBehavior::setFetchItemsErrorCode(Akonadi::Collection::Id id, int errorCode)
{
    m_fetchItemsErrorCode[id] = errorCode;
}

int AkonadiFakeStorageBehavior::fetchItemsErrorCode(Akonadi::Collection::Id id) const
{
    return m_fetchItemsErrorCode.value(id, KJob::NoError);
}

void AkonadiFakeStorageBehavior::setFetchItemsBehavior(Akonadi::Collection::Id id, FetchBehavior behavior)
{
    m_fetchItemsBehavior[id] = behavior;
}

AkonadiFakeStorageBehavior::FetchBehavior AkonadiFakeStorageBehavior::fetchItemsBehavior(Akonadi::Collection::Id id) const
{
    return m_fetchItemsBehavior.value(id, NormalFetch);
}

void AkonadiFakeStorageBehavior::setFetchItemErrorCode(Akonadi::Item::Id id, int errorCode)
{
    m_fetchItemErrorCode[id] = errorCode;
}

int AkonadiFakeStorageBehavior::fetchItemErrorCode(Akonadi::Item::Id id) const
{
    return m_fetchItemErrorCode.value(id, KJob::NoError);
}

void AkonadiFakeStorageBehavior::setFetchItemBehavior(Akonadi::Item::Id id, FetchBehavior behavior)
{
    m_fetchItemBehavior[id] = behavior;
}

AkonadiFakeStorageBehavior::FetchBehavior AkonadiFakeStorageBehavior::fetchItemBehavior(Akonadi::Item::Id id) const
{
    return m_fetchItemBehavior.value(id, NormalFetch);
}

void AkonadiFakeStorageBehavior::setCreateNextItemError(int errorCode, const QString &errorText)
{
    m_createNextItemErrorCode = errorCode;
    m_createNextItemErrorText = errorText;
}

int AkonadiFakeStorageBehavior::createNextItemErrorCode()
{
    return std::exchange(m_createNextItemErrorCode, KJob::NoError);
}

QString AkonadiFakeStorageBehavior::createNextItemErrorText()
{
    return std::exchange(m_createNextItemErrorText, QString());
}

void AkonadiFakeStorageBehavior::setDeleteNextItemError(int errorCode, const QString &errorText)
{
    m_deleteNextItemErrorCode = errorCode;
    m_deleteNextItemErrorText = errorText;
}

int AkonadiFakeStorageBehavior::deleteNextItemErrorCode()
{
    return std::exchange(m_deleteNextItemErrorCode, KJob::NoError);
}

QString AkonadiFakeStorageBehavior::deleteNextItemErrorText()
{
    return std::exchange(m_deleteNextItemErrorText, QString());
}

void AkonadiFakeStorageBehavior::setUpdateNextItemError(int errorCode, const QString &errorText)
{
    m_updateNextItemErrorCode = errorCode;
    m_updateNextItemErrorText = errorText;
}

int AkonadiFakeStorageBehavior::updateNextItemErrorCode()
{
    return std::exchange(m_updateNextItemErrorCode, KJob::NoError);
}

QString AkonadiFakeStorageBehavior::updateNextItemErrorText()
{
    return std::exchange(m_updateNextItemErrorText, QString());
}
