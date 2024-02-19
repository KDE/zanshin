/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include "domain/datasource.h"

#include <QSignalSpy>

using namespace Domain;

class DataSourceTest : public QObject
{
    Q_OBJECT
public:
    explicit DataSourceTest(QObject *parent = nullptr)
        : QObject(parent)
    {
        qRegisterMetaType<DataSource::ContentTypes>();
    }

private slots:
    void shouldHaveEmptyPropertiesByDefault()
    {
        DataSource ds;
        QCOMPARE(ds.name(), QString());
        QCOMPARE(ds.iconName(), QString());
        QCOMPARE(ds.contentTypes(), DataSource::NoContent);
        QVERIFY(!ds.isSelected());
    }

    void shouldNotifyNameChanges()
    {
        DataSource ds;
        QSignalSpy spy(&ds, &DataSource::nameChanged);
        ds.setName(QStringLiteral("Foo"));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toString(), QStringLiteral("Foo"));
    }

    void shouldNotNotifyIdenticalNameChanges()
    {
        DataSource ds;
        ds.setName(QStringLiteral("Foo"));
        QSignalSpy spy(&ds, &DataSource::nameChanged);
        ds.setName(QStringLiteral("Foo"));
        QCOMPARE(spy.count(), 0);
    }

    void shouldNotifyIconNameChanges()
    {
        DataSource ds;
        QSignalSpy spy(&ds, &DataSource::iconNameChanged);
        ds.setIconName(QStringLiteral("Foo"));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toString(), QStringLiteral("Foo"));
    }

    void shouldNotNotifyIdenticalIconNameChanges()
    {
        DataSource ds;
        ds.setIconName(QStringLiteral("Foo"));
        QSignalSpy spy(&ds, &DataSource::iconNameChanged);
        ds.setIconName(QStringLiteral("Foo"));
        QCOMPARE(spy.count(), 0);
    }

    void shouldNotifyContentTypesChanges()
    {
        DataSource ds;
        QSignalSpy spy(&ds, &DataSource::contentTypesChanged);
        ds.setContentTypes(Domain::DataSource::Tasks);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().value<Domain::DataSource::ContentTypes>(),
                 Domain::DataSource::Tasks);
    }

    void shouldNotNotifyIdenticalContentTypesChanges()
    {
        DataSource ds;
        ds.setContentTypes(Domain::DataSource::Tasks);
        QSignalSpy spy(&ds, &DataSource::contentTypesChanged);
        ds.setContentTypes(Domain::DataSource::Tasks);
        QCOMPARE(spy.count(), 0);
    }

    void shouldNotifySelectedChanges()
    {
        DataSource ds;
        QSignalSpy spy(&ds, &DataSource::selectedChanged);
        ds.setSelected(true);
        QCOMPARE(spy.count(), 1);
        QVERIFY(spy.first().first().toBool());
    }

    void shouldNotNotifyIdenticalSelectedChanges()
    {
        DataSource ds;
        ds.setSelected(true);
        QSignalSpy spy(&ds, &DataSource::selectedChanged);
        ds.setSelected(true);
        QCOMPARE(spy.count(), 0);
    }
};

ZANSHIN_TEST_MAIN(DataSourceTest)

#include "datasourcetest.moc"
