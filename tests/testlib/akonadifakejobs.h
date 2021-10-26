/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef TESTLIB_AKONADIFAKEJOBS_H
#define TESTLIB_AKONADIFAKEJOBS_H

#include "fakejob.h"
#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"

namespace Testlib {

// cppcheck seems to get confused by the "using" in this class
// cppcheck-suppress noConstructor
class AkonadiFakeCollectionFetchJob : public FakeJob, public Akonadi::CollectionFetchJobInterface
{
    Q_OBJECT
public:
    using FakeJob::FakeJob;

    void setCollections(const Akonadi::Collection::List &collections);
    Akonadi::Collection::List collections() const override;

    QString resource() const;
    void setResource(const QString &resource) override;

private:
    Akonadi::Collection::List m_collections;
    QString m_resource;
};

class AkonadiFakeItemFetchJob : public FakeJob, public Akonadi::ItemFetchJobInterface
{
    Q_OBJECT
public:
    using FakeJob::FakeJob;

    void setItems(const Akonadi::Item::List &items);
    Akonadi::Item::List items() const override;

    Akonadi::Collection collection() const;
    void setCollection(const Akonadi::Collection &collection) override;

private:
    Akonadi::Item::List m_items;
    Akonadi::Collection m_collection;
};

}

#endif // TESTLIB_AKONADIFAKEJOBS_H
