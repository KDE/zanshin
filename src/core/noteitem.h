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

#ifndef NOTEITEM_H
#define NOTEITEM_H

#include <QtCore/QString>

#include <kdatetime.h>

#include <Akonadi/Item>
#include <akonadi/notes/noteutils.h>
#include <QString>

#include "pimitem.h"

class NoteItem : public PimItem
{
public:
    typedef QSharedPointer<NoteItem> Ptr;
    /**
     * For creating a new note
     */
    NoteItem();

    /**
     * For acessing existing notes
     */
    NoteItem(const Akonadi::Item&);

    QString mimeType();
    PimItemIndex::ItemType itemType();
    virtual bool hasValidPayload();
    virtual ItemStatus getStatus() const;

    virtual QString getUid();
    virtual void setText(const QString &, bool isRich = false);
    virtual QString getText();
    virtual void setTitle(const QString &, bool isRich = false);
    virtual QString getTitle();
    virtual void setCreationDate(const KDateTime &);
    virtual KDateTime getCreationDate();
    virtual QString getIconName();
    KDateTime getPrimaryDate();
    virtual KDateTime getLastModifiedDate();

    virtual QList< PimItemRelation > getRelations();
    virtual void setRelations(const QList< PimItemRelation >& );

private:
    virtual void setItem(const Akonadi::Item& );
    void commitData();
    QSharedPointer<Akonadi::NoteUtils::NoteMessageWrapper> messageWrapper;
};

#endif
