/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-FileCopyrightText: 2014 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QApplication>
#include <KConfigGroup>
#include <KSharedConfig>
#include "zanshin021migrator.h"
#include "zanshincontextitemsmigrator.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    // Qt5 TODO use QCommandLineParser
    const bool forceTags = (argc > 1 && QByteArray(argv[1]) == "--forceMigratingTags");

    KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("zanshin-migratorrc"));
    KConfigGroup group = config->group("Migrations");
    if (!group.readEntry("Migrated021Projects", false)) {
        Zanshin021Migrator migrator;
        if (!migrator.migrateProjects()) {
            return 1;
        }
        group.writeEntry("Migrated021Projects", true);
    }

    if (forceTags || !group.readEntry("MigratedTags", false)) {
        ZanshinContextItemsMigrator migrator(forceTags);
        if (!migrator.migrateTags()) {
            return 1;
        }
        group.writeEntry("MigratedTags", true);
    }

    config->sync();

    return 0;
}
