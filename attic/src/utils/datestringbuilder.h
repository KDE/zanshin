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


#ifndef DATESTRINGBUILDER_H
#define DATESTRINGBUILDER_H

#include <QString>
#include <KDateTime>

/**
 * Format date to string
 *
 *
 *
 *
 *
 **/
namespace DateStringBuilder
{

    /**
     *
     * For sections:
     *
     * Tomorrow
     * Today
     * Yesterday
     * Tuesday
     * Monday
     * Last Week
     * Oktober
     * September
     * Dezember 2009
     */
    QString getGroupedDate(const KDateTime &);
    /**
     * * For table:
     *
     * Tomorrow
     * Today
     * Yesterday
     * Tuesday
     * Monday
     * Sun 11.05
     * Mon 11.03
     * 17.08.2008
     * */
    QString getShortDate(const KDateTime &);
    /**
    * For itemview:
    *
    * Today 17 November 2009
    */
    QString getFullDate(const KDateTime &);
    /**
     * for tooltip:
     * Full date + time 
     */
    QString getFullDateTime(const KDateTime &);
    
    QString getDateString(const KDateTime &, bool grouped = false);

    
};

#endif // DATESTRINGBUILDER_H
