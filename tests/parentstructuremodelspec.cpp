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

#include "reparentingmodel.h"
#include "reparentingstrategy.h"

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
    
    void parentsBeforeItems_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelStructure>( "outputStructure" );
        
        // Base items
        G inbox(1, Zanshin::ItemTypeRole, Zanshin::Inbox);
        inbox.data.insert(Qt::DisplayRole, "No Topic");
        G root(2, Zanshin::ItemTypeRole, Zanshin::TopicRoot);
        root.data.insert(Qt::DisplayRole, "Topics");
        
        G p1(8);
        p1.data.insert(TestParentStructureStrategy::TopicRole, 1);
        p1.data.insert(Qt::DisplayRole, "topic1");
        G p2(9);
        p2.data.insert(TestParentStructureStrategy::TopicRole, 2);
        p2.data.insert(Qt::DisplayRole, "topic2");
        
        G t1(3, TestParentStructureStrategy::TopicParentRole, 1);
        G t2(4, TestParentStructureStrategy::TopicParentRole, 1);
        G t3(5, TestParentStructureStrategy::TopicParentRole, 2);
        G t4(6, TestParentStructureStrategy::TopicParentRole, 2);
        G t5(7);
        
        
        // Create the source structure once and for all
        ModelStructure sourceStructure;
        sourceStructure 
        << t1
        << t2
        << t3
        << t4
        << t5;
        
        ModelStructure outputStructure;
        outputStructure 
        << inbox
        << _+t5
        << root
        << _+p1
        << __+t1
        << __+t2
        << _+p2
        << __+t3
        << __+t4;
        
        QTest::newRow( "nominal case" ) << sourceStructure << outputStructure;
    }
    
    void parentsBeforeItems()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);
        
        //Source model
        QStandardItemModel source;

        TestParentStructureStrategy *testadapter = new TestParentStructureStrategy();

        ReparentingModel categoriesModel(testadapter, this);
                
        categoriesModel.setSourceModel(&source);

        //Parents
        testadapter->addParent(1, -1, "topic1");
        testadapter->addParent(2, -1, "topic2");
        
        //Items
        ModelUtils::create(&source, sourceStructure);

        ModelTest t1(&categoriesModel); //The sourcemodel must be populated for the test to pass
        
        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);
        
        QCOMPARE(categoriesModel, output);
    }
    
    void itemsBeforeParents_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelStructure>( "outputStructure" );
        
        // Base items
        G inbox(1, Zanshin::ItemTypeRole, Zanshin::Inbox);
        inbox.data.insert(Qt::DisplayRole, "No Topic");
        G root(2, Zanshin::ItemTypeRole, Zanshin::TopicRoot);
        root.data.insert(Qt::DisplayRole, "Topics");
        
        G p1(8);
        p1.data.insert(TestParentStructureStrategy::TopicRole, 1);
        p1.data.insert(Qt::DisplayRole, "topic1");
        G p2(9);
        p2.data.insert(TestParentStructureStrategy::TopicRole, 2);
        p2.data.insert(Qt::DisplayRole, "topic2");
        
        G t1(3, TestParentStructureStrategy::TopicParentRole, 1);
        G t2(4, TestParentStructureStrategy::TopicParentRole, 1);
        G t3(5, TestParentStructureStrategy::TopicParentRole, 2);
        G t4(6, TestParentStructureStrategy::TopicParentRole, 2);
        G t5(7);
        
        // Create the source structure once and for all
        ModelStructure sourceStructure;
        sourceStructure 
        << t1
        << t2
        << t3
        << t4
        << t5;
        
        ModelStructure outputStructure;
        outputStructure 
        << inbox
        << _+t5
        << root
        << _+p1
        << __+t1
        << __+t2
        << _+p2
        << __+t3
        << __+t4;
        
        QTest::newRow( "nominal case" ) << sourceStructure << outputStructure;
    }
    
    void itemsBeforeParents()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);
        
        //Source model
        QStandardItemModel source;
        
        TestParentStructureStrategy *testadapter = new TestParentStructureStrategy();
        
        ReparentingModel categoriesModel(testadapter, this);
        
        categoriesModel.setSourceModel(&source);
                
        //items
        ModelUtils::create(&source, sourceStructure);
        
        ModelTest t1(&categoriesModel); //The sourcemodel must be populated for the test to pass

//         Helper::printModel(&source);
//         Helper::printModel(&categoriesModel);
        
        //parents
        testadapter->addParent(1, -1, "topic1");
        testadapter->addParent(2, -1, "topic2");

//         Helper::printModel(&source);
//         Helper::printModel(&categoriesModel);
                
        ModelTest t2(&categoriesModel);
        
        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);
        
        QCOMPARE(categoriesModel, output);
    }
    
    void shouldReparentFromInbox_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelStructure>( "intermediateStructure" );
        QTest::addColumn<ModelStructure>( "outputStructure" );
        QTest::addColumn<ModelPath::List>( "itemsToChangeTopic1" );
        
        // Base items
        G inbox(1, Zanshin::ItemTypeRole, Zanshin::Inbox);
        inbox.data.insert(Qt::DisplayRole, "No Topic");
        G root(2, Zanshin::ItemTypeRole, Zanshin::TopicRoot);
        root.data.insert(Qt::DisplayRole, "Topics");
        
        G p1(8);
        p1.data.insert(TestParentStructureStrategy::TopicRole, 1);
        p1.data.insert(Qt::DisplayRole, "topic1");
        G p2(9);
        p2.data.insert(TestParentStructureStrategy::TopicRole, 2);
        p2.data.insert(Qt::DisplayRole, "topic2");
        
        G t1(3);
        G t2(4);
        G t3(5);
        G t4(6);
        G t5(7);
        
        
        // Create the source structure once and for all
        ModelStructure sourceStructure;
        sourceStructure 
        << t1
        << t2
        << t3
        << t4
        << t5;
        
        ModelPath::List itemsToChangeTopic1;
        itemsToChangeTopic1 << t1;
        itemsToChangeTopic1 << t2;
        itemsToChangeTopic1 << t3;
        itemsToChangeTopic1 << t5;
        
        ModelStructure intermediateStructure;
        intermediateStructure 
        << inbox
        << _+t1
        << _+t2
        << _+t3
        << _+t4
        << _+t5
        << root;
        
        ModelStructure outputStructure;
        outputStructure 
        << inbox
        << _+t4
        << root
        << _+p1
        << __+t1
        << __+t2
        << __+t3
        << __+t5
        << _+p2;
        
        
        QTest::newRow( "nominal case" ) << sourceStructure << intermediateStructure << outputStructure << itemsToChangeTopic1;
    }
    
    void shouldReparentFromInbox()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);
        
        //Source model
        QStandardItemModel source;
        
        TestParentStructureStrategy *testadapter = new TestParentStructureStrategy();
        
        ReparentingModel categoriesModel(testadapter, this);
        
        categoriesModel.setSourceModel(&source);
        
        //items
        ModelUtils::create(&source, sourceStructure);
        
        ModelTest t1(&categoriesModel);
        
        QStandardItemModel intermediate;
        QFETCH(ModelStructure, intermediateStructure);
        ModelUtils::create(&intermediate, intermediateStructure);
        QCOMPARE(categoriesModel, intermediate);
        
        //parents
        testadapter->addParent(1, -1, "topic1");
        testadapter->addParent(2, -1, "topic2");
                
        //Reparent items
        QFETCH(ModelPath::List, itemsToChangeTopic1);
        foreach (const ModelPath &itemToChange, itemsToChangeTopic1) {
            QModelIndex index = ModelUtils::locateItem(&source, itemToChange);
            Q_ASSERT(index.isValid());
            source.setData(index, 1, TestParentStructureStrategy::TopicParentRole);
        }
                
        ModelTest t2(&categoriesModel);
        
        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);
        
        QCOMPARE(categoriesModel, output);
        
    }
    
    
    void reparentParents_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelStructure>( "outputStructure" );
        
        // Base items
        G inbox(1, Zanshin::ItemTypeRole, Zanshin::Inbox);
        inbox.data.insert(Qt::DisplayRole, "No Topic");
        G root(2, Zanshin::ItemTypeRole, Zanshin::TopicRoot);
        root.data.insert(Qt::DisplayRole, "Topics");
        
        G p1(8);
        p1.data.insert(TestParentStructureStrategy::TopicRole, 1);
        p1.data.insert(Qt::DisplayRole, "topic1");
        G p2(9);
        p2.data.insert(TestParentStructureStrategy::TopicRole, 2);
        p2.data.insert(TestParentStructureStrategy::TopicParentRole, 1);
        p2.data.insert(Qt::DisplayRole, "topic2");
        
        G t1(3, TestParentStructureStrategy::TopicParentRole, 1);
        G t2(4, TestParentStructureStrategy::TopicParentRole, 1);
        G t3(5, TestParentStructureStrategy::TopicParentRole, 2);
        G t4(6, TestParentStructureStrategy::TopicParentRole, 2);
        G t5(7);
        
        
        // Create the source structure once and for all
        ModelStructure sourceStructure;
        sourceStructure 
        << t1
        << t2
        << t3
        << t4
        << t5;
        
        ModelStructure outputStructure;
        outputStructure 
        << inbox
        << _+t5
        << root
        << _+p1
        << __+p2
        << ___+t3
        << ___+t4
        << __+t1
        << __+t2;
        
        
        QTest::newRow( "nominal case" ) << sourceStructure << outputStructure;
    }
    
    void reparentParents()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);
        
        //Source model
        QStandardItemModel source;
        
        TestParentStructureStrategy *testadapter = new TestParentStructureStrategy();
        
        ReparentingModel categoriesModel(testadapter, this);
        
        categoriesModel.setSourceModel(&source);
        
        //Parents
        testadapter->addParent(1, -1, "topic1");
        testadapter->addParent(2, 1, "topic2");
        
        //Items
        ModelUtils::create(&source, sourceStructure);
        
        ModelTest t1(&categoriesModel); //The sourcemodel must be populated for the test to pass
        
        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);
        
        QCOMPARE(categoriesModel, output);
    }
    
    void reparentingStructures_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelStructure>( "outputStructure" );
        
        // Base items
        G inbox(1, Zanshin::ItemTypeRole, Zanshin::Inbox);
        inbox.data.insert(Qt::DisplayRole, "No Topic");
        G root(2, Zanshin::ItemTypeRole, Zanshin::TopicRoot);
        root.data.insert(Qt::DisplayRole, "Topics");
        
        G p1(8, TestParentStructureStrategy::TopicRole, 1);
        p1.data.insert(Qt::DisplayRole, "topic1");
        G p2(9, TestParentStructureStrategy::TopicRole, 2);
        p2.data.insert(TestParentStructureStrategy::TopicParentRole, 1);
        p2.data.insert(Qt::DisplayRole, "topic2");
        G p3(10, TestParentStructureStrategy::TopicRole, 3);
        p3.data.insert(TestParentStructureStrategy::TopicParentRole, 2);
        p3.data.insert(Qt::DisplayRole, "topic3");
        
        G t1(3, TestParentStructureStrategy::TopicParentRole, 1);
        G t2(4, TestParentStructureStrategy::TopicParentRole, 1);
        G t3(5, TestParentStructureStrategy::TopicParentRole, 2);
        G t4(6, TestParentStructureStrategy::TopicParentRole, 2);
        G t5(7, TestParentStructureStrategy::TopicParentRole, 3);
        
        
        // Create the source structure once and for all
        ModelStructure sourceStructure;
        sourceStructure 
        << t1
        << t2
        << t3
        << t4
        << t5;
        
        ModelStructure outputStructure;
        outputStructure 
        << inbox
        << root
        << _+p1
        << __+p2
        << ___+p3
        << ____+t5
        << ___+t3
        << ___+t4
        << __+t1
        << __+t2;
        
        
        QTest::newRow( "nominal case" ) << sourceStructure << outputStructure;
    }
    
    void reparentingStructures()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);
        
        //Source model
        QStandardItemModel source;
        
        TestParentStructureStrategy *testadapter = new TestParentStructureStrategy();
        
        ReparentingModel categoriesModel(testadapter, this);
        
        categoriesModel.setSourceModel(&source);
        
        //Parents
        testadapter->addParent(1, -1, "topic1");
        testadapter->addParent(3, 2, "topic3");
        testadapter->addParent(2, 1, "topic2");
        
//         Helper::printModel(&categoriesModel);
        
        //Items
        ModelUtils::create(&source, sourceStructure);
        
        ModelTest t1(&categoriesModel); //The sourcemodel must be populated for the test to pass
        
        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);
        
        QCOMPARE(categoriesModel, output);
    }
    
    void onNodeRemoval_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelStructure>( "inputStructure" );
        QTest::addColumn<ModelStructure>( "outputStructure" );
        
        // Base items
        G inbox(1, Zanshin::ItemTypeRole, Zanshin::Inbox);
        inbox.data.insert(Qt::DisplayRole, "No Topic");
        G root(2, Zanshin::ItemTypeRole, Zanshin::TopicRoot);
        root.data.insert(Qt::DisplayRole, "Topics");
        
        G p1(8, TestParentStructureStrategy::TopicRole, 1);
        p1.data.insert(Qt::DisplayRole, "topic1");
        G p2(9, TestParentStructureStrategy::TopicRole, 2);
        p2.data.insert(TestParentStructureStrategy::TopicParentRole, 1);
        p2.data.insert(Qt::DisplayRole, "topic2");
        G p3(10, TestParentStructureStrategy::TopicRole, 3);
        p3.data.insert(TestParentStructureStrategy::TopicParentRole, 2);
        p3.data.insert(Qt::DisplayRole, "topic3");
        
        G t1(3, TestParentStructureStrategy::TopicParentRole, 1);
        G t2(4, TestParentStructureStrategy::TopicParentRole, 1);
        G t3(5, TestParentStructureStrategy::TopicParentRole, 2);
        G t4(6, TestParentStructureStrategy::TopicParentRole, 2);
        G t5(7, TestParentStructureStrategy::TopicParentRole, 3);
        
        
        // Create the source structure once and for all
        ModelStructure sourceStructure;
        sourceStructure 
        << t1
        << t2
        << t3
        << t4
        << t5;
        
        ModelStructure inputStructure;
        inputStructure 
        << inbox
        << root
        << _+p1
        << __+p2
        << ___+p3
        << ____+t5
        << ___+t3
        << ___+t4
        << __+t1
        << __+t2;
                
        ModelStructure outputStructure;
        outputStructure 
        << inbox
        << _+t5
        << _+t3
        << _+t4
        << root
        << _+p1
        << __+t1
        << __+t2;
        
        
        QTest::newRow( "nominal case" ) << sourceStructure << inputStructure << outputStructure;
    }
    
    //Move back to inbox after node removal
    void onNodeRemoval()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);
        
        //Source model
        QStandardItemModel source;
        
        TestParentStructureStrategy *testadapter = new TestParentStructureStrategy();
        
        ReparentingModel categoriesModel(testadapter, this);
        
        categoriesModel.setSourceModel(&source);
        
        //Parents
        testadapter->addParent(1, -1, "topic1");
        testadapter->addParent(2, 1, "topic2");
        testadapter->addParent(3, 2, "topic3");
        
//         Helper::printModel(&categoriesModel);
        
        //Items
        ModelUtils::create(&source, sourceStructure);
        
        ModelTest t1(&categoriesModel); //The sourcemodel must be populated for the test to pass
        
        //THEN
        QFETCH(ModelStructure, inputStructure);
        QStandardItemModel input;
        ModelUtils::create(&input, inputStructure);
        
        QCOMPARE(categoriesModel, input);
        
        testadapter->removeParent(2);
        
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);

        Helper::printModel(&categoriesModel);
        Helper::printModel(&output);
        
        QCOMPARE(categoriesModel, output);
    }
    
    
    void reparentFromInbox_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelStructure>( "inputStructure" );
        QTest::addColumn<ModelStructure>( "outputStructure" );
        QTest::addColumn<ModelPath>( "itemToChange" );
        
        // Base items
        G inbox(1, Zanshin::ItemTypeRole, Zanshin::Inbox);
        inbox.data.insert(Qt::DisplayRole, "No Topic");
        G root(2, Zanshin::ItemTypeRole, Zanshin::TopicRoot);
        root.data.insert(Qt::DisplayRole, "Topics");
        
        G p1(8, TestParentStructureStrategy::TopicRole, 1);
        p1.data.insert(Qt::DisplayRole, "topic1");
        
        G t1(3);
        G t2(4);
        
        
        // Create the source structure once and for all
        ModelStructure sourceStructure;
        sourceStructure 
        << t1
        << t2;
        
        ModelPath itemToChange(t1);
        
        ModelStructure inputStructure;
        inputStructure 
        << inbox
        << _+t1
        << _+t2
        << root
        << _+p1;
        
        ModelStructure outputStructure;
        outputStructure 
        << inbox
        << _+t2
        << root
        << _+p1
        << __+t1;
        
        
        QTest::newRow( "nominal case" ) << sourceStructure << inputStructure << outputStructure << itemToChange;
    }
    
    //move item out of inbox if parent changes
    void reparentFromInbox()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);
        
        //Source model
        QStandardItemModel source;
        
        TestParentStructureStrategy *testadapter = new TestParentStructureStrategy();
        
        ReparentingModel categoriesModel(testadapter, this);
        
        categoriesModel.setSourceModel(&source);
        
        //Parents
        testadapter->addParent(1, -1, "topic1");
        
        Helper::printModel(&categoriesModel);
        
        //Items
        ModelUtils::create(&source, sourceStructure);
        
        ModelTest t1(&categoriesModel); //The sourcemodel must be populated for the test to pass
        
        //THEN
        QFETCH(ModelStructure, inputStructure);
        QStandardItemModel input;
        ModelUtils::create(&input, inputStructure);
        
        QCOMPARE(categoriesModel, input);
        
        QFETCH(ModelPath, itemToChange);
        QModelIndex index = ModelUtils::locateItem(&source, itemToChange);
        Q_ASSERT(index.isValid());
        testadapter->setParent(index, 1);
        
        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);
        
        QCOMPARE(categoriesModel, output);
    }
    
    

};

QTEST_KDEMAIN(ReparentingModelSpec, GUI)

#include "topicsmodelspec.moc"
