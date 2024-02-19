/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include "domain/project.h"

#include <QSignalSpy>

using namespace Domain;

class ProjectTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldHaveEmptyPropertiesByDefault()
    {
        Project p;
        QCOMPARE(p.name(), QString());
    }

    void shouldNotifyNameChanges()
    {
        Project p;
        QSignalSpy spy(&p, &Project::nameChanged);
        p.setName(QStringLiteral("foo"));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toString(), QStringLiteral("foo"));
    }

    void shouldNotNotifyIdenticalNameChanges()
    {
        Project p;
        p.setName(QStringLiteral("foo"));
        QSignalSpy spy(&p, &Project::nameChanged);
        p.setName(QStringLiteral("foo"));
        QCOMPARE(spy.count(), 0);
    }
};

ZANSHIN_TEST_MAIN(ProjectTest)

#include "projecttest.moc"
