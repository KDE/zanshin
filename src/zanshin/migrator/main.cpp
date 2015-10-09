/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
   Copyright 2014 David Faure <faure@kde.org>

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

#include <QApplication>
#include <KConfigGroup>
#include <KSharedConfig>
#include "zanshin021migrator.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    // Qt5 TODO use QCommandLineParser
    const bool force = (argc > 1 && QByteArray(argv[1]) == "--force");

    Zanshin021Migrator migrator;

    KSharedConfig::Ptr config = KSharedConfig::openConfig("zanshin-migratorrc");
    KConfigGroup group = config->group("Migrations");
    if (force || !group.readEntry("Migrated021Projects", false)) {
        if (!migrator.migrateProjects()) {
            return 1;
        }
        group.writeEntry("Migrated021Projects", true);
    }

    return 0;
};

