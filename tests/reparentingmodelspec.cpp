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


            QTest::newRow( "unordered tree" ) << sourceStructure << outputStructure << ModelStructure() << ModelPath();
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

        QCOMPARE(model, output);
    }

    void handleMoves_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelStructure>( "outputStructure" );
        QTest::addColumn<ModelPath>( "itemToChange" );
        QTest::addColumn<Id>( "parentId" );

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

            ModelPath itemToChange = t3;
            Id parentId = 3;

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
            Id parentId = -1;

            ModelStructure outputStructure;
            outputStructure
            << p1
            << _+t1
            << _+t2
            << p2
            << t5
            << t3
            << _+t4;


            QTest::newRow( "move toplevel" ) << sourceStructure << outputStructure << itemToChange << parentId;
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
        QFETCH(Id, parentId);
        source.setData(index, parentId, TestReparentingStrategy::ParentRole);

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
