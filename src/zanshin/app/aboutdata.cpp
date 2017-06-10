/* This file is part of Zanshin

   Copyright 2011-2014 Kevin Ottens <ervin@kde.org>

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
#include "../../appversion.h"
#include <QObject>

KAboutData App::getAboutData()
{
    KAboutData about(QStringLiteral("zanshin"),
                     QObject::tr("Zanshin Tasks"), QStringLiteral(APPLICATION_VERSION),
                     QObject::tr("A Getting Things Done application which aims at getting your mind like water"),
                     KAboutLicense::GPL_V3,
                     QObject::tr("Copyright 2008-2016, Kevin Ottens <ervin@kde.org>"));

    about.addAuthor(QObject::tr("Kevin Ottens"),
                    QObject::tr("Lead Developer"),
                    QStringLiteral("ervin@kde.org"));

    about.addAuthor(QObject::tr("Mario Bensi"),
                    QObject::tr("Developer"),
                    QStringLiteral("nef@ipsquad.net"));

    about.addAuthor(QObject::tr("Franck Arrecot"),
                    QObject::tr("Developer"),
                    QStringLiteral("franck.arrecot@gmail.com"));

    return about;
}

