/* This file is part of Zanshin Todo.

   Copyright 2011 Christian Mollekopf <chrigi_1@fastmail.fm>

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

#ifndef KJOTS_NOTE_H
#define KJOTS_NOTE_H

#include <QtCore/QString>

#include <kdatetime.h>

#include <Akonadi/Item>
#include <QString>

#include "pimitem.h"

/**
 *
 */
class Note : public PimItem
{
public:
    typedef QSharedPointer<Note> Ptr;
    /**
     * For creating a new note
     */
    Note(QObject *parent = 0);

    /**
     * For acessing existing notes
     */
    Note(const Akonadi::Item&, QObject *parent = 0);

    /**
     * For converting other items into notes
     */
    Note(PimItem&, QObject *parent = 0);

    //~Note();

    QString mimeType();

    KDateTime getPrimaryDate();
    QString getIconName();
    virtual KDateTime getLastModifiedDate();

    virtual bool hasValidPayload();

    /**
     * Returns Note
     */
    ItemType itemType();

    virtual QList< PimItemRelation > getRelations();
    virtual void setRelations(const QList< PimItemRelation >& );

protected:
    KDateTime m_lastModifiedDate;

    virtual ItemStatus getStatus() const;

    void commitData();
    void fetchData();
};


#endif
