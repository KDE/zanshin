/* This file is part of Zanshin

   Copyright 2014 Mario Bensi <mbensi@ipsquad.net>
   Copyright 2014 Kevin Ottens <ervin@kde.org>

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


#ifndef PRESENTATION_QUERYTREEMODEL_H
#define PRESENTATION_QUERYTREEMODEL_H

#include "querytreenode.h"

namespace Presentation {

template<typename ItemType>
class QueryTreeModel : public QueryTreeModelBase
{
public:
    typedef typename QueryTreeNode<ItemType>::QueryGenerator QueryGenerator;
    typedef typename QueryTreeNode<ItemType>::FlagsFunction FlagsFunction;
    typedef typename QueryTreeNode<ItemType>::DataFunction DataFunction;
    typedef typename QueryTreeNode<ItemType>::SetDataFunction SetDataFunction;
    typedef typename QueryTreeNode<ItemType>::DropFunction DropFunction;
    typedef typename QueryTreeNode<ItemType>::DragFunction DragFunction;

    explicit QueryTreeModel(const QueryGenerator &queryGenerator,
                            const FlagsFunction &flagsFunction,
                            const DataFunction &dataFunction,
                            const SetDataFunction &setDataFunction,
                            QObject *parent = 0)
        : QueryTreeModelBase(new QueryTreeNode<ItemType>(ItemType(), 0, this,
                                                         queryGenerator, flagsFunction,
                                                         dataFunction, setDataFunction),
                             parent)
    {
    }

    explicit QueryTreeModel(const QueryGenerator &queryGenerator,
                            const FlagsFunction &flagsFunction,
                            const DataFunction &dataFunction,
                            const SetDataFunction &setDataFunction,
                            const DropFunction &dropFunction,
                            const DragFunction &dragFunction,
                            QObject *parent = 0)
        : QueryTreeModelBase(new QueryTreeNode<ItemType>(ItemType(), 0, this,
                                                         queryGenerator, flagsFunction,
                                                         dataFunction, setDataFunction,
                                                         dropFunction, dragFunction),
                             parent)
    {
    }
};

}

#endif // PRESENTATION_QUERYTREEMODEL_H
