/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) <year>  <name of author>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
