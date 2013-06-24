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

#include "core/todometadatamodel.h"
#include "reparentingmodel/reparentingmodel.h"
#include "core/projectstrategy.h"
#include "core/pimitemservices.h"
#include "core/noteitem.h"
#include "core/pimitemmodel.h"
#include "testlib/testlib.h"
#include "testlib/mockdatastore.h"
#include "testlib/mockmodel.h"
#include "testlib/modeltest.h"
#include "testlib/modelbuilderbehavior.h"
#include "testlib/helperutils.h"

using namespace Zanshin::Test;

Q_DECLARE_METATYPE(QModelIndex)
Q_DECLARE_METATYPE(QList<int>)

/*
 * Test for the ProjectStrategy
 * Assumes a working ReparentingModel so run ReparentingModelSpec first.
 */
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
        DataStoreInterface::overrideImplementation(new MockDataStore);
    }

    void shouldRememberItsSourceModel()
    {
        //GIVEN
        QStandardItemModel baseModel;
        ReparentingModel treeModel(new ProjectStrategy());
        ModelTest t1(&treeModel);

        //WHEN
        treeModel.setSourceModel(&baseModel);

        //THEN
        QVERIFY(treeModel.sourceModel() == &baseModel);
    }

    void shouldReparentBasedOnUids_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelStructure>( "outputStructure" );

        // Base items
        V inbox(Inbox);
        C c1(1, 0, "c1");
        C c2(2, 0, "c2");
        T t1(1, 1, "t1", QString(), "t1", InProgress, ProjectTag);
        T t2(2, 1, "t2", "t1", "t2");
        T t3(5, 2, "t3", QString(), "t3", InProgress, ProjectTag);
        T t4(6, 2, "t4", QString(), "t4", InProgress, ProjectTag);
        {
            // Create the source structure once and for all
            ModelStructure sourceStructure;
            sourceStructure << t1
                            << t2;

            ModelStructure outputStructure;
            outputStructure << inbox
                            << t1
                            << _+t2;

            QTest::newRow( "add todo" ) << sourceStructure << outputStructure;
        }
        {
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

            QTest::newRow( "add todo with collection" ) << sourceStructure << outputStructure;
        }
        {
            // Create the source structure once and for all
            ModelStructure sourceStructure;
            sourceStructure
//                             << c1
//                             << _+t1
//                             << _+t2
                            << c2
                            << _+t3
                            << _+t4;

            ModelStructure outputStructure;
            outputStructure << inbox
//                             << c1
//                             << _+t1
//                             << __+t2
                            << c2
                            << _+t3
                            << _+t4;

            QTest::newRow( "don't move projects to inbox" ) << sourceStructure << outputStructure;
        }
        {
            G n1(100, Qt::DisplayRole, "note1");
            n1.data.insert(PimItemModel::ItemTypeRole, PimItemIndex::Note);
            NoteItem note;
            note.setRelations(QList<PimItemRelation>() << PimItemRelation(PimItemRelation::Project, QList<PimItemTreeNode>() << PimItemTreeNode(t1.uid.toLatin1())));
            Akonadi::Item item = note.getItem();
            item.setId(234);
            item.setParentCollection(Akonadi::Collection(c2.id));
            n1.data.insert(Akonadi::EntityTreeModel::ItemRole, QVariant::fromValue<Akonadi::Item>(item));
            // Create the source structure once and for all
            ModelStructure sourceStructure;
            sourceStructure
                            << c1
                            << _+t1
                            << c2
                            << _+n1;

            ModelStructure outputStructure;
            outputStructure << inbox
                            << c1
                            << _+t1
                            << __+n1
                            << c2
                            << _+n1;

            QTest::newRow( "build tree with notes" ) << sourceStructure << outputStructure;
        }
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
        ReparentingModel treeModel(new ProjectStrategy());
//         ReparentingModel treeModel(new PimItemRelationStrategy(PimItemRelation::Project));
        ModelTest t1(&treeModel);

        treeModel.setSourceModel(&source);

        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);

        QCOMPARE(treeModel, output);
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
        ReparentingModel treeModel(new ProjectStrategy());
        ModelTest t1(&treeModel);

        treeModel.setSourceModel(&source);

        Helper::printModel(&treeModel);
        //WHEN
        QFETCH(ModelPath::List, itemsToRemove);

        // Collect data to ensure we signalled the outside properly
        QSignalSpy aboutToRemoveSpy(&treeModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
        QSignalSpy removeSpy(&treeModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));

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

        Helper::printModel(&treeModel);
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
        T t4(4, 1, "t4", QString(), "t4");
        T t5(5, 1, "t5", "t4", "t5");
        T t6(6, 1, "t6", "t5", "t6");

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
        sourceStructure << c1
                        << _+t1
                        << __+t2;

        sourceParentPath = c1 % t1 %t2;

        proxyParentPathList.clear();
        proxyParentPathList << c1
                            << inbox
                            << c1 % t2
                            << c1 % t2 % t3;

        insertedStructure.clear();
        insertedStructure << t3;

        outputStructure.clear();
        outputStructure << inbox
                        << c1
                        << _+t1
                        << __+t2
                        << ___+t3;

        insertRows.clear();
        insertRows << 0;

        QTest::newRow( "add todo to project without project tag" ) << sourceStructure << sourceParentPath
                                                                   << proxyParentPathList << insertedStructure
                                                                   << outputStructure;

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

        sourceStructure.clear();
        sourceStructure << c1;

        sourceParentPath = c1;

        proxyParentPathList.clear();
        proxyParentPathList << inbox
                            << c1
                            << c1 % t1
                            << inbox
                            << c1
                            << c1 % t4
                            << c1 % t1 % t2
                            << c1
                            << c1 % t4
                            << c1 % t4 % t5;

        insertedStructure.clear();
        insertedStructure << t2
                          << t1
                          << t5
                          << t6
                          << t3
                          << t4;

        outputStructure.clear();
        outputStructure << inbox
                        << c1
                        << _+t1
                        << __+t2
                        << ___+t3
                        << _+t4
                        << __+t5
                        << ___+t6;

        insertRows.clear();
        insertRows << 0
                   << 0
                   << 0
                   << 0
                   << 1
                   << 0
                   << 0
                   << 2
                   << 0
                   << 0;

        QTest::newRow( "add todos in random order" )  << sourceStructure << sourceParentPath
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
        ReparentingModel treeModel(new ProjectStrategy());
        ModelTest t2(&treeModel);

        treeModel.setSourceModel(&metadataModel);

        //WHEN
        QFETCH(ModelPath, sourceParentPath);
        QFETCH(ModelStructure, insertedStructure);

        // Collect data to ensure we signalled the outside properly
        QSignalSpy aboutToInsertSpy(&treeModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
        QSignalSpy insertSpy(&treeModel, SIGNAL(rowsInserted(QModelIndex,int,int)));

        ModelUtils::create(&source, insertedStructure, sourceParentPath);

        // What row number will we expect?
//         QFETCH(ModelPath::List, proxyParentPathList);
//         QFETCH(QList<int>, insertRows);
//         QModelIndexList parentIndexList;
//         foreach (ModelPath proxyParentPath, proxyParentPathList) {
//             parentIndexList << ModelUtils::locateItem(&treeModel, proxyParentPath);
//         }

        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);

        QCOMPARE(treeModel, output);

//         QCOMPARE(aboutToInsertSpy.size(), insertRows.size());
//         QCOMPARE(insertSpy.size(), insertRows.size());
// 
//         QCOMPARE(aboutToInsertSpy.size(), parentIndexList.size());
//         QCOMPARE(insertSpy.size(), parentIndexList.size());
// 
//         for (int i=0; i<aboutToInsertSpy.size(); ++i) {
//             QCOMPARE(aboutToInsertSpy[i].at(0).value<QModelIndex>(), parentIndexList[i]);
//             QCOMPARE(aboutToInsertSpy[i].at(1).toInt(), insertRows[i]);
//             QCOMPARE(aboutToInsertSpy[i].at(2).toInt(), insertRows[i]);
// 
//             QCOMPARE(insertSpy[i].at(0).value<QModelIndex>(), parentIndexList[i]);
//             QCOMPARE(insertSpy[i].at(1).toInt(), insertRows[i]);
//             QCOMPARE(insertSpy[i].at(2).toInt(), insertRows[i]);
//         }
    }

    void shouldHandleProjectMoves_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelPath>( "itemToChange" );
        QTest::addColumn<QString>( "parentUid" );
        QTest::addColumn<ModelStructure>( "outputStructure" );

        // Base items
        V inbox(Inbox);
        C c1(1, 0, "c1");
        T p1(1, 1, "p1", QString(), "p1", InProgress, ProjectTag);
        T p2(2, 1, "p2", QString(), "p2", InProgress, ProjectTag);
        T p3(3, 1, "p3", "p1", "p3", InProgress, ProjectTag);
        T t1(11, 1, "t1", "p1", "t1");
        T t2(22, 1, "t2", "p2", "t2");
        T t3(33, 1, "t3", "p3", "t3");
        C c2(2, 0, "c2");
        T p4(4, 2, "p4", QString(), "p4", InProgress, ProjectTag);

        {
            ModelStructure sourceStructure;
            sourceStructure << c1
                            << _+t2
                            << _+p1
                            << _+p2;

            ModelPath itemToChange = c1 % p2;

            QString parentUid = "p1";

            ModelStructure outputStructure;
            outputStructure << inbox
                            << c1
                            << _+p1
                            << __+p2
                            << ___+t2;

            QTest::newRow( "root project moved under another project of the same collection" )
                << sourceStructure << itemToChange
                << parentUid << outputStructure;
        }
        {
            ModelStructure sourceStructure;
            sourceStructure << c1
                            << _+t2
                            << _+t3
                            << _+p1
                            << _+p2
                            << _+p3;

            ModelPath itemToChange = c1 % p3;
            QString parentUid = QString();
            ModelStructure outputStructure;
            outputStructure << inbox
                            << c1
                            << _+p1
                            << _+p2
                            << __+t2
                            << _+p3
                            << __+t3;
                            
            QTest::newRow( "sub-project moved as root in the same collection" )
                << sourceStructure << itemToChange
                << parentUid << outputStructure;
        }

    }

    void shouldHandleProjectMoves()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        ModelUtils::create(&source, sourceStructure);

        //create treeModel
        ProjectStructureCache *structure = new ProjectStructureCache();
        ProjectStructureInterface *interface = new ProjectStructureInterface();
        interface->setRelationsStructure(structure);
        ReparentingModel treeModel(new ProjectStrategy(structure));
        ModelTest t1(&treeModel);

        treeModel.setSourceModel(&source);

        //WHEN
        QFETCH(ModelPath, itemToChange);
        QModelIndex index = ModelUtils::locateItem(&source, itemToChange);

        QFETCH(QString, parentUid);
        if (!parentUid.isEmpty()) {
            PimItemIndex parent(PimItemIndex::Project);
            parent.uid = parentUid;
            interface->moveTo(PimItemRelationInterface::fromIndex(index), parent);
        } else {
            interface->moveTo(PimItemRelationInterface::fromIndex(index), PimItemIndex(PimItemIndex::Inbox));
        }

        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);

        QCOMPARE(treeModel, output);
    }

    void shouldHandleTodoMoves_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelPath>( "itemToChange" );
        QTest::addColumn<QString>( "parentUid" );
        QTest::addColumn<ModelStructure>( "outputStructure" );

        // Base items
        V inbox(Inbox);
        C c1(1, 0, "c1");
        T p1(1, 1, "p1", QString(), "p1", InProgress, ProjectTag);
        T p2(2, 1, "p2", "p1", "p2", InProgress, ProjectTag);
        T t1(11, 1, "t1", "p2", "t1");
        T t2(22, 1, "t2", "p2", "t2");
        T t3(33, 1, "t3", "p1", "t3");
        T t4(44, 1, "t4", "t3", "t4");
        T t5(55, 1, "t5", "t3", "t5");

        {
            ModelStructure sourceStructure;
            sourceStructure << c1
                            << _+p1
                            << __+p2
                            << ___+t1
                            << ___+t2
                            << __+t3
                            << ___+t4;

            ModelPath itemToChange = c1 % p1 % p2 % t2;

            QString parentUid = "t3";

            ModelStructure outputStructure;
            outputStructure << inbox
                            << c1
                            << _+p1
                            << __+p2
                            << ___+t1
                            << __+t3
                            << ___+t4
                            << ___+t2;

            QTest::newRow( "todo moved under another project without project tag" )
                << sourceStructure << itemToChange
                << parentUid << outputStructure;
        }

        {
            ModelStructure sourceStructure;
            sourceStructure << c1
                            << _+p1
                            << __+p2
                            << ___+t1
                            << ___+t2
                            << __+t3
                            << ___+t4
                            << ___+t5;

            ModelPath itemToChange = c1 % p1 % t3 % t5;

            QString parentUid = "p2";

            ModelStructure outputStructure;
            outputStructure << inbox
                            << c1
                            << _+p1
                            << __+p2
                            << ___+t1
                            << ___+t2
                            << ___+t5
                            << __+t3
                            << ___+t4;

            QTest::newRow( "todo moved from project without tag under another project with project tag" )
                << sourceStructure << itemToChange
                << parentUid << outputStructure;
        }

        {
            ModelStructure sourceStructure;
            sourceStructure << c1
                            << _+p1
                            << __+p2
                            << ___+t1
                            << ___+t2
                            << __+t3
                            << ___+t4;

            ModelPath itemToChange = c1 % p1 % t3 % t4;

            QString parentUid = "p2";

            ModelStructure outputStructure;
            outputStructure << inbox
                            << c1
                            << _+p1
                            << __+p2
                            << ___+t1
                            << ___+t2
                            << ___+t4
                            << __+t3;

            QTest::newRow( "todo moved from project without tag with only this todo and the project should begun a todo" )
                << sourceStructure << itemToChange
                << parentUid << outputStructure;
        }
    }

    void shouldHandleTodoMoves()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        ModelUtils::create(&source, sourceStructure);

        //create treeModel
        ProjectStructureCache *structure = new ProjectStructureCache();
        ProjectStructureInterface *interface = new ProjectStructureInterface();
        interface->setRelationsStructure(structure);
        ReparentingModel treeModel(new ProjectStrategy(structure));
        ModelTest t1(&treeModel);

        treeModel.setSourceModel(&source);

        //WHEN
        QFETCH(ModelPath, itemToChange);
        QModelIndex index = ModelUtils::locateItem(&source, itemToChange);

        QFETCH(QString, parentUid);
        if (!parentUid.isEmpty()) {
            PimItemIndex parent(PimItemIndex::Project);
            parent.uid = parentUid;
            interface->moveTo(PimItemRelationInterface::fromIndex(index), parent);
        } else {
            interface->moveTo(PimItemRelationInterface::fromIndex(index), PimItemIndex(PimItemIndex::Inbox));
        }

        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);

        QCOMPARE(treeModel, output);
    }


    void shouldKeepSourceOrder_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelStructure>( "outputStructure" );

        // Base items
        V inbox(Inbox);
        C c1(1, 0, "c1");
        C c2(2, 0, "c2");
        T t1(1, 0, "t1", QString(), "t1", InProgress, ProjectTag);
        T t2(2, 1, "t2", "t1", "t2");
        T t3(3, 1, "t3", "t1", "t3", InProgress, ProjectTag);
        T t4(4, 1, "t4", "t1", "t4");
        T t5(5, 1, "t5", "t1", "t5", InProgress, ProjectTag);

        // Create the source structure once and for all
        ModelStructure sourceStructure;
        sourceStructure << t1
                        << c1;

        ModelStructure outputStructure;
        outputStructure << inbox
                        << t1
                        << c1;

        QTest::newRow( "collection and project order" ) << sourceStructure
                                                        << outputStructure;

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t1
                        << _+t2
                        << _+t3;

        outputStructure.clear();
        outputStructure << inbox
                        << c1
                        << _+t1
                        << __+t2
                        << __+t3;

        QTest::newRow( "project and todo order" ) << sourceStructure
                                                  << outputStructure;

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t1
                        << _+t4
                        << _+t2;

        outputStructure.clear();
        outputStructure << inbox
                        << c1
                        << _+t1
                        << __+t4
                        << __+t2;

        QTest::newRow( "order between two todo" ) << sourceStructure
                                                  << outputStructure;

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t1
                        << _+t5
                        << _+t3;

        outputStructure.clear();
        outputStructure << inbox
                        << c1
                        << _+t1
                        << __+t5
                        << __+t3;

        QTest::newRow( "order between two project" ) << sourceStructure
                                                     << outputStructure;

        sourceStructure.clear();
        sourceStructure << c2
                        << c1;

        outputStructure.clear();
        outputStructure << inbox
                        << c2
                        << c1;

        QTest::newRow( "order between two collection" ) << sourceStructure
                                                        << outputStructure;
    }

    void shouldKeepSourceOrder()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        ModelUtils::create(&source, sourceStructure);

        //create todoTreeModel
        ReparentingModel todoTreeModel(new ProjectStrategy());
        ModelTest t1(&todoTreeModel);

        //WHEN
        todoTreeModel.setSourceModel(&source);

        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);

        QCOMPARE(todoTreeModel, output);
    }

    void shouldRetrieveItemFlags_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelPath>( "itemPath" );
        QTest::addColumn<int>( "column" );
        QTest::addColumn<int>( "flags" );

        C c1(1, 0, "c1");
        T t1(1, 1, "t1", QString(), "t1", InProgress, ProjectTag);
        T t2(2, 1, "t2", "t1", "t2");
        V inbox(Inbox);

        ModelStructure sourceStructure;
        sourceStructure << c1;

        int flags = Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
        int column = 0;
        ModelPath itemPath = c1;

        QTest::newRow( "get flags on collection" ) << sourceStructure << itemPath << column << flags;

        sourceStructure.clear();
        sourceStructure << t1 << t2;

        flags = Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
        itemPath = t1 % t2;

        QTest::newRow( "get flags on todo" ) << sourceStructure << itemPath << column << flags;

        flags = Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
        column = 4;

        QTest::newRow( "get flags on todo on column 4" ) << sourceStructure << itemPath << column << flags;

        sourceStructure.clear();
        sourceStructure << t1;

        itemPath = t1;

        column = 0;
        flags = Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

        QTest::newRow( "get flags on project" ) << sourceStructure << itemPath << column << flags;

        itemPath = inbox;

        flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

        QTest::newRow( "get flags on inbox" ) << sourceStructure << itemPath << column << flags;
    }

    void shouldRetrieveItemFlags()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        ModelUtils::create(&source, sourceStructure);

        //create metadataModel
        ReparentingModel todoTreeModel(new ProjectStrategy);
        ModelTest t1(&todoTreeModel);

        //WHEN
        todoTreeModel.setSourceModel(&source);

        //THEN
        QFETCH(ModelPath, itemPath);
        QFETCH(int, column);
        QFETCH(int, flags);

        Helper::printModel(&todoTreeModel);
        
        //If this fails we likely forgot to return the proper Zanshin::ItemType TODO write an explicit test to check if that is done properly
        QModelIndex index = ModelUtils::locateItem(&todoTreeModel, itemPath);
        index = index.sibling(index.row(), column);

        QCOMPARE(todoTreeModel.flags(index), flags);
    }

    void shouldReactToSourceRowRemovalsWithMetadata_data()
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
        T t5(7, 2, "t5", "", "t5");
        T t6(8, 2, "t6", "t5", "t6");

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

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t5
                        << _+t6;

        itemsToRemove.clear();
        itemsToRemove << c1 % t6;

        outputStructure.clear();
        outputStructure << inbox
                        << _+t5
                        << c1;

        QTest::newRow( "Root Project without tag should return to inbox when it looses its children" )
                                            << sourceStructure << itemsToRemove << outputStructure;
    }

    void shouldReactToSourceRowRemovalsWithMetadata()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        StandardModelBuilderBehavior behavior;
        behavior.setMetadataCreationEnabled(false);
        ModelUtils::create(&source, sourceStructure, ModelPath(), &behavior);

        //create metadataModel
        TodoMetadataModel metadataModel;
        ModelTest t1(&metadataModel);

        metadataModel.setSourceModel(&source);

        //create treeModel
        ReparentingModel treeModel(new ProjectStrategy());
        ModelTest t2(&treeModel);

        treeModel.setSourceModel(&metadataModel);

        Helper::printModel(&treeModel);

        //WHEN
        QFETCH(ModelPath::List, itemsToRemove);

        // destroy the item selected
        ModelUtils::destroy(&source, itemsToRemove);
        Helper::printModel(&treeModel);

        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);

        QCOMPARE(treeModel, output);
    }

    void shouldReparentOnTodoPromotion()
    {
        //GIVEN
        V inbox(Inbox);
        C c1(1, 0, "c1");
        T t1(3, 1, "t1", QString(), "t1");
        T t2(4, 1, "t2", QString(), "t2");

        // Source structure
        ModelStructure sourceStructure;
        sourceStructure << c1
                        << _+t1
                        << _+t2;

        //Source model
        QStandardItemModel source;
        ModelUtils::create(&source, sourceStructure);

        //create treeModel
        ReparentingModel treeModel(new ProjectStrategy());
        ModelTest modelTest(&treeModel);

        treeModel.setSourceModel(&source);

        //WHEN
        ModelPath itemToPromote = c1 % t1;
        QModelIndex index = ModelUtils::locateItem(&source, itemToPromote);
        source.setData(index, Zanshin::ProjectTodo, Zanshin::ItemTypeRole);

        //THEN
        ModelStructure outputStructure;
        outputStructure << inbox
                        << _+t2
                        << c1
                        << _+t1;

        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);

        QCOMPARE(treeModel, output);
    }
    
    void shouldPromoteTodosWithSubtodos()
    {
        //GIVEN
        V inbox(Inbox);
        C c1(1, 0, "c1");
        T t1(3, 1, "t1", QString(), "t1");
        T t2(4, 1, "t2", "t1", "t2");

        // Source structure
        ModelStructure sourceStructure;
        sourceStructure << c1
                        << _+t1
                        << _+t2;

        //Source model
        QStandardItemModel source;
        ModelUtils::create(&source, sourceStructure);

        //create treeModel
        ReparentingModel treeModel(new ProjectStrategy());
        ModelTest modelTest(&treeModel);

        treeModel.setSourceModel(&source);

        //WHEN
        ModelPath itemToPromote = c1 % t1;
        QModelIndex index = ModelUtils::locateItem(&treeModel, itemToPromote);
        QVERIFY(treeModel.data(index, Zanshin::ItemTypeRole).toInt() == Zanshin::ProjectTodo);

        //THEN
        ModelStructure outputStructure;
        outputStructure << inbox
                        << c1
                        << _+t1
                        << __+t2;

        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);

        QCOMPARE(treeModel, output);
    }

    void shouldReactToModelReset_data()
    {
        QTest::addColumn<ModelStructure>("sourceStructure");

        // Base items
        C c1(1, 0, "c1");
        T t1(1, 1, "t1", QString(), "t1", InProgress, ProjectTag);
        T t2(2, 1, "t2", "t1", "t2");

        // Create the source structure once and for all
        ModelStructure sourceStructure;
        sourceStructure << c1
                        << _+t1
                        << _+t2;

        QTest::newRow("clear model") << sourceStructure;
    }


    void shouldReactToModelReset()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        MockModel source;
        ModelUtils::create(&source, sourceStructure);

        //create treeModel
        ReparentingModel treeModel(new ProjectStrategy());
        ModelTest t2(&treeModel);

        treeModel.setSourceModel(&source);

        //WHEN
        source.clearData();

        //THEN
        MockModel output;
        ReparentingModel *treeModelOutput = new ReparentingModel(new ProjectStrategy);

        treeModelOutput->setSourceModel(&output);

        QAbstractItemModel* abstractModel = treeModelOutput;
        QCOMPARE(treeModel, *abstractModel);
        delete treeModelOutput;
    }
};

QTEST_KDEMAIN(TodoTreeModelSpec, GUI)

#include "todotreemodelspec.moc"
