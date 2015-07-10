/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>

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

#ifndef TESTLIB_AKONADIFAKEJOBS_H
#define TESTLIB_AKONADIFAKEJOBS_H

#include "fakejob.h"
#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadicollectionsearchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"
#include "akonadi/akonaditagfetchjobinterface.h"

namespace Testlib {

class AkonadiFakeCollectionFetchJob : public FakeJob, public Akonadi::CollectionFetchJobInterface
{
    Q_OBJECT
public:
    using FakeJob::FakeJob;

    void setCollections(const Akonadi::Collection::List &collections);
    Akonadi::Collection::List collections() const;

    QString resource() const;
    void setResource(const QString &resource) Q_DECL_OVERRIDE;

    bool filtered() const;
    void setFiltered(bool filter);

private:
    Akonadi::Collection::List m_collections;
    QString m_resource;
    bool m_filter;
};

class AkonadiFakeCollectionSearchJob : public FakeJob, public Akonadi::CollectionSearchJobInterface
{
    Q_OBJECT
public:
    using FakeJob::FakeJob;

    void setCollections(const Akonadi::Collection::List &collections);
    Akonadi::Collection::List collections() const;

private:
    Akonadi::Collection::List m_collections;
};

class AkonadiFakeItemFetchJob : public FakeJob, public Akonadi::ItemFetchJobInterface
{
    Q_OBJECT
public:
    using FakeJob::FakeJob;

    void setItems(const Akonadi::Item::List &items);
    Akonadi::Item::List items() const;

private:
    Akonadi::Item::List m_items;
};

class AkonadiFakeTagFetchJob : public FakeJob, public Akonadi::TagFetchJobInterface
{
    Q_OBJECT
public:
    using FakeJob::FakeJob;

    void setTags(const Akonadi::Tag::List &tags);
    Akonadi::Tag::List tags() const;

private:
    Akonadi::Tag::List m_tags;
};

}

#endif // TESTLIB_AKONADIFAKEJOBS_H
