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

#include "librarymodel.h"
#include "todoflatmodel.h"
#include "todotreemodel.h"

class LibraryModelTest : public ModelTestBase
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testInitialState();

private:
    TodoFlatModel m_flatModel;
    QSortFilterProxyModel m_flatSortedModel;
    TodoTreeModel m_treeModel;
    QSortFilterProxyModel m_treeSortedModel;
    LibraryModel m_model;
};

QTEST_KDEMAIN(LibraryModelTest, GUI)

void LibraryModelTest::initTestCase()
{
    ModelTestBase::initTestCase();
    m_treeModel.setSourceModel(&m_flatModel);
    m_treeModel.setCollection(m_collection);

    m_flatSortedModel.setSourceModel(&m_flatModel);
    m_flatSortedModel.sort(TodoFlatModel::RemoteId);

    m_treeSortedModel.setSourceModel(&m_treeModel);
    m_treeSortedModel.sort(TodoFlatModel::Summary);

    m_model.setSourceModel(&m_treeSortedModel);
    flushNotifications();
}

class TreeNode
{
public:
    TreeNode(const QString &s)
        : summary(s) { }

    TreeNode &operator<<(const TreeNode &child)
    {
        children << child;
        return *this;
    }

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
        QCOMPARE(model->data(model->index(row, TodoFlatModel::Summary, root)).toString(), node.summary);

        QCOMPARE(model->rowCount(model->index(row, 0, root)), node.children.size());
        compareTrees(model, node.children, model->index(row, 0, root));
        row++;
    }
}

void LibraryModelTest::testInitialState()
{
    QList<TreeNode> tree;

    tree << TreeNode("Inbox")
         << (TreeNode("Library")
             << TreeNode("Choose a kitty")
             << (TreeNode("First Folder")
                 << (TreeNode("Becoming Astronaut")
                     << TreeNode("Learn the constellations")
                     << TreeNode("Look at the stars")
                    )
                 << (TreeNode("Becoming more relaxed")
                     << TreeNode("Listen new age album 2")
                     << TreeNode("Read magazine")
                    )
                )

             << (TreeNode("Second Folder")
                 << (TreeNode("Pet Project")
                     << TreeNode("Choose a puppy")
                     << TreeNode("Feed the dog")
                     << TreeNode("Walk around with the dog")
                    )
                )
            );

    compareTrees(&m_model, tree);
}

#include "todolibrarymodeltest.moc"
