/* This file is part of Zanshin

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

#include <QtTest>

#include <KGlobal>
#include <KConfig>
#include <KConfigGroup>

#include "akonadi/akonadistoragesettings.h"

using namespace Akonadi;

Q_DECLARE_METATYPE(QSet<Akonadi::Collection::Id>)

class AkonadiStorageSettingsTest : public QObject
{
    Q_OBJECT
public:
    explicit AkonadiStorageSettingsTest(QObject *parent = 0)
        : QObject(parent)
    {
        qRegisterMetaType<Akonadi::Collection>();
        qRegisterMetaType<QSet<Akonadi::Collection::Id>>();
    }

private:
    KConfigGroup configGroup()
    {
        return KConfigGroup(KGlobal::config(), "General");
    }

    QList<Akonadi::Collection::Id> idList(int max)
    {
        QList<Akonadi::Collection::Id> list;
        for (int i = 1; i < max; i++) {
            list << i;
        }
        return list;
    }

private slots:
    void shouldReadFromConfigurationFile()
    {
        // GIVEN

        KConfigGroup g = configGroup();

        for (int i = 1; i <= 18; i += 3) {
            // WHEN
            g.writeEntry("defaultCollection", i);
            g.writeEntry("defaultNoteCollection", i + 1);
            g.writeEntry("activeCollections", idList(i + 2));

            // THEN
            QCOMPARE(StorageSettings::instance().defaultTaskCollection(), Collection(i));
            QCOMPARE(StorageSettings::instance().defaultNoteCollection(), Collection(i + 1));
            QCOMPARE(StorageSettings::instance().activeCollections(), idList(i + 2).toSet());
        }
    }

    void shouldWriteToConfigurationFile()
    {
        // GIVEN

        KConfigGroup g = configGroup();

        for (int i = 1; i <= 18; i += 3) {
            // WHEN
            StorageSettings::instance().setDefaultTaskCollection(Collection(i));
            StorageSettings::instance().setDefaultNoteCollection(Collection(i + 1));
            StorageSettings::instance().setActiveCollections(idList(i + 2).toSet());

            // THEN
            QCOMPARE(g.readEntry("defaultCollection", -1), i);
            QCOMPARE(g.readEntry("defaultNoteCollection", -1), i + 1);
            QCOMPARE(g.readEntry("activeCollections", QList<Akonadi::Collection::Id>()).toSet(),
                     idList(i + 2).toSet());
        }
    }

    void shouldNotifyTaskCollectionChanges()
    {
        StorageSettings &settings = StorageSettings::instance();
        QSignalSpy spy(&settings, SIGNAL(defaultTaskCollectionChanged(Akonadi::Collection)));
        settings.setDefaultTaskCollection(Collection(2));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().value<Collection>(), Collection(2));
    }

    void shouldNotNotifyIdenticalTaskCollectionChanges()
    {
        StorageSettings &settings = StorageSettings::instance();
        settings.setDefaultTaskCollection(Collection(4));
        QSignalSpy spy(&settings, SIGNAL(defaultTaskCollectionChanged(Akonadi::Collection)));
        settings.setDefaultTaskCollection(Collection(4));
        QCOMPARE(spy.count(), 0);
    }

    void shouldNotifyNoteCollectionChanges()
    {
        StorageSettings &settings = StorageSettings::instance();
        QSignalSpy spy(&settings, SIGNAL(defaultNoteCollectionChanged(Akonadi::Collection)));
        settings.setDefaultNoteCollection(Collection(2));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().value<Collection>(), Collection(2));
    }

    void shouldNotNotifyIdenticalNoteCollectionChanges()
    {
        StorageSettings &settings = StorageSettings::instance();
        settings.setDefaultNoteCollection(Collection(4));
        QSignalSpy spy(&settings, SIGNAL(defaultNoteCollectionChanged(Akonadi::Collection)));
        settings.setDefaultNoteCollection(Collection(4));
        QCOMPARE(spy.count(), 0);
    }

    void shouldNotifyActiveCollectionsChanges()
    {
        StorageSettings &settings = StorageSettings::instance();
        QSignalSpy spy(&settings, SIGNAL(activeCollectionsChanged(QSet<Akonadi::Collection::Id>)));
        settings.setActiveCollections(idList(2).toSet());
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().value<QSet<Akonadi::Collection::Id>>(), idList(2).toSet());
    }

    void shouldNotNotifyIdenticalActiveCollectionsChanges()
    {
        StorageSettings &settings = StorageSettings::instance();
        settings.setActiveCollections(idList(4).toSet());
        QSignalSpy spy(&settings, SIGNAL(activeCollectionsChanged(QSet<Akonadi::Collection::Id>)));
        settings.setActiveCollections(idList(4).toSet());
        QCOMPARE(spy.count(), 0);
    }
};

QTEST_MAIN(AkonadiStorageSettingsTest)

#include "akonadistoragesettingstest.moc"
