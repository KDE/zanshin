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
#ifndef PIMITEM_H
#define PIMITEM_H

#include "core/abstractpimitem.h"

namespace Nepomuk2 {
class Resource;
}

namespace PimItemUtils {

    //use together with QScopedPointer
    AbstractPimItem* getItem(const Akonadi::Item &item, QObject *parent = 0);

    /*typedef QSharedPointer<AbstractPimItem> AbstractPimItemPtr;
    AbstractPimItemPtr getItem(const Akonadi::Item &item, QObject *parent = 0)
    {
        //Q_ASSERT(item.isValid());
        if (!item.isValid()) {
            kWarning() << "invalid item";
            return AbstractPimItemPtr();
        }
        AbstractPimItem::ItemType itemType = AbstractPimItem::itemType(item);
        if (itemType & AbstractPimItem::Note) {
            return  AbstractPimItemPtr(new Note(item, parent));
        } else if (itemType & AbstractPimItem::Incidence) {
            return AbstractPimItemPtr(new IncidenceItem(item, parent));
        }
        //Q_ASSERT(0);
        return AbstractPimItemPtr();
    };*/

    AbstractPimItem* fromUrl( const KUrl &url );
    
    Akonadi::Item getItemFromResource(const Nepomuk2::Resource &resource);
    void moveToProject(Akonadi::Item &item, const QString &parentUid, bool linkOnly = false);
}

#endif
