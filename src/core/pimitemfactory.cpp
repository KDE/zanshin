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
#include "noteitem.h"
#include "incidenceitem.h"

PimItem::Ptr PimItemFactory::getItem(const Akonadi::Item& item, QObject* parent)
{
    Q_UNUSED(parent);
    if (!item.isValid()) {
        return PimItem::Ptr();
    }
    PimItem::ItemType itemType = PimItem::itemType(item);
    if (itemType & PimItem::Note) {
        return NoteItem::Ptr(new NoteItem(item));
    } else if (itemType & PimItem::Incidence) {
        return IncidenceItem::Ptr(new IncidenceItem(item));
    }
    return PimItem::Ptr();
}
