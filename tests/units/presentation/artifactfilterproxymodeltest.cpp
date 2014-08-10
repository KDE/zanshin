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
    }

    void shouldFilterByTextAndTitle()
    {
        // GIVEN
        QStandardItemModel input;
        input.appendRow(createTaskItem("1. foo", "find me"));
        input.appendRow(createTaskItem("2. Find Me", "bar"));
        input.appendRow(createTaskItem("3. baz", "baz"));
        input.appendRow(createNoteItem("4. foo", "find me"));
        input.appendRow(createNoteItem("5. find me", "bar"));
        input.appendRow(createNoteItem("6. baz", "baz"));

        Presentation::ArtifactFilterProxyModel output;
        output.setSourceModel(&input);

        // WHEN
        output.setFilterFixedString("find me");

        // THEN
        QCOMPARE(output.rowCount(), 4);
        QCOMPARE(output.index(0, 0).data().toString(), QString("1. foo"));
        QCOMPARE(output.index(1, 0).data().toString(), QString("2. Find Me"));
        QCOMPARE(output.index(2, 0).data().toString(), QString("4. foo"));
        QCOMPARE(output.index(3, 0).data().toString(), QString("5. find me"));
    }

    void shouldKeepRowIfItHasAcceptableChildren()
    {
        // GIVEN
        QStandardItemModel input;
        input.appendRow(createTaskItem("1. foo", "find me"));
        QStandardItem *item = createTaskItem("2. baz", "baz");
        item->appendRow(createTaskItem("21. bar", "bar"));
        item->appendRow(createNoteItem("22. foo", "Find Me"));
        item->appendRow(createTaskItem("23. find me", "foo"));
        input.appendRow(item);
        input.appendRow(createTaskItem("3. baz", "baz"));

        Presentation::ArtifactFilterProxyModel output;
        output.setSourceModel(&input);

        // WHEN
        output.setFilterFixedString("find me");

        // THEN
        QCOMPARE(output.rowCount(), 2);
        QCOMPARE(output.index(0, 0).data().toString(), QString("1. foo"));
        QCOMPARE(output.index(1, 0).data().toString(), QString("2. baz"));

        const QModelIndex parent = output.index(1, 0);
        QCOMPARE(output.rowCount(parent), 2);
        QCOMPARE(output.index(0, 0, parent).data().toString(), QString("22. foo"));
        QCOMPARE(output.index(1, 0, parent).data().toString(), QString("23. find me"));
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
        inputItems << createTaskItem("B", "foo") << createNoteItem("A", "foo") << createTaskItem("C", "foo");
        expectedOutputTitles << "A" << "B" << "C";
        QTest::newRow("title ascending") << int(Presentation::ArtifactFilterProxyModel::TitleSort)
                                         << int(Qt::AscendingOrder)
                                         << inputItems << expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem("B", "foo") << createNoteItem("A", "foo") << createTaskItem("C", "foo");
        expectedOutputTitles << "C" << "B" << "A";
        QTest::newRow("title descending") << int(Presentation::ArtifactFilterProxyModel::TitleSort)
                                         << int(Qt::DescendingOrder)
                                         << inputItems << expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem("B", "foo", QDate(2014, 03, 10))
                   << createNoteItem("A", "foo")
                   << createTaskItem("C", "foo", QDate(2014, 03, 01))
                   << createTaskItem("D", "foo");
        expectedOutputTitles << "C" << "B" << "D" << "A";
        QTest::newRow("start date ascending") << int(Presentation::ArtifactFilterProxyModel::DateSort)
                                              << int(Qt::AscendingOrder)
                                              << inputItems << expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem("B", "foo", QDate(2014, 03, 10))
                   << createNoteItem("A", "foo")
                   << createTaskItem("C", "foo", QDate(2014, 03, 01))
                   << createTaskItem("D", "foo");
        expectedOutputTitles << "A" << "D" << "B" << "C";
        QTest::newRow("start date descending") << int(Presentation::ArtifactFilterProxyModel::DateSort)
                                               << int(Qt::DescendingOrder)
                                               << inputItems << expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem("B", "foo", QDate(), QDate(2014, 03, 10))
                   << createNoteItem("A", "foo")
                   << createTaskItem("C", "foo", QDate(), QDate(2014, 03, 01))
                   << createTaskItem("D", "foo");
        expectedOutputTitles << "C" << "B" << "D" << "A";
        QTest::newRow("due date ascending") << int(Presentation::ArtifactFilterProxyModel::DateSort)
                                            << int(Qt::AscendingOrder)
                                            << inputItems << expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem("B", "foo", QDate(), QDate(2014, 03, 10))
                   << createNoteItem("A", "foo")
                   << createTaskItem("C", "foo", QDate(), QDate(2014, 03, 01))
                   << createTaskItem("D", "foo");
        expectedOutputTitles << "A" << "D" << "B" << "C";
        QTest::newRow("due date descending") << int(Presentation::ArtifactFilterProxyModel::DateSort)
                                             << int(Qt::DescendingOrder)
                                             << inputItems << expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem("A", "foo", QDate(2014, 03, 01), QDate(2014, 03, 10))
                   << createTaskItem("B", "foo", QDate(2014, 03, 10), QDate(2014, 03, 01));
        expectedOutputTitles << "B" << "A";
        QTest::newRow("due date over start date") << int(Presentation::ArtifactFilterProxyModel::DateSort)
                                                  << int(Qt::AscendingOrder)
                                                  << inputItems << expectedOutputTitles;

        inputItems.clear();
        expectedOutputTitles.clear();
        inputItems << createTaskItem("A", "foo", QDate(), QDate(2014, 03, 10))
                   << createTaskItem("B", "foo", QDate(2014, 03, 01), QDate());
        expectedOutputTitles << "B" << "A";
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
        for (int row = 0; row < output.rowCount(); row++) {
            outputTitles << output.index(row, 0).data().toString();
        }

        // THEN
        QCOMPARE(outputTitles, expectedOutputTitles);
    }
};

QTEST_MAIN(ArtifactFilterProxyModelTest)

#include "artifactfilterproxymodeltest.moc"
