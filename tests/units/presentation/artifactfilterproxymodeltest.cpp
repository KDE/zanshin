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

#include "domain/note.h"
#include "domain/task.h"

#include "presentation/artifactfilterproxymodel.h"
#include "presentation/querytreemodelbase.h"

Q_DECLARE_METATYPE(QList<QStandardItem*>)

class ArtifactFilterProxyModelTest : public QObject
{
    Q_OBJECT
private:
    QStandardItem *createTaskItem(const QString &title, const QString &text,
                                  const QDate &start = QDate(),
                                  const QDate &due = QDate()) const
    {
        auto task = Domain::Task::Ptr::create();
        task->setTitle(title);
        task->setText(text);
        task->setStartDate(QDateTime(start));
        task->setDueDate(QDateTime(due));

        auto item = new QStandardItem;
        item->setData(task->title(), Qt::DisplayRole);
        item->setData(QVariant::fromValue(Domain::Artifact::Ptr(task)),
                      Presentation::QueryTreeModelBase::ObjectRole);
        return item;
    }

    QStandardItem *createNoteItem(const QString &title, const QString &text) const
    {
        auto note = Domain::Note::Ptr::create();
        note->setTitle(title);
        note->setText(text);

        auto item = new QStandardItem;
        item->setData(note->title(), Qt::DisplayRole);
        item->setData(QVariant::fromValue(Domain::Artifact::Ptr(note)),
                      Presentation::QueryTreeModelBase::ObjectRole);
        return item;
    }

private slots:
    void shouldHaveDefaultState()
    {
        Presentation::ArtifactFilterProxyModel proxy;
        QVERIFY(!proxy.sourceModel());
        QCOMPARE(proxy.sortColumn(), 0);
        QCOMPARE(proxy.sortOrder(), Qt::AscendingOrder);
        QCOMPARE(proxy.sortType(), Presentation::ArtifactFilterProxyModel::TitleSort);
        QCOMPARE(proxy.sortCaseSensitivity(), Qt::CaseInsensitive);
        QVERIFY(!proxy.showFutureTasks());
    }

    void shouldFilterByTextAndTitle()
    {
        // GIVEN
        QStandardItemModel input;
        input.appendRow(createTaskItem(QStringLiteral("1. foo"), QStringLiteral("find me")));
        input.appendRow(createTaskItem(QStringLiteral("2. Find Me"), QStringLiteral("bar")));
        input.appendRow(createTaskItem(QStringLiteral("3. baz"), QStringLiteral("baz")));
        input.appendRow(createNoteItem(QStringLiteral("4. foo"), QStringLiteral("find me")));
        input.appendRow(createNoteItem(QStringLiteral("5. find me"), QStringLiteral("bar")));
        input.appendRow(createNoteItem(QStringLiteral("6. baz"), QStringLiteral("baz")));

        Presentation::ArtifactFilterProxyModel output;
        output.setSourceModel(&input);

        // WHEN
        output.setFilterFixedString(QStringLiteral("find me"));

        // THEN
        QCOMPARE(output.rowCount(), 4);
        QCOMPARE(output.index(0, 0).data().toString(), QStringLiteral("1. foo"));
        QCOMPARE(output.index(1, 0).data().toString(), QStringLiteral("2. Find Me"));
        QCOMPARE(output.index(2, 0).data().toString(), QStringLiteral("4. foo"));
        QCOMPARE(output.index(3, 0).data().toString(), QStringLiteral("5. find me"));
    }

    void shouldFilterByStartDate()
    {
        // GIVEN
        QStandardItemModel input;
        input.appendRow(createTaskItem(QStringLiteral("1. past"), QStringLiteral(""), QDate::currentDate().addDays(-1)));
        input.appendRow(createTaskItem(QStringLiteral("2. present"), QStringLiteral(""), QDate::currentDate()));
        input.appendRow(createTaskItem(QStringLiteral("3. future"), QStringLiteral(""), QDate::currentDate().addDays(1)));
        input.appendRow(createTaskItem(QStringLiteral("4. whatever"), QStringLiteral("")));

        Presentation::ArtifactFilterProxyModel output;
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
        item->appendRow(createNoteItem(QStringLiteral("22. foo"), QStringLiteral("Find Me")));
        item->appendRow(createTaskItem(QStringLiteral("23. find me"), QStringLiteral("foo")));
        input.appendRow(item);
        input.appendRow(createTaskItem(QStringLiteral("3. baz"), QStringLiteral("baz")));

        Presentation::ArtifactFilterProxyModel output;
        output.setSourceModel(&input);

        // WHEN
        output.setFilterFixedString(QStringLiteral("find me"));

        // THEN
        QCOMPARE(output.rowCount(), 2);
        QCOMPARE(output.index(0, 0).data().toString(), QStringLiteral("1. foo"));
        QCOMPARE(output.index(1, 0).data().toString(), QStringLiteral("2. baz"));

        const QModelIndex parent = output.index(1, 0);
        QCOMPARE(output.rowCount(parent), 2);
        QCOMPARE(output.index(0, 0, parent).data().toString(), QStringLiteral("22. foo"));
        QCOMPARE(output.index(1, 0, parent).data().toString(), QStringLiteral("23. find me"));
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
        inputItems << createTaskItem(QStringLiteral("B"), QStringLiteral("foo")) << createNoteItem(QStringLiteral("A"), QStringLiteral("foo")) << createTaskItem(QStringLiteral("C"), QStringLiteral("foo"));
        expectedOutputTitles << QStringLiteral("A") << QStringLiteral("B") << QStringLiteral("C");
        QTest::newRow("title ascending") << int(Presentation::ArtifactFilterProxyModel::TitleSort)
                                         << int(Qt::AscendingOrder)
                                         << inputItems << expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem(QStringLiteral("B"), QStringLiteral("foo")) << createNoteItem(QStringLiteral("A"), QStringLiteral("foo")) << createTaskItem(QStringLiteral("C"), QStringLiteral("foo"));
        expectedOutputTitles << QStringLiteral("C") << QStringLiteral("B") << QStringLiteral("A");
        QTest::newRow("title descending") << int(Presentation::ArtifactFilterProxyModel::TitleSort)
                                         << int(Qt::DescendingOrder)
                                         << inputItems << expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem(QStringLiteral("B"), QStringLiteral("foo"), QDate(2014, 03, 10))
                   << createNoteItem(QStringLiteral("A"), QStringLiteral("foo"))
                   << createTaskItem(QStringLiteral("C"), QStringLiteral("foo"), QDate(2014, 03, 01))
                   << createTaskItem(QStringLiteral("D"), QStringLiteral("foo"));
        expectedOutputTitles << QStringLiteral("C") << QStringLiteral("B") << QStringLiteral("D") << QStringLiteral("A");
        QTest::newRow("start date ascending") << int(Presentation::ArtifactFilterProxyModel::DateSort)
                                              << int(Qt::AscendingOrder)
                                              << inputItems << expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem(QStringLiteral("B"), QStringLiteral("foo"), QDate(2014, 03, 10))
                   << createNoteItem(QStringLiteral("A"), QStringLiteral("foo"))
                   << createTaskItem(QStringLiteral("C"), QStringLiteral("foo"), QDate(2014, 03, 01))
                   << createTaskItem(QStringLiteral("D"), QStringLiteral("foo"));
        expectedOutputTitles << QStringLiteral("A") << QStringLiteral("D") << QStringLiteral("B") << QStringLiteral("C");
        QTest::newRow("start date descending") << int(Presentation::ArtifactFilterProxyModel::DateSort)
                                               << int(Qt::DescendingOrder)
                                               << inputItems << expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem(QStringLiteral("B"), QStringLiteral("foo"), QDate(), QDate(2014, 03, 10))
                   << createNoteItem(QStringLiteral("A"), QStringLiteral("foo"))
                   << createTaskItem(QStringLiteral("C"), QStringLiteral("foo"), QDate(), QDate(2014, 03, 01))
                   << createTaskItem(QStringLiteral("D"), QStringLiteral("foo"));
        expectedOutputTitles << QStringLiteral("C") << QStringLiteral("B") << QStringLiteral("D") << QStringLiteral("A");
        QTest::newRow("due date ascending") << int(Presentation::ArtifactFilterProxyModel::DateSort)
                                            << int(Qt::AscendingOrder)
                                            << inputItems << expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem(QStringLiteral("B"), QStringLiteral("foo"), QDate(), QDate(2014, 03, 10))
                   << createNoteItem(QStringLiteral("A"), QStringLiteral("foo"))
                   << createTaskItem(QStringLiteral("C"), QStringLiteral("foo"), QDate(), QDate(2014, 03, 01))
                   << createTaskItem(QStringLiteral("D"), QStringLiteral("foo"));
        expectedOutputTitles << QStringLiteral("A") << QStringLiteral("D") << QStringLiteral("B") << QStringLiteral("C");
        QTest::newRow("due date descending") << int(Presentation::ArtifactFilterProxyModel::DateSort)
                                             << int(Qt::DescendingOrder)
                                             << inputItems << expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem(QStringLiteral("A"), QStringLiteral("foo"), QDate(2014, 03, 01), QDate(2014, 03, 10))
                   << createTaskItem(QStringLiteral("B"), QStringLiteral("foo"), QDate(2014, 03, 10), QDate(2014, 03, 01));
        expectedOutputTitles << QStringLiteral("B") << QStringLiteral("A");
        QTest::newRow("due date over start date") << int(Presentation::ArtifactFilterProxyModel::DateSort)
                                                  << int(Qt::AscendingOrder)
                                                  << inputItems << expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem(QStringLiteral("A"), QStringLiteral("foo"), QDate(), QDate(2014, 03, 10))
                   << createTaskItem(QStringLiteral("B"), QStringLiteral("foo"), QDate(2014, 03, 01), QDate());
        expectedOutputTitles << QStringLiteral("B") << QStringLiteral("A");
        QTest::newRow("due date over start date") << int(Presentation::ArtifactFilterProxyModel::DateSort)
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
        Presentation::ArtifactFilterProxyModel output;
        output.setSourceModel(&input);
        output.setSortType(Presentation::ArtifactFilterProxyModel::SortType(sortType));
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

ZANSHIN_TEST_MAIN(ArtifactFilterProxyModelTest)

#include "artifactfilterproxymodeltest.moc"
