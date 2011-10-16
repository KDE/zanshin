/*
    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef KJOTS_NOTE_H
#define KJOTS_NOTE_H

#include <QtCore/QString>

#include <kdatetime.h>

#include <Akonadi/Item>
#include <QString>

#include "abstractpimitem.h"

/**
 *
 */
class Note : public AbstractPimItem
{
public:
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
    Note(AbstractPimItem&, QObject *parent = 0);

    //~Note();

    QString mimeType();

    KDateTime getPrimaryDate();
    QString getIconName();

    virtual bool hasValidPayload();

    /**
     * Returns Note
     */
    ItemType itemType();

protected:
    /*QString m_text;
    QString m_title;
    KDateTime m_creationDate;*/

    virtual ItemStatus getStatus() const;

    void commitData();
    void fetchData();
};


#endif
