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

#ifndef AKONADI_NOTEQUERIES_H
#define AKONADI_NOTEQUERIES_H

#include "domain/notequeries.h"

#include "akonadi/akonadilivequeryhelpers.h"
#include "akonadi/akonadilivequeryintegrator.h"

namespace Akonadi {

class NoteQueries : public Domain::NoteQueries
{
public:
    typedef QSharedPointer<NoteQueries> Ptr;

    typedef Domain::LiveQueryInput<Akonadi::Item> ItemInputQuery;
    typedef Domain::LiveQueryOutput<Domain::Note::Ptr> NoteQueryOutput;
    typedef Domain::QueryResultProvider<Domain::Note::Ptr> NoteProvider;
    typedef Domain::QueryResult<Domain::Note::Ptr> NoteResult;

    NoteQueries(const StorageInterface::Ptr &storage,
                const SerializerInterface::Ptr &serializer,
                const MonitorInterface::Ptr &monitor);

    NoteResult::Ptr findAll() const Q_DECL_OVERRIDE;
    NoteResult::Ptr findInbox() const Q_DECL_OVERRIDE;

private:
    SerializerInterface::Ptr m_serializer;
    LiveQueryHelpers::Ptr m_helpers;
    LiveQueryIntegrator::Ptr m_integrator;

    mutable NoteQueryOutput::Ptr m_findAll;
};

}

#endif // AKONADI_NOTEQUERIES_H
