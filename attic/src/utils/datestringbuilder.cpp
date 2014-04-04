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


#include "datestringbuilder.h"

#include <klocale.h>
#include <qcache.h>

//namespace DateStringBuilder {

QString getDayName(const KDateTime &dateTime)
{
    if (!dateTime.isValid() || dateTime.isNull()) {
        return QString();
    }
    QString day;
    if (QDateTime().currentDateTime().date() == dateTime.date()) {
        return i18n("Today" );
    }
    if (QDateTime().currentDateTime().date().addDays(1) == dateTime.date()) {
        return i18n( "Tomorrow" );
    }
    if (QDateTime().currentDateTime().date() == dateTime.date().addDays(1)) {
        return i18n("Yesterday" );
    }
    return dateTime.toString("%A");

}

QString DateStringBuilder::getFullDate(const KDateTime &dateTime)
{
    if (!dateTime.isValid() || dateTime.isNull()) {
        return QString();
    }
    QString date;
    date.append(getDayName(dateTime));
    date.append(", ");
    date.append(dateTime.toString("%e %B %Y"));
    return date;
}

QString DateStringBuilder::getFullDateTime(const KDateTime &dateTime)
{
    if (!dateTime.isValid() || dateTime.isNull()) {
        return QString();
    }
    QString date;
    date.append(getFullDate(dateTime));
    date.append(" ");
    date.append(dateTime.toString("%k:%M:%S"));
    return date;
}

QString DateStringBuilder::getGroupedDate(const KDateTime &dateTime)
{
    if (!dateTime.isValid() || dateTime.isNull()) {
        return QString();
    }
    QDate currentDate = QDateTime::currentDateTime().date();
    if (currentDate.weekNumber() == dateTime.date().weekNumber()) { //this week
        return getDayName(dateTime);
    }
    if (currentDate.addDays(-7).weekNumber() == dateTime.date().weekNumber()) { //last week
        return i18n("Last Week");
    }
    if (currentDate.year() == dateTime.date().year()) { //this year
        return dateTime.toString("%B");
    }
    return dateTime.toString("%B %Y");
}


QString DateStringBuilder::getShortDate(const KDateTime &dateTime)
{
    if (!dateTime.isValid() || dateTime.isNull()) {
        return QString();
    }
    QDate currentDate = QDateTime().currentDateTime().date();
    if (currentDate.weekNumber() == dateTime.date().weekNumber() || currentDate.addDays(1) == dateTime.date()) { //this week or tomorrow (i.e. on sunday)
        return getDayName(dateTime);
    }
    if (currentDate.year() == dateTime.date().year()) { //this year
        //Micro optimization because this function showed up as hotspot
        static QCache<uint, QString> cache;
        uint hash = dateTime.date().month() ^ dateTime.date().day();
        if (!cache.contains(hash)) {
            cache.insert(hash, new QString(dateTime.toString("%d.%m")));
        }
        return *cache[hash];
    }
    return dateTime.toString("%d.%m.%Y");
}



QString DateStringBuilder::getDateString(const KDateTime &dateTime, bool grouped)
{
    if (!dateTime.isValid() || dateTime.isNull()) {
        return QString();
    }
    QString day;
    if (QDateTime().currentDateTime().date() == dateTime.date()) {
        day = i18nc( "today", "Today" );
    }
    if (QDateTime().currentDateTime().date().addDays(1) == dateTime.date()) {
        day = i18nc( "tomorrow", "Tomorrow" );
    }
    if (QDateTime().currentDateTime().date() == dateTime.date().addDays(1)) {
        day = i18nc( "yesterday", "Yesterday" );
    }
    if (!grouped && !day.isEmpty()) {
        return day.append("/t").append(dateTime.toString("%d.%m.%Y"));
    }

    if (!grouped && day.isEmpty()) {
        return dateTime.toString("%:a %d.%m.%Y");
    }

    if (QDateTime().currentDateTime().date().weekNumber() == dateTime.date().weekNumber()) {
        return dateTime.toString("%A");
    }

    //TODO last week

    return dateTime.toString("%B");
    //KGlobal::locale()->formatDate(pimitem->getPrimaryDate().dateTime());
    //return pimitem->getPrimaryDate().dateTime().toString("ddd dd.MM hh:mm");
    //return dateTime.toString("%:a %d.%m.%Y");
}

//}// Namespace