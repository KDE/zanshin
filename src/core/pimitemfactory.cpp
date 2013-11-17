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
#include "akonadinoteitem.h"
#include "akonadiincidenceitem.h"

AkonadiBaseItem::Ptr PimItemFactory::getItem(const Akonadi::Item &item)
{
    if (!item.isValid()) {
        return AkonadiBaseItem::Ptr();
    }
    const PimItem::ItemType itemType = AkonadiBaseItem::typeFromItem(item);
    if (itemType == PimItem::Note) {
        return AkonadiNoteItem::Ptr(new AkonadiNoteItem(item));
    } else if (itemType == PimItem::Event
            || itemType == PimItem::Todo
            || itemType == PimItem::Journal) {
        return AkonadiIncidenceItem::Ptr(new AkonadiIncidenceItem(item));
    }
    return AkonadiBaseItem::Ptr();
}
