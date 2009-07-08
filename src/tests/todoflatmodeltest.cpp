/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>
   Copyright 2008, 2009 Mario Bensi <nef@ipsquad.net>

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

#include <akonadi/collection.h>
#include <akonadi/item.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/itemdeletejob.h>
#include <akonadi/qtest_akonadi.h>

#include <boost/shared_ptr.hpp>

#include <kcal/todo.h>

#include <QtGui/QSortFilterProxyModel>

#include "todoflatmodel.h"

typedef boost::shared_ptr<KCal::Incidence> IncidencePtr;

class TodoFlatModelTest : public ModelTestBase
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testInitialState_data();
    void testInitialState();
    void testItemModification_data();
    void testItemModification();
    void testSingleRemoved();
    void testRowTypeDuringInsert();

    void onInsertRows(const QModelIndex &parent, int begin, int end);
private:
    TodoFlatModel m_model;
    QSortFilterProxyModel m_sortedModel;
};

QTEST_AKONADIMAIN(TodoFlatModelTest, GUI)

void TodoFlatModelTest::initTestCase()
{
    ModelTestBase::initTestCase();
    m_model.setCollection(m_collection);
    flushNotifications();

    m_sortedModel.setSourceModel(&m_model);
    m_sortedModel.sort(TodoFlatModel::RemoteId);
}

void TodoFlatModelTest::testInitialState_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("remoteId");
    QTest::addColumn<QString>("summary");
    QTest::addColumn<QStringList>("categories");
    QTest::addColumn<QString>("parentRemoteId");
    QTest::addColumn<QString>("dueDate");
    QTest::addColumn<bool>("isImportant");

    QTest::newRow("0") <<  0 << "fake-01" << "Becoming Astronaut" << QStringList() << "fake-12" << "" << false;
    QTest::newRow("1") <<  1 << "fake-02" << "Look at the stars" << QStringList() << "fake-01" << "" << false;
    QTest::newRow("2") <<  2 << "fake-03" << "Walk around with the dog"
                       << (QStringList() << "Errands") << "fake-10" << "2008-12-25" << false;
    QTest::newRow("3") <<  3 << "fake-04" << "Second Folder" << QStringList() << "" << "" << false;
    QTest::newRow("4") <<  4 << "fake-05" << "Feed the dog"
                       << (QStringList() << "Home") << "fake-10" << "" << false;
    QTest::newRow("5") <<  5 << "fake-06" << "Read magazine"
                       << (QStringList() << "Office" << "Computer") << "fake-11" << "" << false;
    QTest::newRow("6") <<  6 << "fake-07" << "Learn the constellations" << QStringList() << "fake-01" << "" << false;
    QTest::newRow("7") <<  7 << "fake-08" << "Choose a puppy" << QStringList() << "fake-10" << "" << false;
    QTest::newRow("8") <<  8 << "fake-09" << "Listen new age album 2" << QStringList() << "fake-11" << "" << false;
    QTest::newRow("9") <<  9 << "fake-10" << "Pet Project" << QStringList() << "fake-04" << "" << false;
    QTest::newRow("10") << 10 << "fake-11" << "Becoming more relaxed" << QStringList() << "fake-12" << "" << false;
    QTest::newRow("11") << 11 << "fake-12" << "First Folder" << QStringList() << "" << "" << false;
    QTest::newRow("12") << 12 << "fake-14" << "Choose a kitty" << QStringList() << "" << "" << false;
}

void TodoFlatModelTest::testInitialState()
{
    QFETCH(int, row);
    QFETCH(QString, remoteId);
    QFETCH(QString, summary);
    QFETCH(QStringList, categories);
    QFETCH(QString, parentRemoteId);
    QFETCH(QString, dueDate);
    QFETCH(bool, isImportant);

    QCOMPARE(m_sortedModel.rowCount(), 13);
    QCOMPARE(m_sortedModel.columnCount(), 7);

    QModelIndex index = m_sortedModel.index(row, TodoFlatModel::RemoteId);
    QCOMPARE(m_sortedModel.data(index).toString(), remoteId);
    QCOMPARE(m_sortedModel.rowCount(index), 0);
    QCOMPARE(m_sortedModel.columnCount(index), 0);

    index = m_sortedModel.index(row, TodoFlatModel::Summary);
    QCOMPARE(m_sortedModel.data(index).toString(), summary);
    QCOMPARE(m_sortedModel.rowCount(index), 0);
    QCOMPARE(m_sortedModel.columnCount(index), 0);

    index = m_sortedModel.index(row, TodoFlatModel::Categories);
    QCOMPARE(m_sortedModel.data(index).toStringList(), categories);
    QCOMPARE(m_sortedModel.rowCount(index), 0);
    QCOMPARE(m_sortedModel.columnCount(index), 0);

    index = m_sortedModel.index(row, TodoFlatModel::ParentRemoteId);
    QCOMPARE(m_sortedModel.data(index).toString(), parentRemoteId);
    QCOMPARE(m_sortedModel.rowCount(index), 0);
    QCOMPARE(m_sortedModel.columnCount(index), 0);

    index = m_sortedModel.index(row, TodoFlatModel::DueDate);
    QCOMPARE(m_sortedModel.data(index).toString(), dueDate);
    QCOMPARE(m_sortedModel.rowCount(index), 0);
    QCOMPARE(m_sortedModel.columnCount(index), 0);
}

void TodoFlatModelTest::testItemModification_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("column");
    QTest::addColumn<QString>("newData");
    QTest::addColumn<bool>("expected");

    QTest::newRow("Changing Summary") <<  2 << (int)TodoFlatModel::Summary << "Walk around with the CAT" << true;
    QTest::newRow("Changing Summary (bis)") <<  2 << (int)TodoFlatModel::Summary << "Walk around with the dog" << true;
    QTest::newRow("New Parent") <<  1 << (int)TodoFlatModel::ParentRemoteId << "fake-10" << true;
    QTest::newRow("Wrong Parent") <<  1 << (int)TodoFlatModel::ParentRemoteId << "zonzog" << false;
    QTest::newRow("New Categories") <<  3 << (int)TodoFlatModel::Categories << "Computer, Office" << true;
    QTest::newRow("New Due Date") <<  6 << (int)TodoFlatModel::DueDate << "2009-03-14" << true;
    QTest::newRow("Wrong Due Date") <<  6 << (int)TodoFlatModel::DueDate << "2009-42-21" << false;
    QTest::newRow("Cycle Prevention") <<  11 << (int)TodoFlatModel::ParentRemoteId << "fake-02" << false;
}

void TodoFlatModelTest::testItemModification()
{
    QFETCH(int, row);
    QFETCH(int, column);
    QFETCH(QString, newData);
    QFETCH(bool, expected);

    QSignalSpy spy(&m_sortedModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)));

    QModelIndex index = m_sortedModel.index(row, column);
    bool result = m_sortedModel.setData(index, newData);
    QCOMPARE(result, expected);

    flushNotifications();

    if (!expected) {
        QCOMPARE(spy.count(), 0);
        return;
    }

    QCOMPARE(m_sortedModel.data(index).toStringList().join(", "), newData);

    QCOMPARE(spy.count(), 1);
    QVariantList signal = spy.takeFirst();
    QCOMPARE(signal.count(), 2);
    QCOMPARE(signal.at(0).value<QModelIndex>(), m_sortedModel.index(row, 0));
    QCOMPARE(signal.at(1).value<QModelIndex>(), m_sortedModel.index(row, TodoFlatModel::LastColumn));
}

void TodoFlatModelTest::testSingleRemoved()
{
    Akonadi::Item item = m_model.itemForIndex(m_sortedModel.mapToSource(m_sortedModel.index(1, 0)));

    int count = m_model.rowCount();

    QSignalSpy spy(&m_model, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)));

    Akonadi::ItemDeleteJob *job = new Akonadi::ItemDeleteJob(item);
    QVERIFY(job->exec());

    flushNotifications();

    QCOMPARE(m_model.rowCount(), count - 1);

    QCOMPARE(spy.count(), 1);
    QVariantList signal = spy.takeFirst();
    QCOMPARE(signal.count(), 3);
    QCOMPARE(signal.at(0).value<QModelIndex>(), QModelIndex());
    QCOMPARE(signal.at(1).toInt(), 1);
    QCOMPARE(signal.at(1).toInt(), 1);

}

void TodoFlatModelTest::testRowTypeDuringInsert()
{
    int count = m_model.rowCount();

    connect(&m_model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(onInsertRows(QModelIndex, int, int)));

    KCal::Todo *todo = new KCal::Todo();
    todo->setSummary("foo folder");
    todo->addComment("X-Zanshin-Folder");
    IncidencePtr incidence(todo);

    Akonadi::Item item;
    item.setMimeType("application/x-vnd.akonadi.calendar.todo");
    item.setPayload<IncidencePtr>(incidence);

    Akonadi::Collection collection = m_model.collection();
    Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob(item, collection);
    QVERIFY(job->exec());

    flushNotifications();

    QCOMPARE(m_model.rowCount(), count+1);

    disconnect(&m_model, SIGNAL(rowsInserted(QModelIndex,int,int)),
               this, SLOT(onInsertRows(QModelIndex, int, int)));
}

void TodoFlatModelTest::onInsertRows(const QModelIndex &parent, int begin, int end)
{
    QVERIFY(!parent.isValid());
    QVERIFY(begin==end);

    QModelIndex index = m_model.index(begin, TodoFlatModel::RowType);
    QCOMPARE(m_model.data(index).toInt(), (int)TodoFlatModel::FolderTodo);
}

#include "todoflatmodeltest.moc"
