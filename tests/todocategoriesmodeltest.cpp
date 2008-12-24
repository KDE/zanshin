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

#include <QtGui/QSortFilterProxyModel>

#include "todoflatmodel.h"
#include "todocategoriesmodel.h"

class TodoCategoriesModelTest : public ModelTestBase
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testInitialState();
    void testSingleModification();
    void testReparentModification();
    void testSeveralReparentModification();

private:
    TodoFlatModel m_flatModel;
    QSortFilterProxyModel m_flatSortedModel;
    TodoCategoriesModel m_model;
    QSortFilterProxyModel m_sortedModel;
};

QTEST_KDEMAIN(TodoCategoriesModelTest, GUI)

void TodoCategoriesModelTest::initTestCase()
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
        const_cast<TreeNode&>(child).parent = summary;
        children << child;
        return *this;
    }

    QString id;
    QString parent;
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
        qDebug() << prefix << "[" << node.parent << node.id << node.summary;
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
        if (node.parent.isEmpty()) {
            QVERIFY(model->data(model->index(row, TodoFlatModel::Categories, root)).toStringList().isEmpty());
        } else {
            QVERIFY(model->data(model->index(row, TodoFlatModel::Categories, root)).toStringList().contains(node.parent));
        }

        QCOMPARE(model->rowCount(model->index(row, 0, root)), node.children.size());
        compareTrees(model, node.children, model->index(row, 0, root));
        row++;
    }
}

void TodoCategoriesModelTest::testInitialState()
{
    QList<TreeNode> tree;

    tree << (TreeNode(QString(), "Computer")
             << TreeNode(QString(), "Online")
             << TreeNode("fake-06", "Read magazine")
            )

         << (TreeNode(QString(), "Errands")
             << TreeNode("fake-03", "Walk around with the dog")
            )

         << (TreeNode(QString(), "Home")
             << TreeNode("fake-05", "Feed the dog")
            )

         << (TreeNode(QString(), "Office")
             << TreeNode("fake-06", "Read magazine")
            )

         << TreeNode(QString(), "Phone");

    compareTrees(&m_sortedModel, tree);
}

void TodoCategoriesModelTest::testSingleModification()
{
    Akonadi::Item item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(5, 0)));
    QModelIndexList indexes = m_model.indexesForItem(item, TodoFlatModel::Summary);

    QSignalSpy spy(&m_model, SIGNAL(dataChanged(QModelIndex, QModelIndex)));

    QVERIFY(m_model.setData(indexes.first(), "Feed the cat"));

    flushNotifications();

    foreach (const QModelIndex &index, indexes) {
        QCOMPARE(m_model.data(index).toString(), QString("Feed the cat"));
    }

    indexes = m_model.indexesForItem(item, 0);
    QCOMPARE(spy.count(), indexes.size());

    while (!spy.isEmpty()) {
        QVariantList signal = spy.takeFirst();

        QCOMPARE(signal.count(), 2);
        QModelIndex begin = signal.at(0).value<QModelIndex>();
        QModelIndex end = signal.at(1).value<QModelIndex>();

        QVERIFY(begin.row()==end.row());
        QVERIFY(begin.parent()==end.parent());
        QCOMPARE(begin.column(), 0);
        QCOMPARE(end.column(), (int)TodoFlatModel::LastColumn);

        QVERIFY(indexes.contains(begin));
    }
}

void TodoCategoriesModelTest::testReparentModification()
{
    Akonadi::Item item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(5, 0)));
    QModelIndexList indexes = m_model.indexesForItem(item, TodoFlatModel::Categories);

    QModelIndexList oldParents;
    QList<int> oldParentsRow;
    foreach (const QModelIndex &index, indexes) {
        oldParents << index.parent();
        oldParentsRow << index.row();
    }

    QModelIndexList newParents;
    QList<int> newParentsRow;
    newParents << m_model.indexForCategory("Errands")
               << m_model.indexForCategory("Home");
    foreach (const QModelIndex &newParent, newParents) {
        newParentsRow << m_model.rowCount(newParent);
    }

    QSignalSpy rowsInserted(&m_model, SIGNAL(rowsInserted(QModelIndex, int, int)));
    QSignalSpy rowsRemoved(&m_model, SIGNAL(rowsRemoved(QModelIndex, int, int)));

    // Note: It includes a category "Foo" which wasn't in the model previously,
    // we should get an insert event for it too!
    QVERIFY(m_model.setData(indexes.first(), "Home, Errands, Foo"));

    flushNotifications();

    QCOMPARE(rowsRemoved.count(), 2);
    while (!rowsRemoved.isEmpty()) {
        QVariantList signal = rowsRemoved.takeFirst();
        QCOMPARE(signal.count(), 3);

        QModelIndex parent = signal.at(0).value<QModelIndex>();
        QVERIFY(oldParents.contains(parent));

        int expectedRow = oldParentsRow[oldParents.indexOf(parent)];
        QCOMPARE(signal.at(1).toInt(), expectedRow);
        QCOMPARE(signal.at(2).toInt(), expectedRow);
    }

    QCOMPARE(rowsInserted.count(), 4);

    // first insert event should be for "Foo":
    {
        QVariantList signal = rowsInserted.takeFirst();
        QCOMPARE(signal.count(), 3);

        QModelIndex parent = signal.at(0).value<QModelIndex>();
        QVERIFY(!parent.isValid());

        int expectedRow = m_model.rowCount()-1;
        QCOMPARE(signal.at(1).toInt(), expectedRow);
        QCOMPARE(signal.at(2).toInt(), expectedRow);
    }

    // Adjust newParents and newParentsRow to include the new "Foo" category
    newParents << m_model.index(m_model.rowCount()-1, 0);
    newParentsRow << 0;

    // now check the other events for the item itself
    while (!rowsInserted.isEmpty()) {
        QVariantList signal = rowsInserted.takeFirst();
        QCOMPARE(signal.count(), 3);

        QModelIndex parent = signal.at(0).value<QModelIndex>();
        QVERIFY(newParents.contains(parent));

        int expectedRow = newParentsRow[newParents.indexOf(parent)];
        QCOMPARE(signal.at(1).toInt(), expectedRow);
        QCOMPARE(signal.at(2).toInt(), expectedRow);
    }
}

void TodoCategoriesModelTest::testSeveralReparentModification()
{
    Akonadi::Item item = m_flatModel.itemForIndex(m_flatSortedModel.mapToSource(m_flatSortedModel.index(4, 0)));
    QModelIndexList indexes = m_model.indexesForItem(item, TodoFlatModel::Categories);
    QCOMPARE(indexes.size(), 1);
    QModelIndex index = indexes.takeFirst();

    QModelIndex oldParent = index.parent();
    QCOMPARE(oldParent, m_model.indexForCategory("Home"));

    QModelIndex newParent = m_model.indexForCategory("Errands");

    QSignalSpy rowsInserted(&m_model, SIGNAL(rowsInserted(QModelIndex, int, int)));
    QSignalSpy rowsRemoved(&m_model, SIGNAL(rowsRemoved(QModelIndex, int, int)));


    QVERIFY(m_model.setData(index, "Errands"));

    flushNotifications();

    QCOMPARE(rowsRemoved.count(), 1);
    QVariantList signal = rowsRemoved.takeFirst();
    QCOMPARE(signal.count(), 3);
    QCOMPARE(signal.at(0).value<QModelIndex>(), oldParent);
    QCOMPARE(signal.at(1).toInt(), 0);
    QCOMPARE(signal.at(2).toInt(), 0);

    QCOMPARE(rowsInserted.count(), 1);
    signal = rowsInserted.takeFirst();
    QCOMPARE(signal.count(), 3);
    QCOMPARE(signal.at(0).value<QModelIndex>(), newParent);
    QCOMPARE(signal.at(1).toInt(), 2);
    QCOMPARE(signal.at(2).toInt(), 2);


    indexes = m_model.indexesForItem(item, TodoFlatModel::Categories);
    QCOMPARE(indexes.size(), 1);
    index = indexes.takeFirst();

    QVERIFY(m_model.setData(index, "Home"));

    flushNotifications();

    QCOMPARE(rowsRemoved.count(), 1);
    signal = rowsRemoved.takeFirst();
    QCOMPARE(signal.count(), 3);
    qDebug() << signal.at(0).value<QModelIndex>() << newParent;
    QCOMPARE(signal.at(0).value<QModelIndex>(), newParent);
    QCOMPARE(signal.at(1).toInt(), 2);
    QCOMPARE(signal.at(2).toInt(), 2);

    QCOMPARE(rowsInserted.count(), 1);
    signal = rowsInserted.takeFirst();
    QCOMPARE(signal.count(), 3);
    QCOMPARE(signal.at(0).value<QModelIndex>(), oldParent);
    QCOMPARE(signal.at(1).toInt(), 1);
    QCOMPARE(signal.at(2).toInt(), 1);
}

#include "todocategoriesmodeltest.moc"
