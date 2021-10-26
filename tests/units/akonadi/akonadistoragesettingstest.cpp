/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>

#include "akonadi/akonadistoragesettings.h"

using namespace Akonadi;

Q_DECLARE_METATYPE(QSet<Akonadi::Collection::Id>)

class AkonadiStorageSettingsTest : public QObject
{
    Q_OBJECT
public:
    explicit AkonadiStorageSettingsTest(QObject *parent = nullptr)
        : QObject(parent)
    {
        qRegisterMetaType<Akonadi::Collection>();
        qRegisterMetaType<Akonadi::Collection::List>();
        qRegisterMetaType<QSet<Akonadi::Collection::Id>>();
    }

private:
    KConfigGroup configGroup()
    {
        return KConfigGroup(KSharedConfig::openConfig(), "General");
    }

    QList<Akonadi::Collection::Id> idList(int max)
    {
        QList<Akonadi::Collection::Id> list;
        list.reserve(max);
        for (int i = 1; i < max; i++) {
            list << i;
        }
        return list;
    }

    Akonadi::Collection::List colList(int max)
    {
        Akonadi::Collection::List list;
        list.reserve(max);
        foreach (auto id, idList(max)) {
            list << Collection(id);
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

            // THEN
            QCOMPARE(StorageSettings::instance().defaultCollection(), Collection(i));
        }
    }

    void shouldWriteToConfigurationFile()
    {
        // GIVEN

        KConfigGroup g = configGroup();

        for (int i = 1; i <= 18; i += 3) {
            // WHEN
            StorageSettings::instance().setDefaultCollection(Collection(i));

            // THEN
            QCOMPARE(g.readEntry("defaultCollection", -1), i);
        }
    }

    void shouldNotifyTaskCollectionChanges()
    {
        StorageSettings &settings = StorageSettings::instance();
        QSignalSpy spy(&settings, &Akonadi::StorageSettings::defaultCollectionChanged);
        settings.setDefaultCollection(Collection(2));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().value<Collection>(), Collection(2));
    }

    void shouldNotNotifyIdenticalTaskCollectionChanges()
    {
        StorageSettings &settings = StorageSettings::instance();
        settings.setDefaultCollection(Collection(4));
        QSignalSpy spy(&settings, &Akonadi::StorageSettings::defaultCollectionChanged);
        settings.setDefaultCollection(Collection(4));
        QCOMPARE(spy.count(), 0);
    }
};

ZANSHIN_TEST_MAIN(AkonadiStorageSettingsTest)

#include "akonadistoragesettingstest.moc"
