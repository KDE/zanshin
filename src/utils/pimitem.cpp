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
#include <Nepomuk/Vocabulary/NIE>
#include <Nepomuk/Variant>
#include "queries.h"
#include <Nepomuk/Query/QueryServiceClient>

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
    
    Nepomuk::Resource getResource(const Akonadi::Item &item)
    {
        Q_ASSERT(item.isValid());
        //Since the feeder will add all needed information, this is enough for our needs
        Nepomuk::Resource resource(item.url());
        //We set the minimum required type, in case the feeder didn't create tht item yet, so the thing != resource
        resource.addType( Nepomuk::Vocabulary::NIE::InformationElement() );
        return resource;
    }

    Nepomuk::Thing getThing(const Akonadi::Item &item)
    {
        
        Nepomuk::Query::Query query;
        query.setTerm(MindMirrorQueries::itemThingTerm(item));
        query.setLimit(1);
        
        QList<Nepomuk::Query::Result> results = Nepomuk::Query::QueryServiceClient::syncSparqlQuery(query.toSparqlQuery()); //This is esentially the same as the resourcedata code is doing
        if (results.isEmpty()) {
            /*kDebug() << "createing new thing" << item.url();
            Nepomuk::Resource res = getResource(item);
            Nepomuk::Thing thing = res.pimoThing();

            QScopedPointer<AbstractPimItem> pimitem(PimItemUtils::getItem(item));
            if (!pimitem.isNull()) {
                thing.setLabel(pimitem->getTitle());
            }
            return thing;*/
            //FIXME this creates duplicate things atm (probably due to the not yet ported resource api)
            return Nepomuk::Thing();
        }
        if (results.size() > 1) {
            kWarning() << "more than one Thing found, your db is broken";
        }
        return Nepomuk::Thing(results.first().resource().resourceUri());
    }
    
    Akonadi::Item getItemFromResource(const Nepomuk::Resource &resource)
    {
        //TODO add property to Nepomuk::Resource
        //kDebug() << resource.property(Nepomuk::Vocabulary::NIE::url()).toUrl();
        if (!resource.hasProperty(Nepomuk::Vocabulary::NIE::url())) {
            kWarning() << "url property is missing (did you pass a thing instead of the grounding occurence?)";
            kWarning() << resource.uri();
            return Akonadi::Item();
        }
        Akonadi::Item item = Akonadi::Item::fromUrl(resource.property(Nepomuk::Vocabulary::NIE::url()).toUrl());//sizeof "NotetakerItem:"
        if (item.isValid()) {
            //kDebug() << "found item" << item.url();
            return item;
        }
        kWarning() << "no item found";
        return Akonadi::Item();
    }

    

    Nepomuk::Thing getExistingThing(const Akonadi::Item &item)
    {
        Q_ASSERT(item.isValid());
        //FIXME find a fast replacement for the notesorefilterproxymodel, maybe describe resources
        Nepomuk::Resource res(item.url());
        return res.pimoThing();
    }
}

