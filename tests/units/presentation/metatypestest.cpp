/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include "domain/datasource.h"
#include "domain/task.h"

#include "presentation/metatypes.h"

class MetaTypesTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void shouldRegisterMetaTypes()
    {
        // GIVEN

        // WHEN
        Presentation::MetaTypes::registerAll();

        // THEN
        QVERIFY(QMetaType::isRegistered(qMetaTypeId<QAbstractItemModel*>()));
        QVERIFY(QMetaType::isRegistered(qMetaTypeId<QObjectPtr>()));
        QVERIFY(QMetaType::isRegistered(qMetaTypeId<Domain::Task::Ptr>()));
        QVERIFY(QMetaType::isRegistered(qMetaTypeId<Domain::DataSource::Ptr>()));
    }
};

ZANSHIN_TEST_MAIN(MetaTypesTest)

#include "metatypestest.moc"
