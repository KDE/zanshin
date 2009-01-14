/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

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

#include <akonadi/collection.h>
#include <akonadi/item.h>
#include <akonadi/itemdeletejob.h>

#include <QtGui/QSortFilterProxyModel>

#include "todoflatmodel.h"
#include "todotreemodel.h"

class TodoTreeModelTest : public ModelTestBase
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testInitialState();
    void testSingleModification();
    void testReparentModification();
    void testSingleRemoved();
    void testDragAndDropSimpleItemMove();
    void testDragAndDropMoveTwoItem();
    void testDragAndDropCycleTest();
    void testDragAndDropAddNewItem();
    void testDragAndDropMoveProjectInProject();
    void testDragAndDropMoveProjectInItem();
    void testDragAndDropMoveItemInFolder();
    void testDragAndDropMoveFolderInProject();
    void testDragAndDropMoveFolderInItem();

private:
    TodoFlatModel m_flatModel;
    QSortFilterProxyModel m_flatSortedModel;
    TodoTreeModel m_model;
    QSortFilterProxyModel m_sortedModel;
};

QTEST_KDEMAIN(TodoTreeModelTest, GUI)

void TodoTreeModelTest::initTestCase()
{
    ModelTestBase::initTestCase();
    m_model.setSourceModel(&m_flatModel);
    m_model.setCollection(m_collection);
    flushNotifications();

    m_flatSortedModel.setSourceModel(&m_flatModel);
    m_flatSortedModel.sort(TodoFlatModel::RemoteId);

    m_sortedModel.setSourceModel(&m_model);
    m_sortedModel.sort(TodoFlatModel::Summary);
}

class TreeNode
{
public:
    TreeNode(const QString &i, const QString &s)
        : id(i), summary(s) { }

    TreeNode &operator<<(const TreeNode &child)
    {
        children << child;
        return *this;
    }

    QString id;
    QString summary;
    QList<TreeNode> children;
};

#if 0
static void dumpTree(const QList<TreeNode> &tree, int indent = 0)
{
    QString prefix;
    for (int i=0; i<indent; ++i) {
        prefix+="    ";
    }

    foreach (const TreeNode &node, tree) {
        qDebug() << prefix << "[" << node.id << node.summary;
        dumpTree(node.children, indent+1);
        qDebug() << prefix << "]";
    }
}
#endif

static void compareTrees(QAbstractItemModel *model, const QList<TreeNode> &tree,
                         const QModelIndex &root = QModelIndex())
{
    int row = 0;
    foreach (const TreeNode &node, tree) {
        QCOMPARE(model->data(model->index(row, TodoFlatModel::RemoteId, root)).toString(), node.id);
        QCOMPARE(model->data(model->index(row, TodoFlatModel::Summary, root)).toString(), node.summary);

        QCOMPARE(model->rowCount(model->index(row, 0, root)), node.children.size());
        compareTrees(model, node.children, model->index(row, 0, root));
        row++;
    }
}

void TodoTreeModelTest::testInitialState()
{
    QList<TreeNode> tree;

    tree << TreeNode("fake-14", "Choose a kitty")
         << (TreeNode("fake-12", "First Folder")
             << (TreeNode("fake-01", "Becoming Astronaut")
                 << TreeNode("fake-07", "Learn the constellations")
                 << TreeNode("fake-02", "Look at the stars")
                )
             << (TreeNode("fake-11", "Becoming more relaxed")
                 << TreeNode("fake-09", "Listen new age album 2")
                 << TreeNode("fake-06", "Read magazine")
                )
            )

         << (TreeNode("fake-04", "Second Folder")
             << (TreeNode("fake-10", "Pet Project")
                 << TreeNode("fake-08", "Choose a puppy")
                 << TreeNode("fake-05", "Feed the dog")
                 << TreeNode("fake-03", "Walk around with the dog")
                )
            );

    compareTrees(&m_sortedModel, tree);
}

void TodoTreeModelTest::testSingleModification()
{
    Akonadi::Item item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(6, 0)));
    QModelIndex index = m_model.indexForItem(item, TodoFlatModel::Summary);

    QSignalSpy spy(&m_model, SIGNAL(dataChanged(QModelIndex, QModelIndex)));

    QVERIFY(m_model.setData(index, "Learn something"));

    flushNotifications();

    QCOMPARE(m_model.data(index).toString(), QString("Learn something"));

    QCOMPARE(spy.count(), 1);
    QVariantList signal = spy.takeFirst();
    QCOMPARE(signal.count(), 2);
    QCOMPARE(signal.at(0).value<QModelIndex>(), m_model.index(index.row(), 0, index.parent()));
    QCOMPARE(signal.at(1).value<QModelIndex>(), m_model.index(index.row(), TodoFlatModel::LastColumn, index.parent()));
}

void TodoTreeModelTest::testReparentModification()
{
    Akonadi::Item item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(1, 0)));
    QModelIndex index = m_model.indexForItem(item, TodoFlatModel::ParentRemoteId);

    QModelIndex oldParent = index.parent();

    item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(9, 0)));
    QModelIndex newParent = m_model.indexForItem(item, 0);

    QSignalSpy rowsInserted(&m_model, SIGNAL(rowsInserted(QModelIndex, int, int)));
    QSignalSpy rowsRemoved(&m_model, SIGNAL(rowsRemoved(QModelIndex, int, int)));



    QVERIFY(m_model.setData(index, "fake-10"));

    flushNotifications();

    QCOMPARE(rowsRemoved.count(), 1);
    QVariantList signal = rowsRemoved.takeFirst();
    QCOMPARE(signal.count(), 3);
    QCOMPARE(signal.at(0).value<QModelIndex>(), oldParent);
    QCOMPARE(signal.at(1).toInt(), 1);
    QCOMPARE(signal.at(2).toInt(), 1);

    QCOMPARE(rowsInserted.count(), 1);
    signal = rowsInserted.takeFirst();
    QCOMPARE(signal.count(), 3);
    QCOMPARE(signal.at(0).value<QModelIndex>(), newParent);
    QCOMPARE(signal.at(1).toInt(), 3);
    QCOMPARE(signal.at(2).toInt(), 3);


    item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(1, 0)));
    index = m_model.indexForItem(item, TodoFlatModel::ParentRemoteId);
    QVERIFY(m_model.setData(index, "fake-01"));

    flushNotifications();

    QCOMPARE(rowsRemoved.count(), 1);
    signal = rowsRemoved.takeFirst();
    QCOMPARE(signal.count(), 3);
    qDebug() << signal.at(0).value<QModelIndex>() << newParent;
    QCOMPARE(signal.at(0).value<QModelIndex>(), newParent);
    QCOMPARE(signal.at(1).toInt(), 3);
    QCOMPARE(signal.at(2).toInt(), 3);

    QCOMPARE(rowsInserted.count(), 1);
    signal = rowsInserted.takeFirst();
    QCOMPARE(signal.count(), 3);
    QCOMPARE(signal.at(0).value<QModelIndex>(), oldParent);
    QCOMPARE(signal.at(1).toInt(), 1);
    QCOMPARE(signal.at(2).toInt(), 1);
}

void TodoTreeModelTest::testSingleRemoved()
{
    Akonadi::Item item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(2, 0)));
    QModelIndex index = m_model.indexForItem(item, TodoFlatModel::ParentRemoteId);

    QModelIndex parent = index.parent();
    int count = m_model.rowCount(parent);

    QSignalSpy spy(&m_model, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)));

    Akonadi::ItemDeleteJob *job = new Akonadi::ItemDeleteJob(item);
    QVERIFY(job->exec());

    flushNotifications();

    QCOMPARE(m_model.rowCount(parent), count - 1);

    QCOMPARE(spy.count(), 1);
    QVariantList signal = spy.takeFirst();
    QCOMPARE(signal.count(), 3);
    QCOMPARE(signal.at(0).value<QModelIndex>(), parent);
    QCOMPARE(signal.at(1).toInt(), 2);
    QCOMPARE(signal.at(1).toInt(), 2);
}

void TodoTreeModelTest::testDragAndDropSimpleItemMove()
{
    Akonadi::Item item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(5, 0)));
    QModelIndex index = m_model.indexForItem(item);
    QModelIndex parentRemoteIndex = m_model.indexForItem(item, TodoFlatModel::ParentRemoteId);

    QCOMPARE(m_model.data(parentRemoteIndex).toString(), QString("fake-01"));

    QModelIndexList indexes;
    indexes << index;
    QMimeData *mimeData = m_model.mimeData(indexes);
    QVERIFY(m_model.dropMimeData(mimeData, Qt::MoveAction, 0, 0, QModelIndex()));

    parentRemoteIndex = m_model.indexForItem(item, TodoFlatModel::ParentRemoteId);
    QCOMPARE(m_model.data(parentRemoteIndex).toString(), QString());

    indexes.clear();
}

void TodoTreeModelTest::testDragAndDropMoveTwoItem()
{
    Akonadi::Item item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(3, 0)));
    QModelIndex index = m_model.indexForItem(item);
    QModelIndex parentRemoteIndex = m_model.indexForItem(item, TodoFlatModel::ParentRemoteId);

    QCOMPARE(m_model.data(parentRemoteIndex).toString(), QString("fake-10"));

    Akonadi::Item item2 = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(2, 0)));
    QModelIndex index2 = m_model.indexForItem(item2);
    QModelIndex parentRemoteIndex2 = m_model.indexForItem(item2, TodoFlatModel::ParentRemoteId);

    QCOMPARE(m_model.data(parentRemoteIndex2).toString(), QString(""));

    Akonadi::Item newParentItem = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(10, 0)));
    QModelIndex newParentIndex = m_model.indexForItem(newParentItem);
    QModelIndex newParentRemoteIndex = m_model.indexForItem(newParentItem, TodoFlatModel::RemoteId);
    QString remoteId = m_model.data(newParentRemoteIndex).toString();

    QModelIndexList indexes;
    indexes << index;
    indexes << index2;
    QMimeData *mimeData = m_model.mimeData(indexes);
    QVERIFY(m_model.dropMimeData(mimeData, Qt::MoveAction, 0, 0, newParentIndex));

    parentRemoteIndex = m_model.indexForItem(item, TodoFlatModel::ParentRemoteId);

    QCOMPARE(m_model.data(parentRemoteIndex).toString(), remoteId);

    parentRemoteIndex = m_model.indexForItem(item2, TodoFlatModel::ParentRemoteId);
    QCOMPARE(m_model.data(parentRemoteIndex).toString(), remoteId);

    indexes.clear();
}

void TodoTreeModelTest::testDragAndDropCycleTest()
{
    //test cycle
    Akonadi::Item item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(9, 0)));
    QModelIndex index = m_model.indexForItem(item);
    QModelIndex parentRemoteIndex = m_model.indexForItem(item, TodoFlatModel::ParentRemoteId);

    QCOMPARE(m_model.data(parentRemoteIndex).toString(), QString("fake-12"));

    item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(7, 0)));
    QModelIndex RemoteIndex = m_model.indexForItem(item, TodoFlatModel::RemoteId);
    QCOMPARE(m_model.data(RemoteIndex).toString(), QString("fake-09"));

    QModelIndexList indexes;
    indexes << index;
    QMimeData *mimeData = m_model.mimeData(indexes);
    QVERIFY(!m_model.dropMimeData(mimeData, Qt::MoveAction, 0, 0, RemoteIndex));

    indexes.clear();
}

void TodoTreeModelTest::testDragAndDropAddNewItem()
{
    Akonadi::Item item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(11, 0)));
    QModelIndex index = m_flatModel.indexForItem(item, 0);
    QModelIndex parentRemoteIndex = m_flatModel.indexForItem(item, TodoFlatModel::ParentRemoteId);
    QModelIndex RemoteIndex = m_flatModel.indexForItem(item, TodoFlatModel::RemoteId);

    QCOMPARE(m_flatModel.data(parentRemoteIndex).toString(), QString(""));

    Akonadi::Item newParentItem = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(10, 0)));
    QModelIndex newParentIndex = m_model.indexForItem(newParentItem);
    QModelIndex newParentRemoteIndex = m_model.indexForItem(newParentItem, TodoFlatModel::RemoteId);

    QModelIndexList indexes;
    indexes << index;
    QMimeData *mimeData = m_flatModel.mimeData(indexes);
    QVERIFY(m_model.dropMimeData(mimeData, Qt::MoveAction, 0, 0, newParentIndex));

    parentRemoteIndex = m_model.indexForItem(item, TodoFlatModel::ParentRemoteId);

    QCOMPARE(m_model.data(parentRemoteIndex).toString(), m_model.data(newParentRemoteIndex).toString());

    indexes.clear();
}

void TodoTreeModelTest::testDragAndDropMoveProjectInProject()
{
    Akonadi::Item item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(9, 0)));
    QModelIndex index = m_model.indexForItem(item);
    QModelIndex RemoteIndex = m_model.indexForItem(item, TodoFlatModel::RemoteId);
    QModelIndex TypeIndex = m_model.indexForItem(item, TodoFlatModel::RowType);
    QCOMPARE(m_model.data(TypeIndex).toString(), QString::number(TodoFlatModel::ProjectTodo));
    QCOMPARE(m_model.data(RemoteIndex).toString(), QString("fake-11"));

    item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(0, 0)));
    QModelIndex newParentIndex = m_model.indexForItem(item);
    RemoteIndex = m_model.indexForItem(item, TodoFlatModel::RemoteId);
    TypeIndex = m_model.indexForItem(item, TodoFlatModel::RowType);
    QCOMPARE(m_model.data(TypeIndex).toString(), QString::number(TodoFlatModel::ProjectTodo));
    QCOMPARE(m_model.data(RemoteIndex).toString(), QString("fake-01"));

    QModelIndexList indexes;
    indexes << index;
    QMimeData *mimeData = m_model.mimeData(indexes);
    QVERIFY(!m_model.dropMimeData(mimeData, Qt::MoveAction, 0, 0, newParentIndex));

    indexes.clear();
}

void TodoTreeModelTest::testDragAndDropMoveProjectInItem()
{
    //move project in item
    Akonadi::Item item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(9, 0)));
    QModelIndex index = m_model.indexForItem(item);
    QModelIndex RemoteIndex = m_model.indexForItem(item, TodoFlatModel::RemoteId);
    QModelIndex TypeIndex = m_model.indexForItem(item, TodoFlatModel::RowType);
    QCOMPARE(m_model.data(TypeIndex).toString(), QString::number(TodoFlatModel::ProjectTodo));
    QCOMPARE(m_model.data(RemoteIndex).toString(), QString("fake-11"));

    item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(1, 0)));
    QModelIndex newParentIndex = m_model.indexForItem(item);
    RemoteIndex = m_model.indexForItem(item, TodoFlatModel::RemoteId);
    TypeIndex = m_model.indexForItem(item, TodoFlatModel::RowType);
    QCOMPARE(m_model.data(TypeIndex).toString(), QString::number(TodoFlatModel::StandardTodo));
    QCOMPARE(m_model.data(RemoteIndex).toString(), QString("fake-02"));

    QModelIndexList indexes;
    indexes << index;
    QMimeData *mimeData = m_model.mimeData(indexes);
    QVERIFY(!m_model.dropMimeData(mimeData, Qt::MoveAction, 0, 0, newParentIndex));

    indexes.clear();
}

void TodoTreeModelTest::testDragAndDropMoveItemInFolder()
{
    //move item in folder
    Akonadi::Item item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(4, 0)));
    QModelIndex index = m_model.indexForItem(item);
    QModelIndex RemoteIndex = m_model.indexForItem(item, TodoFlatModel::RemoteId);
    QModelIndex TypeIndex = m_model.indexForItem(item, TodoFlatModel::RowType);
    QCOMPARE(m_model.data(TypeIndex).toString(), QString::number(TodoFlatModel::StandardTodo));
    QCOMPARE(m_model.data(RemoteIndex).toString(), QString("fake-06"));

    item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(10, 0)));
    QModelIndex newParentIndex = m_model.indexForItem(item);
    RemoteIndex = m_model.indexForItem(item, TodoFlatModel::RemoteId);
    TypeIndex = m_model.indexForItem(item, TodoFlatModel::RowType);
    QCOMPARE(m_model.data(TypeIndex).toString(), QString::number(TodoFlatModel::FolderTodo));
    QCOMPARE(m_model.data(RemoteIndex).toString(), QString("fake-12"));

    QModelIndexList indexes;
    indexes << index;
    QMimeData *mimeData = m_model.mimeData(indexes);
    QVERIFY(m_model.dropMimeData(mimeData, Qt::MoveAction, 0, 0, newParentIndex));

    item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(4, 0)));
    RemoteIndex = m_model.indexForItem(item, TodoFlatModel::RemoteId);
    QCOMPARE(m_model.data(RemoteIndex).toString(), QString("fake-06"));
    TypeIndex = m_model.indexForItem(item, TodoFlatModel::RowType);
    QCOMPARE(m_model.data(TypeIndex).toString(), QString::number(TodoFlatModel::ProjectTodo));

    indexes.clear();
}

void TodoTreeModelTest::testDragAndDropMoveFolderInProject()
{
    Akonadi::Item item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(2, 0)));
    QModelIndex index = m_model.indexForItem(item);
    QModelIndex RemoteIndex = m_model.indexForItem(item, TodoFlatModel::RemoteId);
    QModelIndex TypeIndex = m_model.indexForItem(item, TodoFlatModel::RowType);
    QCOMPARE(m_model.data(TypeIndex).toString(), QString::number(TodoFlatModel::FolderTodo));
    QCOMPARE(m_model.data(RemoteIndex).toString(), QString("fake-04"));

    item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(4, 0)));
    QModelIndex newParentIndex = m_model.indexForItem(item);
    RemoteIndex = m_model.indexForItem(item, TodoFlatModel::RemoteId);
    TypeIndex = m_model.indexForItem(item, TodoFlatModel::RowType);
    QCOMPARE(m_model.data(TypeIndex).toString(), QString::number(TodoFlatModel::ProjectTodo));
    QCOMPARE(m_model.data(RemoteIndex).toString(), QString("fake-06"));

    QModelIndexList indexes;
    indexes << index;
    QMimeData *mimeData = m_model.mimeData(indexes);
    QVERIFY(!m_model.dropMimeData(mimeData, Qt::MoveAction, 0, 0, newParentIndex));

    indexes.clear();
}

void TodoTreeModelTest::testDragAndDropMoveFolderInItem()
{
    Akonadi::Item item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(2, 0)));
    QModelIndex index = m_model.indexForItem(item);
    QModelIndex RemoteIndex = m_model.indexForItem(item, TodoFlatModel::RemoteId);
    QModelIndex TypeIndex = m_model.indexForItem(item, TodoFlatModel::RowType);
    QCOMPARE(m_model.data(TypeIndex).toString(), QString::number(TodoFlatModel::FolderTodo));
    QCOMPARE(m_model.data(RemoteIndex).toString(), QString("fake-04"));

    item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(7, 0)));
    QModelIndex newParentIndex = m_model.indexForItem(item);
    RemoteIndex = m_model.indexForItem(item, TodoFlatModel::RemoteId);
    TypeIndex = m_model.indexForItem(item, TodoFlatModel::RowType);
    QCOMPARE(m_model.data(TypeIndex).toString(), QString::number(TodoFlatModel::StandardTodo));
    QCOMPARE(m_model.data(RemoteIndex).toString(), QString("fake-09"));

    QModelIndexList indexes;
    indexes << index;
    QMimeData *mimeData = m_model.mimeData(indexes);
    QVERIFY(!m_model.dropMimeData(mimeData, Qt::MoveAction, 0, 0, newParentIndex));
}

#include "todotreemodeltest.moc"
