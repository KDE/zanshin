/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include "domain/context.h"
#include <QSignalSpy>

using namespace Domain;

class ContextTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldHaveEmptyPropertiesByDefault()
    {
        Context c;
        QCOMPARE(c.name(), QString());
    }

    void shouldNotifyNameChanges()
    {
        Context c;
        QSignalSpy spy(&c, &Context::nameChanged);
        c.setName(QStringLiteral("foo"));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toString(), QStringLiteral("foo"));
    }

    void shouldNotNotifyIdenticalNameChanges()
    {
        Context c;
        c.setName(QStringLiteral("foo"));
        QSignalSpy spy(&c, &Context::nameChanged);
        c.setName(QStringLiteral("foo"));
        QCOMPARE(spy.count(), 0);
    }
};

ZANSHIN_TEST_MAIN(ContextTest)

#include "contexttest.moc"
