/* This file is part of Zanshin Todo.

   Copyright 2011 Kevin Ottens <ervin@kde.org>

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

#include "aboutdata.h"

KAboutData Zanshin::getAboutData()
{
    KAboutData about("zanshin", "zanshin",
                     ki18n("Zanshin Todo"), "0.2.1",
                     ki18n("A Getting Things Done application which aims at getting your mind like water"),
                     KAboutData::License_GPL_V3,
                     ki18n("Copyright 2008-2010, Kevin Ottens <ervin@kde.org>"));

    about.addAuthor(ki18n("Kevin Ottens"),
                    ki18n("Lead Developer"),
                    "ervin@kde.org");

    about.addAuthor(ki18n("Mario Bensi"),
                    ki18n("Developer"),
                    "nef@ipsquad.net");

    return about;
}

