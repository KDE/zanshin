/* This file is part of Zanshin Todo.
 *
 *   Copyright 2012 Christian Mollekopf
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License or (at your option) version 3 or any later version
 *   accepted by the membership of KDE e.V. (or its successor approved
 *   by the membership of KDE e.V.), which shall act as a proxy
 *   defined in Section 14 of version 3 of the license.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 *   USA.
 */

#include <qtest_kde.h>

#include "reparentingmodel/reparentingmodel.h"
#include "reparentingmodel/reparentingstrategy.h"

#include <globaldefs.h>
#include "testlib/testlib.h"
#include "testlib/mockmodel.h"
#include "testlib/modelbuilderbehavior.h"
#include "testlib/helperutils.h"

#include <QtGui/QTreeView>
#include <QtCore/QEventLoop>

using namespace Zanshin::Test;

Q_DECLARE_METATYPE(QModelIndex)

/*
 * Test for the reparenting capabilities of the ReparentingModel without virtual nodes.
 */
class ReparentingModelSpec : public QObject
{
    Q_OBJECT
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

    void flatModel_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelStructure>( "outputStructure" );

        G t1(1, TestReparentingStrategy::IdRole, 1);
        G t2(2, TestReparentingStrategy::IdRole, 2);
        G t3(3, TestReparentingStrategy::IdRole, 3);
        G t4(4, TestReparentingStrategy::IdRole, 4);
        G t5(5, TestReparentingStrategy::IdRole, 5);

        {
            // Create the source structure once and for all
            ModelStructure sourceStructure;
            sourceStructure
            << t1
            << t2
            << t3
            << t4
            << t5;

            QTest::newRow( "identity case" ) << sourceStructure << sourceStructure;
        }

        {
            // Create the source structure once and for all
            ModelStructure sourceStructure;
            sourceStructure
            << t1
            << _+t2
            << _+t3
            << __+t4
            << t5;


            ModelStructure outputStructure;
            outputStructure
            << t1
            << t2
            << t3
            << t4
            << t5;

            QTest::newRow( "flattening case" ) << sourceStructure << outputStructure;
        }
    }

    void flatModel()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;

        TestReparentingStrategy *strategy = new TestReparentingStrategy();

        ReparentingModel model(strategy, this);

        model.setSourceModel(&source);

        //Items
        ModelUtils::create(&source, sourceStructure);

        ModelTest t1(&model); //The sourcemodel must be populated for the test to pass

        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);

        QCOMPARE(model, output);
    }

    void buildTree_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelStructure>( "outputStructure" );
        QTest::addColumn<ModelStructure>( "insertedStructure" );
        QTest::addColumn<ModelPath>( "sourceParentPath" );

        G p1(1, TestReparentingStrategy::IdRole, 1);
        G p2(2, TestReparentingStrategy::IdRole, 2);
        G t1(3, TestReparentingStrategy::IdRole, 3);
        t1.data.insert(TestReparentingStrategy::ParentRole, 1);
        G t2(4, TestReparentingStrategy::IdRole, 4);
        t2.data.insert(TestReparentingStrategy::ParentRole, 1);
        G t3(5, TestReparentingStrategy::IdRole, 5);
        t3.data.insert(TestReparentingStrategy::ParentRole, 2);
        G t4(6, TestReparentingStrategy::IdRole, 6);
        t4.data.insert(TestReparentingStrategy::ParentRole, 5);
        G t5(7, TestReparentingStrategy::IdRole, 7);
        {
            ModelStructure sourceStructure;
            sourceStructure
            << p1
            << t1
            << t2
            << p2
            << t3
            << t4
            << t5;

            ModelStructure outputStructure;
            outputStructure
            << p1
            << _+t1
            << _+t2
            << p2
            << _+t3
            << __+t4
            << t5;


            QTest::newRow( "simple tree" ) << sourceStructure << outputStructure << ModelStructure() << ModelPath();
        }
        {
            ModelStructure sourceStructure;
            sourceStructure
            << t1
            << p1;


            ModelStructure outputStructure;
            outputStructure
            << p1
            << _+t1;

            QTest::newRow( "unordered tree simple" ) << sourceStructure << outputStructure << ModelStructure() << ModelPath();
        }
        {
            ModelStructure sourceStructure;
            sourceStructure
            << t1
            << t2
            << p1
            << p2
            << t5
            << t4
            << t3;


            ModelStructure outputStructure;
            outputStructure
            << p1
            << _+t1
            << _+t2
            << p2
            << _+t3
            << __+t4
            << t5;

            QTest::newRow( "unordered tree complex" ) << sourceStructure << outputStructure << ModelStructure() << ModelPath();
        }
        {
            ModelStructure sourceStructure;
            sourceStructure
            << t1
            << t2
            << p1
            << p2
            << t5;


            ModelStructure outputStructure;
            outputStructure
            << p1
            << _+t1
            << _+t2
            << p2
            << _+t3
            << __+t4
            << t5;

            ModelStructure insertStructure;
            insertStructure
            << t3
            << t4;

            ModelPath sourceParentPath = p2;


            QTest::newRow( "insert parents" ) << sourceStructure << outputStructure << insertStructure << sourceParentPath;
        }
        {
            G p3(1, TestReparentingStrategy::IdRole, 1);
            G p4(2, TestReparentingStrategy::IdRole, 2);
            G t6(3, TestReparentingStrategy::IdRole, 3);
            t6.data.insert(TestReparentingStrategy::ParentListRole, QVariant::fromValue<IdList>(IdList() << 1 << 2));

            ModelStructure sourceStructure;
            sourceStructure
            << p3
            << p4
            << t6;

            ModelStructure outputStructure;
            outputStructure
            << p3
            << _+t6
            << p4
            << _+t6;

            QTest::newRow( "simple multiparent tree" ) << sourceStructure << outputStructure << ModelStructure() << ModelPath();
        }
        {
        /*
         * Not sure if this test covers anything which the previous tests didn't. It's just a more complex scenario.
         * 
         * TODO it's currently not possible to have an item both toplevel and somewhere in the tree because: toplevel == empty parentlist (-1 for toplevel ?)
         */
            G p3(1, TestReparentingStrategy::IdRole, 1);
            G p4(2, TestReparentingStrategy::IdRole, 2);
            G t6(3, TestReparentingStrategy::IdRole, 3);
            t6.data.insert(TestReparentingStrategy::ParentListRole, QVariant::fromValue<IdList>(IdList() << 1 << 2));
            G t7(4, TestReparentingStrategy::IdRole, 4);
            t7.data.insert(TestReparentingStrategy::ParentRole, 3);
            G t8(5, TestReparentingStrategy::IdRole, 5);
            t8.data.insert(TestReparentingStrategy::ParentRole, 2);
            G t9(6, TestReparentingStrategy::IdRole, 6);
            t9.data.insert(TestReparentingStrategy::ParentRole, 5);
            G t10(7, TestReparentingStrategy::IdRole, 7);
            t10.data.insert(TestReparentingStrategy::ParentListRole, QVariant::fromValue<IdList>(IdList() << 5));
            ModelStructure sourceStructure;
            sourceStructure
            << p3
            << t6
            << t7
            << p4
            << t8
            << t9
            << t10;

            ModelStructure outputStructure;
            outputStructure
            << p3
            << _+t6
            << __+t7
            << p4
            << _+t6
            << __+t7
            << _+t8
            << __+t9
            << __+t10
            /*<< t10*/;


            QTest::newRow( "multiparent tree" ) << sourceStructure << outputStructure << ModelStructure() << ModelPath();
        }
        
    }

    void buildTree()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;

        TestReparentingStrategy *strategy = new TestReparentingStrategy();

        ReparentingModel model(strategy, this);

        model.setSourceModel(&source);

        //Items
        ModelUtils::create(&source, sourceStructure);

        ModelTest t1(&model); //The sourcemodel must be populated for the test to pass

        //Insert
        QFETCH(ModelPath, sourceParentPath);
        QFETCH(ModelStructure, insertedStructure);
        // Collect data to ensure we signalled the outside properly
        //TODO check signals
//         QSignalSpy aboutToInsertSpy(&model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
//         QSignalSpy insertSpy(&model, SIGNAL(rowsInserted(QModelIndex,int,int)));
        ModelUtils::create(&source, insertedStructure, sourceParentPath);
        
        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);

        Helper::printModel(&model);
        Helper::printModel(&output);

        QCOMPARE(model, output);
    }

    void handleMoves_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelStructure>( "outputStructure" );
        QTest::addColumn<ModelPath>( "itemToChange" );
        QTest::addColumn<IdList>( "parentId" );

        G p1(1, TestReparentingStrategy::IdRole, 1);
        G p2(2, TestReparentingStrategy::IdRole, 2);
        G t1(3, TestReparentingStrategy::IdRole, 3);
        t1.data.insert(TestReparentingStrategy::ParentRole, 1);
        G t2(4, TestReparentingStrategy::IdRole, 4);
        t2.data.insert(TestReparentingStrategy::ParentRole, 1);
        G t3(5, TestReparentingStrategy::IdRole, 5);
        t3.data.insert(TestReparentingStrategy::ParentRole, 2);
        G t4(6, TestReparentingStrategy::IdRole, 6);
        t4.data.insert(TestReparentingStrategy::ParentRole, 5);
        G t5(7, TestReparentingStrategy::IdRole, 7);

        G p3(8, TestReparentingStrategy::IdRole, 8);
        p3.data.insert(TestReparentingStrategy::ParentListRole, QVariant::fromValue<IdList>(IdList() << 1 << 2));
        G t6(9, TestReparentingStrategy::IdRole, 9);
        t6.data.insert(TestReparentingStrategy::ParentRole, 8);
        G t7(10, TestReparentingStrategy::IdRole, 10);
        t7.data.insert(TestReparentingStrategy::ParentRole, 8);
        {
            ModelStructure sourceStructure;
            sourceStructure
            << t1
            << t2
            << p1
            << p2
            << t5
            << t4
            << t3;

            /*
            ModelStructure outputStructure;
            outputStructure
            << p1
            << _+t1
            << _+t2
            << p2
            << _+t3
            << __+t4
            << t5;
            */

            ModelPath itemToChange = t3;
            IdList parentId;
            parentId << t1.id;

            ModelStructure outputStructure;
            outputStructure
            << p1
            << _+t1
            << __+t3
            << ___+t4
            << _+t2
            << p2
            << t5;


            QTest::newRow( "reparent subtree" ) << sourceStructure << outputStructure << itemToChange << parentId;
        }
        {
            ModelStructure sourceStructure;
            sourceStructure
            << t1
            << t2
            << p1
            << p2
            << t5
            << t4
            << t3;

            /*
            ModelStructure outputStructure;
            outputStructure
            << p1
            << _+t1
            << _+t2
            << p2
            << _+t3
            << __+t4
            << t5;
            */

            ModelPath itemToChange = t3;
            IdList parentId;

            ModelStructure outputStructure;
            outputStructure
            << p1
            << _+t1
            << _+t2
            << p2
            << t5
            << t3
            << _+t4;


            QTest::newRow( "move to toplevel" ) << sourceStructure << outputStructure << itemToChange << parentId;
        }
        {
            ModelStructure sourceStructure;
            sourceStructure
            << t1
            << t2
            << p1
            << p2
            << t5
            << t4
            << t3;

            /*
            ModelStructure outputStructure;
            outputStructure
            << p1
            << _+t1
            << _+t2
            << p2
            << _+t3
            << __+t4
            << t5;
            */

            ModelPath itemToChange = p2;
            IdList parentId;
            parentId << p1.id;

            ModelStructure outputStructure;
            outputStructure
            << p1
            << _+t1
            << _+t2
            << _+p2
            << __+t3
            << ___+t4
            << t5;

            QTest::newRow( "reparent virtual node" ) << sourceStructure << outputStructure << itemToChange << parentId;
        }
        {
            ModelStructure sourceStructure;
            sourceStructure
            << t1
            << t2
            << p1
            << p2
            << t5;

            /*
            ModelStructure outputStructure;
            outputStructure
            << p1
            << _+t1
            << _+t2
            << p2
            << t5;
            */

            ModelPath itemToChange = p1;
            IdList parentId;
            parentId << p2.id << t5.id;

            ModelStructure outputStructure;
            outputStructure
            << p2
            << _+p1
            << __+t1
            << __+t2
            << t5
            << _+p1
            << __+t1
            << __+t2;

            QTest::newRow( "reparent twice with multiple children" ) << sourceStructure << outputStructure << itemToChange << parentId;
        }
        {
            ModelStructure sourceStructure;
            sourceStructure
            << p1
            << p2
            << p3
            << t6
            << t7;

            /*
            ModelStructure outputStructure;
            outputStructure
            << p1
            << _+p3
            << __+t6
            << __+t7
            << p2
            << _+p3
            << __+t6
            << __+t7
            */

            ModelPath itemToChange = p3;
            IdList parentId;

            ModelStructure outputStructure;
            outputStructure
            << p1
            << p2
            << p3
            << _+t6
            << _+t7;

            QTest::newRow( "reparent from multi to single toplevel" ) << sourceStructure << outputStructure << itemToChange << parentId;
        }
        {
            ModelStructure sourceStructure;
            sourceStructure
            << p1
            << p2
            << p3
            << t6
            << t7;

            /*
            ModelStructure outputStructure;
            outputStructure
            << p1
            << _+p3
            << __+t6
            << __+t7
            << p2
            << _+p3
            << __+t6
            << __+t7
            */

            ModelPath itemToChange = p3;
            IdList parentId;
            parentId << p2.id;

            ModelStructure outputStructure;
            outputStructure
            << p1
            << p2
            << _+p3
            << __+t6
            << __+t7;

            QTest::newRow( "reparent from multi to single" ) << sourceStructure << outputStructure << itemToChange << parentId;
        }
    }

    void handleMoves()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;

        TestReparentingStrategy *strategy = new TestReparentingStrategy();
        ReparentingModel model(strategy, this);
        model.setSourceModel(&source);

        //Items
        ModelUtils::create(&source, sourceStructure);

        ModelTest t1(&model); //The sourcemodel must be populated for the test to pass

        //Insert
        QFETCH(ModelPath, itemToChange);
        QModelIndex index = ModelUtils::locateItem(&source, itemToChange);
        QFETCH(IdList, parentId);
        source.setData(index, QVariant::fromValue<IdList>(parentId), TestReparentingStrategy::ParentListRole);

        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);

        QCOMPARE(model, output);
    }

    void handleRemoves_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelStructure>( "outputStructure" );
        QTest::addColumn<ModelPath>( "itemToRemove" );

        G p1(1, TestReparentingStrategy::IdRole, 1);
        G p2(2, TestReparentingStrategy::IdRole, 2);
        G t1(3, TestReparentingStrategy::IdRole, 3);
        t1.data.insert(TestReparentingStrategy::ParentRole, 1);
        G t2(4, TestReparentingStrategy::IdRole, 4);
        t2.data.insert(TestReparentingStrategy::ParentRole, 1);
        G t3(5, TestReparentingStrategy::IdRole, 5);
        t3.data.insert(TestReparentingStrategy::ParentRole, 2);
        G t4(6, TestReparentingStrategy::IdRole, 6);
        t4.data.insert(TestReparentingStrategy::ParentRole, 5);
        G t5(7, TestReparentingStrategy::IdRole, 7);
        {
            ModelStructure sourceStructure;
            sourceStructure
            << t1
            << t2
            << p1
            << p2
            << t5
            << t4
            << t3;

            /*
            ModelStructure outputStructure;
            outputStructure
            << p1
            << _+t1
            << _+t2
            << p2
            << _+t3
            << __+t4
            << t5;
            */

            ModelPath itemToRemove = t3;

            ModelStructure outputStructure;
            outputStructure
            << p1
            << _+t1
            << _+t2
            << p2
            << t5
            << t4;

            QTest::newRow( "remove single" ) << sourceStructure << outputStructure << itemToRemove;
        }
        {
            ModelStructure sourceStructure;
            sourceStructure
            << t1
            << t2
            << p1
            << p2
            << t5
            << t4
            << t3;

            /*
            ModelStructure outputStructure;
            outputStructure
            << p1
            << _+t1
            << _+t2
            << p2
            << _+t3
            << __+t4
            << t5;
            */

            ModelPath itemToRemove = p2;

            ModelStructure outputStructure;
            outputStructure
            << p1
            << _+t1
            << _+t2
            << t5
            << t3
            << _+t4;

            QTest::newRow( "remove toplevel" ) << sourceStructure << outputStructure << itemToRemove;
        }
    }

    void handleRemoves()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;

        TestReparentingStrategy *strategy = new TestReparentingStrategy();
        ReparentingModel model(strategy, this);
        model.setSourceModel(&source);

        //Items
        ModelUtils::create(&source, sourceStructure);

        ModelTest t1(&model); //The sourcemodel must be populated for the test to pass

        Helper::printModel(&model);

        //Insert
        QFETCH(ModelPath, itemToRemove);
        ModelUtils::destroy(&source, itemToRemove);

        Helper::printModel(&model);

        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);

        QCOMPARE(model, output);
    }
};

QTEST_KDEMAIN(ReparentingModelSpec, GUI)

#include "reparentingmodelspec.moc"
