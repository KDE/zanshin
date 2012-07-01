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
#include "pimitem.h"

#include "note.h"
#include "incidenceitem.h"
#include <Nepomuk2/Vocabulary/NIE>
#include <Nepomuk2/Variant>
#include "queries.h"
#include <Nepomuk2/Query/QueryServiceClient>

namespace PimItemUtils {

    //use together with QScopedPointer
    AbstractPimItem* getItem(const Akonadi::Item &item, QObject *parent)
    {
        if (!item.isValid()) {
            kWarning() << "invalid item";
            return 0;
        }
        AbstractPimItem::ItemType itemType = AbstractPimItem::itemType(item);
        if (itemType & AbstractPimItem::Note) {
            return  new Note(item, parent);
        } else if (itemType & AbstractPimItem::Incidence) {
            return new IncidenceItem(item, parent);
        }
        return 0;
    };

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
/*
    static Akonadi::Item getAkonadiItem(const QModelIndex &index)
    {
        const Akonadi::Item item = index0.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
        if (!item.isValid()) {
            kDebug() << "invalid item";
        }
        return item;
    };
*/

    AbstractPimItem* fromUrl( const KUrl &url )
    {
        return getItem(Akonadi::Item::fromUrl(url));
    };


    Akonadi::Item getItemFromResource(const Nepomuk2::Resource &resource)
    {
        //TODO add property to Nepomuk2::Resource
        //kDebug() << resource.property(Nepomuk2::Vocabulary::NIE::url()).toUrl();
        if (!resource.hasProperty(Nepomuk2::Vocabulary::NIE::url())) {
            kWarning() << "url property is missing (did you pass a thing instead of the grounding occurence?)";
            kWarning() << resource.resourceUri();
            return Akonadi::Item();
        }
        Akonadi::Item item = Akonadi::Item::fromUrl(resource.property(Nepomuk2::Vocabulary::NIE::url()).toUrl());//sizeof "NotetakerItem:"
        if (item.isValid()) {
            //kDebug() << "found item" << item.url();
            return item;
        }
        kWarning() << "no item found";
        return Akonadi::Item();
    }

}

