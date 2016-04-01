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

#include "zanshin021migrator.h"

#include <testlib/qtest_zanshin.h>
#include <QSignalSpy>
#include <testlib/testsafety.h>
#include <testlib/akonadidebug.h>

#include <KCalCore/Todo>
#include <KCalCore/ICalFormat>

#include <AkonadiCore/Collection>
#include <AkonadiCore/TransactionSequence>
#include "akonadi/akonadicollectionfetchjobinterface.h"
#include "akonadi/akonadiitemfetchjobinterface.h"
#include "akonadi/akonadistorage.h"

#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/ItemFetchJob>
#include <AkonadiCore/ItemFetchScope>

#include <QProcess>

class Zanshin021MigrationTest : public QObject
{
    Q_OBJECT
public:
    explicit Zanshin021MigrationTest(QObject *parent = Q_NULLPTR)
        : QObject(parent)
    {
        qRegisterMetaType<Akonadi::Collection>();
        qRegisterMetaType<Akonadi::Item>();
        qRegisterMetaType<Akonadi::Tag>();
    }

private slots:
    void initTestCase()
    {
        QVERIFY(TestLib::TestSafety::checkTestIsIsolated());
        auto storage = Akonadi::StorageInterface::Ptr(new Akonadi::Storage);
        TestLib::AkonadiDebug::dumpTree(storage);
        //QProcess proc;
        //proc.execute("akonadiconsole");
        //qApp->exec();
    }

    void cleanupTestCase()
    {
        // Give a chance for jobs still waiting for an event loop
        // run to be deleted through deleteLater()
        QTest::qWait(10);
    }

    void shouldFetchAllItems()
    {
        // GIVEN
        Zanshin021Migrator migrator;

        // WHEN
        Zanshin021Migrator::SeenItemHash hash = migrator.fetchAllItems();

        // THEN
        // the migrator gathered all items and the initial state of "is a project" for each one is correct
        m_expectedUids.insert(QStringLiteral("child-of-project"), false);
        m_expectedUids.insert(QStringLiteral("new-project-with-property"), true);
        m_expectedUids.insert(QStringLiteral("old-project-with-comment"), false); // not yet
        m_expectedUids.insert(QStringLiteral("project-with-comment-and-property"), true);
        m_expectedUids.insert(QStringLiteral("project-with-children"), false); // not yet
        m_expectedUids.insert(QStringLiteral("standalone-task"), false);

        checkExpectedIsProject(hash, m_expectedUids);
    }

    void shouldMigrateCommentToProperty()
    {
        // GIVEN
        Zanshin021Migrator migrator;
        Zanshin021Migrator::SeenItemHash hash = migrator.fetchAllItems();

        // WHEN
        Akonadi::TransactionSequence *sequence = new Akonadi::TransactionSequence();
        migrator.migrateProjectComments(hash, sequence);

        // THEN
        // the project with an old-style comment was modified to have the property
        SeenItem item = hash.value(QStringLiteral("old-project-with-comment"));
        QVERIFY(item.isDirty());

        m_expectedUids[QStringLiteral("old-project-with-comment")] = true; // migrated!
        checkExpectedIsProject(hash, m_expectedUids);
        m_expectedUids[QStringLiteral("old-project-with-comment")] = false; // revert for now

        sequence->rollback();
        sequence->exec();
    }

    void shouldMigrateTaskWithChildrenToProject()
    {
        // GIVEN
        Zanshin021Migrator migrator;
        Zanshin021Migrator::SeenItemHash hash = migrator.fetchAllItems();

        // WHEN
        Akonadi::TransactionSequence *sequence = new Akonadi::TransactionSequence();
        migrator.migrateProjectWithChildren(hash, sequence);

        // THEN
        // the project with children was modified to have the property
        SeenItem item = hash.value(QStringLiteral("project-with-children"));
        QVERIFY(item.isDirty());

        m_expectedUids[QStringLiteral("project-with-children")] = true; // migrated!
        checkExpectedIsProject(hash, m_expectedUids);
        m_expectedUids[QStringLiteral("project-with-children")] = false; // revert for now

        sequence->rollback();
        sequence->exec();
    }

    void shouldMigrateProjects()
    {
        // GIVEN
        Zanshin021Migrator migrator;

        // WHEN
        const bool ret = migrator.migrateProjects();

        // THEN
        QVERIFY(ret); // success
        m_expectedUids[QStringLiteral("old-project-with-comment")] = true; // migrated!
        m_expectedUids[QStringLiteral("project-with-children")] = true; // migrated!
        Zanshin021Migrator::SeenItemHash hash = migrator.fetchAllItems();
        checkExpectedIsProject(hash, m_expectedUids);
    }

private:

    void checkExpectedIsProject(const Zanshin021Migrator::SeenItemHash &hash, const QMap<QString /*uid*/, bool /*isProject*/> &expectedItems)
    {
        QStringList uids = hash.keys();
        uids.sort();
        if (uids.count() != expectedItems.count()) // QCOMPARE for QStringList isn't verbose enough
            qWarning() << "Got" << uids << "expected" << expectedItems.keys();
        QCOMPARE(uids, QStringList(expectedItems.keys()));

        for (auto it = expectedItems.constBegin(); it != expectedItems.constEnd(); ++it) {
            //qDebug() << it.key();
            QCOMPARE(Zanshin021Migrator::isProject(hash.value(it.key()).item()), it.value());
        }
    }

    QMap<QString /*uid*/, bool /*isProject*/> m_expectedUids;
};


ZANSHIN_TEST_MAIN(Zanshin021MigrationTest)

#include "zanshin021migrationtest.moc"



