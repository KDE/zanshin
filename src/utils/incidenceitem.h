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


#ifndef INCIDENCEITEM_H
#define INCIDENCEITEM_H

#include "abstractpimitem.h"

/**
 * A wrapper class for kcal incidences (todos/events)
 *
 */
class IncidenceItem : public AbstractPimItem
{

public:
    /**
     * Create a new item
     */
    IncidenceItem(AbstractPimItem::ItemType type, QObject *parent = 0);
    /**
     * Access an existing item
     */
    IncidenceItem(const Akonadi::Item&, QObject *parent = 0);
    /**
     * For converting other items into todos/events
     */
    IncidenceItem(AbstractPimItem::ItemType type, AbstractPimItem&, QObject *parent = 0);

    virtual QString mimeType();

    bool hasDueDate() const;
    KDateTime getDueDate();
    void setDueDate(const KDateTime&, bool hasDueDate=true);

    bool hasStartDate() const;
    KDateTime getEventStart();
    void setEventStart(const KDateTime&);

    void setTodoStatus(ItemStatus status);
    AbstractPimItem::ItemStatus getStatus() const;
    
    void setParentTodo(const IncidenceItem &);
    
    KDateTime getPrimaryDate();
    QString getIconName();

    virtual QList< PimItemRelation > getRelations();

    virtual bool hasValidPayload();
    /**
     * Returns Todo/Event/Journal
     */
    ItemType itemType();

protected:
    void commitData();
    void fetchData();
 
};

#endif // INCIDENCEITEM_H
