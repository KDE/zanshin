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

#include <testlib/qtest_zanshin.h>

#include <QStandardItemModel>

#include "domain/task.h"

#include "presentation/querytreemodelbase.h"
#include "presentation/taskfilterproxymodel.h"
#include "utils/datetime.h"

Q_DECLARE_METATYPE(QList<QStandardItem*>)

class TaskFilterProxyModelTest : public QObject
{
    Q_OBJECT
private:
    QStandardItem *createTaskItem(const QString &title, const QString &text,
                                  const QDate &start = QDate(),
                                  const QDate &due = QDate(),
				  bool done = false) const
    {
        auto task = Domain::Task::Ptr::create();
        task->setTitle(title);
        task->setText(text);
        task->setStartDate(start);
        task->setDueDate(due);
	task->setDone(done);

        auto item = new QStandardItem;
        item->setData(task->title(), Qt::DisplayRole);
        item->setData(QVariant::fromValue(task),
                      Presentation::QueryTreeModelBase::ObjectRole);
        return item;
    }

private slots:
    void initTestCase()
    {
        qputenv("ZANSHIN_OVERRIDE_DATE", "2015-03-11");
    }

    void shouldHaveDefaultState()
    {
        Presentation::TaskFilterProxyModel proxy;
        QVERIFY(!proxy.sourceModel());
        QCOMPARE(proxy.sortColumn(), 0);
        QCOMPARE(proxy.sortOrder(), Qt::AscendingOrder);
        QCOMPARE(proxy.sortType(), Presentation::TaskFilterProxyModel::TitleSort);
        QCOMPARE(proxy.sortCaseSensitivity(), Qt::CaseInsensitive);
	QVERIFY(!proxy.showDoneTasks());
        QVERIFY(!proxy.showFutureTasks());
    }

    void shouldFilterByTextAndTitle()
    {
        // GIVEN
        QStandardItemModel input;
        input.appendRow(createTaskItem(QStringLiteral("1. foo"), QStringLiteral("find me")));
        input.appendRow(createTaskItem(QStringLiteral("2. Find Me"), QStringLiteral("bar")));
        input.appendRow(createTaskItem(QStringLiteral("3. baz"), QStringLiteral("baz")));

        Presentation::TaskFilterProxyModel output;
        output.setSourceModel(&input);

        // WHEN
        output.setFilterRegularExpression(QStringLiteral("find me"));

        // THEN
        QCOMPARE(output.rowCount(), 2);
        QCOMPARE(output.index(0, 0).data().toString(), QStringLiteral("1. foo"));
        QCOMPARE(output.index(1, 0).data().toString(), QStringLiteral("2. Find Me"));
    }

    void shouldFilterByDoneState()
    {
        // GIVEN
        QStandardItemModel input;
        const auto today = Utils::DateTime::currentDate();
        input.appendRow(createTaskItem(QStringLiteral("1. Done"), QString(), today.addDays(-1), today, true));
        input.appendRow(createTaskItem(QStringLiteral("2. Done"), QString(), today, QDate(), true));
        input.appendRow(createTaskItem(QStringLiteral("3. Not Done"), QString(), today));
        input.appendRow(createTaskItem(QStringLiteral("4. Not Done"), QString()));

        Presentation::TaskFilterProxyModel output;
        output.setSourceModel(&input);

        // WHEN
        output.setShowDoneTasks(true);

        // THEN
        QCOMPARE(output.rowCount(), 4);
        QCOMPARE(output.index(0, 0).data().toString(), QStringLiteral("1. Done"));
        QCOMPARE(output.index(1, 0).data().toString(), QStringLiteral("2. Done"));
        QCOMPARE(output.index(2, 0).data().toString(), QStringLiteral("3. Not Done"));
        QCOMPARE(output.index(3, 0).data().toString(), QStringLiteral("4. Not Done"));

        // WHEN
        output.setShowDoneTasks(false);

        // THEN
        QCOMPARE(output.rowCount(), 2);
        QCOMPARE(output.index(0, 0).data().toString(), QStringLiteral("3. Not Done"));
        QCOMPARE(output.index(1, 0).data().toString(), QStringLiteral("4. Not Done"));
    }

    void shouldFilterByStartDate()
    {
        // GIVEN
        QStandardItemModel input;
        const auto today = Utils::DateTime::currentDate();
        input.appendRow(createTaskItem(QStringLiteral("1. past"), QString(), today.addDays(-1)));
        input.appendRow(createTaskItem(QStringLiteral("2. present"), QString(), today));
        input.appendRow(createTaskItem(QStringLiteral("3. future"), QString(), today.addDays(1)));
        input.appendRow(createTaskItem(QStringLiteral("4. whatever"), QString()));

        Presentation::TaskFilterProxyModel output;
        output.setSourceModel(&input);

        // WHEN
        output.setShowFutureTasks(true);

        // THEN
        QCOMPARE(output.rowCount(), 4);
        QCOMPARE(output.index(0, 0).data().toString(), QStringLiteral("1. past"));
        QCOMPARE(output.index(1, 0).data().toString(), QStringLiteral("2. present"));
        QCOMPARE(output.index(2, 0).data().toString(), QStringLiteral("3. future"));
        QCOMPARE(output.index(3, 0).data().toString(), QStringLiteral("4. whatever"));

        // WHEN
        output.setShowFutureTasks(false);

        // THEN
        QCOMPARE(output.rowCount(), 3);
        QCOMPARE(output.index(0, 0).data().toString(), QStringLiteral("1. past"));
        QCOMPARE(output.index(1, 0).data().toString(), QStringLiteral("2. present"));
        QCOMPARE(output.index(2, 0).data().toString(), QStringLiteral("4. whatever"));
    }

    void shouldKeepRowIfItHasAcceptableChildren()
    {
        // GIVEN
        QStandardItemModel input;
        input.appendRow(createTaskItem(QStringLiteral("1. foo"), QStringLiteral("find me")));
        QStandardItem *item = createTaskItem(QStringLiteral("2. baz"), QStringLiteral("baz"));
        item->appendRow(createTaskItem(QStringLiteral("21. bar"), QStringLiteral("bar")));
        item->appendRow(createTaskItem(QStringLiteral("22. find me"), QStringLiteral("foo")));
        input.appendRow(item);
        input.appendRow(createTaskItem(QStringLiteral("3. baz"), QStringLiteral("baz")));

        Presentation::TaskFilterProxyModel output;
        output.setSourceModel(&input);

        // WHEN
        output.setFilterRegularExpression(QStringLiteral("find me"));

        // THEN
        QCOMPARE(output.rowCount(), 2);
        QCOMPARE(output.index(0, 0).data().toString(), QStringLiteral("1. foo"));
        QCOMPARE(output.index(1, 0).data().toString(), QStringLiteral("2. baz"));

        const QModelIndex parent = output.index(1, 0);
        QCOMPARE(output.rowCount(parent), 1);
        QCOMPARE(output.index(0, 0, parent).data().toString(), QStringLiteral("22. find me"));
    }

    void shouldSortFollowingType_data()
    {
        QTest::addColumn<int>("sortType");
        QTest::addColumn<int>("sortOrder");
        QTest::addColumn<QList<QStandardItem*>>("inputItems");
        QTest::addColumn<QStringList>("expectedOutputTitles");

        QList<QStandardItem*> inputItems;
        QStringList expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem(QStringLiteral("B"), QStringLiteral("foo")) << createTaskItem(QStringLiteral("C"), QStringLiteral("foo"));
        expectedOutputTitles << QStringLiteral("B") << QStringLiteral("C");
        QTest::newRow("title ascending") << int(Presentation::TaskFilterProxyModel::TitleSort)
                                         << int(Qt::AscendingOrder)
                                         << inputItems << expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem(QStringLiteral("B"), QStringLiteral("foo")) << createTaskItem(QStringLiteral("C"), QStringLiteral("foo"));
        expectedOutputTitles << QStringLiteral("C") << QStringLiteral("B");
        QTest::newRow("title descending") << int(Presentation::TaskFilterProxyModel::TitleSort)
                                         << int(Qt::DescendingOrder)
                                         << inputItems << expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem(QStringLiteral("B"), QStringLiteral("foo"), QDate(2014, 03, 10))
                   << createTaskItem(QStringLiteral("C"), QStringLiteral("foo"), QDate(2014, 03, 01))
                   << createTaskItem(QStringLiteral("D"), QStringLiteral("foo"));
        expectedOutputTitles << QStringLiteral("C") << QStringLiteral("B") << QStringLiteral("D");
        QTest::newRow("start date ascending") << int(Presentation::TaskFilterProxyModel::DateSort)
                                              << int(Qt::AscendingOrder)
                                              << inputItems << expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem(QStringLiteral("B"), QStringLiteral("foo"), QDate(2014, 03, 10))
                   << createTaskItem(QStringLiteral("C"), QStringLiteral("foo"), QDate(2014, 03, 01))
                   << createTaskItem(QStringLiteral("D"), QStringLiteral("foo"));
        expectedOutputTitles << QStringLiteral("D") << QStringLiteral("B") << QStringLiteral("C");
        QTest::newRow("start date descending") << int(Presentation::TaskFilterProxyModel::DateSort)
                                               << int(Qt::DescendingOrder)
                                               << inputItems << expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem(QStringLiteral("B"), QStringLiteral("foo"), QDate(), QDate(2014, 03, 10))
                   << createTaskItem(QStringLiteral("C"), QStringLiteral("foo"), QDate(), QDate(2014, 03, 01))
                   << createTaskItem(QStringLiteral("D"), QStringLiteral("foo"));
        expectedOutputTitles << QStringLiteral("C") << QStringLiteral("B") << QStringLiteral("D");
        QTest::newRow("due date ascending") << int(Presentation::TaskFilterProxyModel::DateSort)
                                            << int(Qt::AscendingOrder)
                                            << inputItems << expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem(QStringLiteral("B"), QStringLiteral("foo"), QDate(), QDate(2014, 03, 10))
                   << createTaskItem(QStringLiteral("C"), QStringLiteral("foo"), QDate(), QDate(2014, 03, 01))
                   << createTaskItem(QStringLiteral("D"), QStringLiteral("foo"));
        expectedOutputTitles << QStringLiteral("D") << QStringLiteral("B") << QStringLiteral("C");
        QTest::newRow("due date descending") << int(Presentation::TaskFilterProxyModel::DateSort)
                                             << int(Qt::DescendingOrder)
                                             << inputItems << expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem(QStringLiteral("A"), QStringLiteral("foo"), QDate(2014, 03, 01), QDate(2014, 03, 10))
                   << createTaskItem(QStringLiteral("B"), QStringLiteral("foo"), QDate(2014, 03, 10), QDate(2014, 03, 01));
        expectedOutputTitles << QStringLiteral("B") << QStringLiteral("A");
        QTest::newRow("due date over start date") << int(Presentation::TaskFilterProxyModel::DateSort)
                                                  << int(Qt::AscendingOrder)
                                                  << inputItems << expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem(QStringLiteral("A"), QStringLiteral("foo"), QDate(), QDate(2014, 03, 10))
                   << createTaskItem(QStringLiteral("B"), QStringLiteral("foo"), QDate(2014, 03, 01), QDate());
        expectedOutputTitles << QStringLiteral("B") << QStringLiteral("A");
        QTest::newRow("due date over start date") << int(Presentation::TaskFilterProxyModel::DateSort)
                                                  << int(Qt::AscendingOrder)
                                                  << inputItems << expectedOutputTitles;
    }

    void shouldSortFollowingType()
    {
        // GIVEN
        QFETCH(int, sortType);
        QFETCH(int, sortOrder);
        QFETCH(QList<QStandardItem*>, inputItems);
        QFETCH(QStringList, expectedOutputTitles);

        QStandardItemModel input;
        foreach (QStandardItem *item, inputItems) {
            input.appendRow(item);
        }

        // WHEN
        Presentation::TaskFilterProxyModel output;
        output.setSourceModel(&input);
        output.setSortType(Presentation::TaskFilterProxyModel::SortType(sortType));
        output.setSortOrder(Qt::SortOrder(sortOrder));

        QStringList outputTitles;
        outputTitles.reserve(output.rowCount());
        for (int row = 0; row < output.rowCount(); row++) {
            outputTitles << output.index(row, 0).data().toString();
        }

        // THEN
        QCOMPARE(outputTitles, expectedOutputTitles);
    }
};

ZANSHIN_TEST_MAIN(TaskFilterProxyModelTest)

#include "taskfilterproxymodeltest.moc"
