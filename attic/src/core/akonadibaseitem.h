/* This file is part of Zanshin Todo.

   Copyright 2011 Christian Mollekopf <chrigi_1@fastmail.fm>
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

#ifndef AKONADIBASEITEM_H
#define AKONADIBASEITEM_H

#include <Akonadi/Item>

#include "pimitem.h"

class AkonadiBaseItem : public PimItem
{
public:
    typedef QSharedPointer<AkonadiBaseItem> Ptr;

    AkonadiBaseItem();
    AkonadiBaseItem(const Akonadi::Item &item);

    //based on item mimetype of item
    static ItemType typeFromItem(const Akonadi::Item &item);

    const Akonadi::Item &getItem() const;

    QString title() const;

    KDateTime date(DateRole role) const;
    bool setDate(DateRole role, const KDateTime &date);


    KJob *saveItem();

protected:
    friend class PimItemMonitor;
    virtual void setItem(const Akonadi::Item &);
    Akonadi::Item m_item;
};

Q_DECLARE_METATYPE(AkonadiBaseItem::Ptr)

#endif
