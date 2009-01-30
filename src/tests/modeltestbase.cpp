/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>

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

#include "modeltestbase.h"

#include <qtest_kde.h>

#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agenttype.h>
#include <akonadi/collectionfetchjob.h>

#include <KDebug>
#include <KTemporaryFile>

#include <QtDBus/QDBusInterface>

ModelTestBase::ModelTestBase()
    : QObject()
{
    unsetenv("XDG_CONFIG_HOME");
}

void ModelTestBase::flushNotifications()
{
    QDBusInterface notifications("org.freedesktop.Akonadi",
                                 "/notifications/debug",
                                 "org.freedesktop.Akonadi.NotificationManager");
    notifications.call("emitPendingNotifications");
    QTest::qWait(250);
}

void ModelTestBase::initTestCase()
{
    qRegisterMetaType<QModelIndex>();

    Akonadi::AgentType type = Akonadi::AgentManager::self()->type("akonadi_ical_resource");
    Akonadi::AgentInstanceCreateJob *resJob = new Akonadi::AgentInstanceCreateJob(type);
    QVERIFY2(resJob->exec(), resJob->errorString().toLatin1().data());

    QFile originalFile(TEST_DATA);
    KTemporaryFile temp;
    temp.setAutoRemove(false);
    temp.setSuffix(".ics");
    QVERIFY(temp.open());
    m_testFile.setFileName(temp.fileName());
    temp.close();
    temp.remove();

    QVERIFY2(originalFile.copy(m_testFile.fileName()), originalFile.errorString().toLatin1().constData());

    m_agentInstance = resJob->instance();
    QDBusInterface settings(QString("org.freedesktop.Akonadi.Resource.")+m_agentInstance.identifier(),
                            "/Settings", "org.kde.Akonadi.ICal.Settings");
    settings.call("setPath", m_testFile.fileName());
    m_agentInstance.reconfigure();

    flushNotifications();

    Akonadi::CollectionFetchJob *colJob = new Akonadi::CollectionFetchJob(Akonadi::Collection::root());
    colJob->setResource(m_agentInstance.identifier());
    QVERIFY2(colJob->exec(), colJob->errorString().toLatin1().data());
    QCOMPARE(colJob->collections().size(), 1);
    m_collection = colJob->collections()[0];
}

void ModelTestBase::cleanupTestCase()
{
    QString identifier = m_agentInstance.identifier();
    Akonadi::AgentManager::self()->removeInstance(m_agentInstance);
    m_testFile.remove();
}

#include "modeltestbase.moc"
