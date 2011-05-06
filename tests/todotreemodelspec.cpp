/* This file is part of Zanshin Todo.

   Copyright 2011 Mario Bensi <mbensi@ipsquad.net>

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

#include <qtest_kde.h>

#include <QtGui/QItemSelectionModel>
#include <QtGui/QStandardItemModel>
#include <QtTest/QSignalSpy>

#include "todotreemodel.h"
#include "todometadatamodel.h"
#include "testlib/testlib.h"
#include "testlib/modeltest.h"

using namespace Zanshin::Test;

Q_DECLARE_METATYPE(QModelIndex)
Q_DECLARE_METATYPE(QList<int>)

class TodoTreeModelSpec : public QObject
{
    Q_OBJECT

private:
    static QModelIndexList collectChildren(QAbstractItemModel *model, const QModelIndex &root)
    {
        QModelIndexList result;

        for (int row=0; row<model->rowCount(root); row++) {
            QModelIndex child = model->index(row, 0, root);

            if (!child.isValid()) {
                return result;
            }

            result+= collectChildren(model, child);
            result << child;
        }

        return result;
    }

private slots:
    void initTestCase()
    {
        qRegisterMetaType<QModelIndex>();

        QList<int> roles;
        roles << Qt::DisplayRole
              << Akonadi::EntityTreeModel::ItemRole
              << Akonadi::EntityTreeModel::CollectionRole;

        QTest::setEvaluatedItemRoles(roles);
    }

    void shouldRememberItsSourceModel()
    {
        //GIVEN
        QStandardItemModel baseModel;
        TodoTreeModel treeModel;
        ModelTest t1(&treeModel);

        //WHEN
        treeModel.setSourceModel(&baseModel);

        //THEN
        QVERIFY(treeModel.sourceModel() == &baseModel);
    }

    void shouldReactToSourceRowRemovals_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelPath::List>( "itemsToRemove" );
        QTest::addColumn<ModelStructure>( "outputStructure" );

        // Base items
        V inbox(Inbox);
        C c1(1, 0, "c1");
        C c2(2, 0, "c2");
        T t1(3, 1, "t1", QString(), "t1", InProgress, ProjectTag);
        T t2(4, 1, "t2", "t1", "t2");
        T t3(5, 2, "t3", QString(), "t3", InProgress, ProjectTag);
        T t4(6, 2, "t4", QString(), "t4", InProgress, ProjectTag);

        // Create the source structure once and for all
        ModelStructure sourceStructure;
        sourceStructure << c1
                        << _+t1
                        << _+t2
                        << c2
                        << _+t3
                        << _+t4;


        ModelPath::List itemsToRemove;
        itemsToRemove << c1;

        ModelStructure outputStructure;
        outputStructure << inbox
                        << c2
                        << _+t3
                        << _+t4;


        QTest::newRow( "delete collection" ) << sourceStructure << itemsToRemove << outputStructure;
    }

    void shouldReactToSourceRowRemovals()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        ModelUtils::create(&source, sourceStructure);

        //create treeModel
        TodoTreeModel treeModel;
        ModelTest t1(&treeModel);

        treeModel.setSourceModel(&source);

        //WHEN
        QFETCH(ModelPath::List, itemsToRemove);

        // Collect data to ensure we signalled the outside properly
        QSignalSpy aboutToRemoveSpy(&treeModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)));
        QSignalSpy removeSpy(&treeModel, SIGNAL(rowsRemoved(QModelIndex, int, int)));

        QList<QModelIndex> parents;
        QList<int> rows;

        foreach (const ModelPath &path, itemsToRemove) {
            QModelIndex sourceIndex = ModelUtils::locateItem(&source, path);
            QModelIndex index = treeModel.mapFromSource(sourceIndex);

            foreach (const QModelIndex &child, collectChildren(&treeModel, index)) {
                parents << child.parent();
                rows << child.row();
            }

            parents << index.parent();
            rows << index.row();
        }

        // destroy the item selected
        ModelUtils::destroy(&source, itemsToRemove);

        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);

        QCOMPARE(treeModel, output);

        QCOMPARE(aboutToRemoveSpy.size(), parents.size());
        QCOMPARE(removeSpy.size(), parents.size());

        while (aboutToRemoveSpy.size()>0) {
            QModelIndex expectedParent = parents.takeFirst();
            int expectedRow = rows.takeFirst();

            QVariantList signalPayload = aboutToRemoveSpy.takeFirst();
            QCOMPARE(signalPayload.at(0).value<QModelIndex>(), expectedParent);
            QCOMPARE(signalPayload.at(1).toInt(), expectedRow);
            QCOMPARE(signalPayload.at(2).toInt(), expectedRow);

            signalPayload = removeSpy.takeFirst();
            QCOMPARE(signalPayload.at(0).value<QModelIndex>(), expectedParent);
            QCOMPARE(signalPayload.at(1).toInt(), expectedRow);
            QCOMPARE(signalPayload.at(2).toInt(), expectedRow);
        }
    }

    void shouldReparentBasedOnUids_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelStructure>( "outputStructure" );

        // Base items
        V inbox(Inbox);
        C c1(1, 0, "c1");
        T t1(1, 1, "t1", QString(), "t1", InProgress, ProjectTag);
        T t2(2, 1, "t2", "t1", "t2");

        // Create the source structure once and for all
        ModelStructure sourceStructure;
        sourceStructure << c1
                        << _+t1
                        << _+t2;

        ModelStructure outputStructure;
        outputStructure << inbox
                        << c1
                        << _+t1
                        << __+t2;

        QTest::newRow( "add todo" ) << sourceStructure << outputStructure;
    }

    void shouldReparentBasedOnUids()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        ModelUtils::create(&source, sourceStructure);

        //WHEN
        //create treeModel
        TodoTreeModel treeModel;
        ModelTest t1(&treeModel);

        //treeModel.setSourceModel(static_cast<QAbstractItemModel*>(&source));
        treeModel.setSourceModel(&source);

        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);

        QCOMPARE(treeModel, output);
    }

    void shouldReactToSourceRowInserts_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelPath>( "sourceParentPath" );
        QTest::addColumn<ModelPath::List>( "proxyParentPathList" );
        QTest::addColumn<ModelStructure>( "insertedStructure" );
        QTest::addColumn<ModelStructure>( "outputStructure" );
        QTest::addColumn<QList<int> >( "insertRows" );

        // Base items
        V inbox(Inbox);
        C c1(1, 0, "c1");
        T t1(1, 1, "t1", QString(), "t1", InProgress, ProjectTag);
        T t2(2, 1, "t2", "t1", "t2");
        T t3(3, 1, "t3", "t2", "t3");

        // Create the source structure once and for all
        ModelStructure sourceStructure;
        sourceStructure << c1
                        << _+t1;

        ModelPath::List proxyParentPathList;

        ModelPath sourceParentPath = c1;
        proxyParentPathList << c1 % t1;

        ModelStructure insertedStructure;
        insertedStructure << t2;

        ModelStructure outputStructure;
        outputStructure << inbox
                        << c1
                        << _+t1
                        << __+t2;

        QList<int> insertRows;
        insertRows << 0;

        QTest::newRow( "add todo to project" ) << sourceStructure << sourceParentPath
                                               << proxyParentPathList << insertedStructure
                                               << outputStructure << insertRows;

        sourceStructure.clear();
        sourceStructure << c1;

        sourceParentPath = c1;
        proxyParentPathList.clear();
        proxyParentPathList << c1
                            << inbox
                            << inbox
                            << c1 % t1
                            << c1 % t1 % t2;

        insertedStructure.clear();
        insertedStructure << t1
                          << t3
                          << t2;

        outputStructure.clear();
        outputStructure << inbox
                        << c1
                        << _+t1
                        << __+t2
                        << ___+t3;

        insertRows.clear();
        insertRows << 0
                   << 0
                   << 0
                   << 0
                   << 0;

        QTest::newRow( "add todo before project" ) << sourceStructure << sourceParentPath
                                                   << proxyParentPathList << insertedStructure
                                                   << outputStructure << insertRows;
    }

    void shouldReactToSourceRowInserts()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        ModelUtils::create(&source, sourceStructure);

        //create metadataModel
        TodoMetadataModel metadataModel;
        ModelTest t1(&metadataModel);

        metadataModel.setSourceModel(&source);

        //create treeModel
        TodoTreeModel treeModel;
        ModelTest t2(&treeModel);

        treeModel.setSourceModel(&metadataModel);

        //WHEN
        QFETCH(ModelPath, sourceParentPath);
        QFETCH(ModelStructure, insertedStructure);

        // Collect data to ensure we signalled the outside properly
        QSignalSpy aboutToInsertSpy(&treeModel, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)));
        QSignalSpy insertSpy(&treeModel, SIGNAL(rowsInserted(QModelIndex, int, int)));

        ModelUtils::create(&source, insertedStructure, sourceParentPath);

        // What row number will we expect?
        QFETCH(ModelPath::List, proxyParentPathList);
        QFETCH(QList<int>, insertRows);
        QModelIndexList parentIndexList;
        foreach (ModelPath proxyParentPath, proxyParentPathList) {
            parentIndexList << ModelUtils::locateItem(&treeModel, proxyParentPath);
        }

        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);

        QCOMPARE(treeModel, output);

        QCOMPARE(aboutToInsertSpy.size(), insertRows.size());
        QCOMPARE(insertSpy.size(), insertRows.size());

        QCOMPARE(aboutToInsertSpy.size(), parentIndexList.size());
        QCOMPARE(insertSpy.size(), parentIndexList.size());

        for (int i=0; i<aboutToInsertSpy.size(); ++i) {
            QCOMPARE(aboutToInsertSpy[i].at(0).value<QModelIndex>(), parentIndexList[i]);
            QCOMPARE(aboutToInsertSpy[i].at(1).toInt(), insertRows[i]);
            QCOMPARE(aboutToInsertSpy[i].at(2).toInt(), insertRows[i]);

            QCOMPARE(insertSpy[i].at(0).value<QModelIndex>(), parentIndexList[i]);
            QCOMPARE(insertSpy[i].at(1).toInt(), insertRows[i]);
            QCOMPARE(insertSpy[i].at(2).toInt(), insertRows[i]);
        }
    }
};

QTEST_KDEMAIN(TodoTreeModelSpec, GUI)

#include "todotreemodelspec.moc"
