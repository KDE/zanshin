/*
 * Copyright (C) 2012  Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pimitemfactory.h"
#include "note.h"
#include "incidenceitem.h"
#include <Nepomuk2/Resource>
#include <Nepomuk2/Vocabulary/NIE>
#include <Nepomuk2/Variant>

AbstractPimItem::Ptr PimItemFactory::getItem(const Akonadi::Item& item, QObject* parent)
{
    if (!item.isValid()) {
        return AbstractPimItem::Ptr();
    }
    AbstractPimItem::ItemType itemType = AbstractPimItem::itemType(item);
    if (itemType & AbstractPimItem::Note) {
        return AbstractPimItem::Ptr(new Note(item, parent));
    } else if (itemType & AbstractPimItem::Incidence) {
        return AbstractPimItem::Ptr(new IncidenceItem(item, parent));
    }
    return AbstractPimItem::Ptr();
}

Akonadi::Item PimItemFactory::getItemFromResource(const Nepomuk2::Resource &resource)
{
    //TODO add property to Nepomuk2::Resource
    //kDebug() << resource.property(Nepomuk2::Vocabulary::NIE::url()).toUrl();
    if (!resource.hasProperty(Nepomuk2::Vocabulary::NIE::url())) {
        kWarning() << "url property is missing (did you pass a thing instead of the grounding occurence?)";
        kWarning() << resource.uri();
        return Akonadi::Item();
    }
    Akonadi::Item item = Akonadi::Item::fromUrl(resource.property(Nepomuk2::Vocabulary::NIE::url()).toUrl());
    if (item.isValid()) {
        //kDebug() << "found item" << item.url();
        return item;
    }
    kWarning() << "no item found";
    return Akonadi::Item();
}
