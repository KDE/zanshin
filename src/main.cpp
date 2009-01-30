/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.
*/

#include <KDE/KAboutData>
#include <KDE/KApplication>
#include <KDE/KCmdLineArgs>
#include <KDE/KLocale>

#include "mainwindow.h"

int main(int argc, char **argv)
{
    KAboutData about("zanshin", "zanshin",
                     ki18n("Zanshin Todo"), "0.1",
                     ki18n("A Getting Things Done application which aims at getting your mind like water"),
                     KAboutData::License_GPL_V3,
                     ki18n("Copyright 2008, Kevin Ottens <ervin@kde.org>"));

    about.addAuthor(ki18n("Kevin Ottens"),
                    ki18n("Lead Developer"),
                    "ervin@kde.org");

    about.addAuthor(ki18n("Mario Bensi"),
                    ki18n("Developer"),
                    "nef@ipsquad.net");

    //TODO: Remove once we have a proper icon
    about.setProgramIconName("korganizer");

    KCmdLineArgs::init(argc, argv, &about);

    KApplication app;

    QWidget *widget = new MainWindow;
    widget->show();

    return app.exec();
}

