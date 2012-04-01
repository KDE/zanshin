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

#include "topicsmodel.h"
#include "nepomukadapter.h"
#include <globaldefs.h>
#include "testlib/testlib.h"
#include "testlib/mockmodel.h"
#include "testlib/modelbuilderbehavior.h"
#include "testlib/helperutils.h"

#include <QtGui/QTreeView>
#include <QtCore/QEventLoop>

using namespace Zanshin::Test;

Q_DECLARE_METATYPE(QModelIndex)

class TopicsModelSpec : public QObject
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
        
        G p1(8, TestStructureAdapter::TopicParentRole, QString());
        p1.data.insert(TestStructureAdapter::TopicRole, "topic1");
        p1.data.insert(Qt::DisplayRole, "topic1");
        G p2(9, TestStructureAdapter::TopicParentRole, QString());
        p2.data.insert(TestStructureAdapter::TopicRole, "topic2");
        p2.data.insert(Qt::DisplayRole, "topic2");
        
        G t1(3, TestStructureAdapter::TopicParentRole, "topic1");
        G t2(4, TestStructureAdapter::TopicParentRole, "topic1");
        G t3(5, TestStructureAdapter::TopicParentRole, "topic2");
        G t4(6, TestStructureAdapter::TopicParentRole, "topic2");
        G t5(7, TestStructureAdapter::TopicParentRole, QString());
        
        
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

        TestStructureAdapter *testadapter = new TestStructureAdapter(this);

        TopicsModel categoriesModel(testadapter, this);        
                
        categoriesModel.setSourceModel(&source);

        //Parents
        testadapter->addParent("topic1", QString(), "topic1");
        testadapter->addParent("topic2", QString(), "topic2");
        
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
        
        G p1(8, TestStructureAdapter::TopicParentRole, QString());
        p1.data.insert(TestStructureAdapter::TopicRole, "topic1");
        p1.data.insert(Qt::DisplayRole, "topic1");
        G p2(9, TestStructureAdapter::TopicParentRole, QString());
        p2.data.insert(TestStructureAdapter::TopicRole, "topic2");
        p2.data.insert(Qt::DisplayRole, "topic2");
        
        G t1(3, TestStructureAdapter::TopicParentRole, "topic1");
        G t2(4, TestStructureAdapter::TopicParentRole, "topic1");
        G t3(5, TestStructureAdapter::TopicParentRole, "topic2");
        G t4(6, TestStructureAdapter::TopicParentRole, "topic2");
        G t5(7, TestStructureAdapter::TopicParentRole, QString());
        
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
        
        TestStructureAdapter *testadapter = new TestStructureAdapter(this);
        
        TopicsModel categoriesModel(testadapter, this);        
        
        categoriesModel.setSourceModel(&source);
                
        //items
        ModelUtils::create(&source, sourceStructure);
        
        ModelTest t1(&categoriesModel); //The sourcemodel must be populated for the test to pass
        
        //parents
        testadapter->addParent("topic1", QString(), "topic1");
        testadapter->addParent("topic2", QString(), "topic2");
                
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
        
        G p1(8, TestStructureAdapter::TopicParentRole, QString());
        p1.data.insert(TestStructureAdapter::TopicRole, "topic1");
        p1.data.insert(Qt::DisplayRole, "topic1");
        G p2(9, TestStructureAdapter::TopicParentRole, QString());
        p2.data.insert(TestStructureAdapter::TopicRole, "topic2");
        p2.data.insert(Qt::DisplayRole, "topic2");
        
        G t1(3, TestStructureAdapter::TopicParentRole, QString());
        G t2(4, TestStructureAdapter::TopicParentRole, QString());
        G t3(5, TestStructureAdapter::TopicParentRole, QString());
        G t4(6, TestStructureAdapter::TopicParentRole, QString());
        G t5(7, TestStructureAdapter::TopicParentRole, QString());
        
        
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
        
        TestStructureAdapter *testadapter = new TestStructureAdapter(this);
        
        TopicsModel categoriesModel(testadapter, this);        
        
        categoriesModel.setSourceModel(&source);
        
        //items
        ModelUtils::create(&source, sourceStructure);
        
        ModelTest t1(&categoriesModel);
        
        QStandardItemModel intermediate;
        QFETCH(ModelStructure, intermediateStructure);
        ModelUtils::create(&intermediate, intermediateStructure);
        QCOMPARE(categoriesModel, intermediate);
        
        //parents
        testadapter->addParent("topic1", QString(), "topic1");
        testadapter->addParent("topic2", QString(), "topic2");
                
        //Reparent items
        QFETCH(ModelPath::List, itemsToChangeTopic1);
        foreach (const ModelPath &itemToChange, itemsToChangeTopic1) {
            QModelIndex index = ModelUtils::locateItem(&source, itemToChange);
            Q_ASSERT(index.isValid());
            source.setData(index, "topic1", TestStructureAdapter::TopicParentRole);
        }
                
        ModelTest t2(&categoriesModel);
        
        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);
        
        QCOMPARE(categoriesModel, output);
        
    }
    
 /*   
    void shouldReactToSourceRowInserts_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelPath>( "sourceParentPath" );
        QTest::addColumn<ModelPath::List>( "sourceSiblingPaths" );
        QTest::addColumn<ModelStructure>( "insertedStructure" );
        QTest::addColumn<ModelStructure>( "outputStructure" );
        
        // Base items
        V nocat(NoCategory);
        V cats(Categories);
        C c1(1, 0, "c1");
        C c2(2, 0, "c2");
        Cat cat1("cat1");
        Cat cat2("cat2");
        T t1(3, 1, "t1", QString(), "t1", InProgress, ProjectTag, QString(), "cat1");
        T t2(4, 1, "t2", "t1", "t2", InProgress, NoTag, QString(), "cat1, cat2");
        T t3(5, 2, "t3", QString(), "t3", InProgress, ProjectTag, QString());
        T t4(6, 2, "t4", QString(), "t4", InProgress, NoTag, QString(), "cat1");
        T t5(6, 2, "t5", QString(), "t5", InProgress, NoTag);
        T t6(7, 1, "t6", QString(), "t6", InProgress, NoTag, QString(), "cat2");
        
        // Create the source structure once and for all
        ModelStructure sourceStructure;
        sourceStructure << c1
        << _+t1
        << _+t6
        << c2
        << _+t3
        << _+t4
        << _+t5;
        
        
        ModelPath sourceParentPath = c1;
        ModelPath::List sourceSiblingPaths;
        sourceSiblingPaths << c1 % t6 << c2 % t4;
        
        ModelStructure insertedStructure;
        insertedStructure << t2;
        
        ModelStructure outputStructure;
        outputStructure << nocat
        << _+t5
        << cats
        << _+cat2
        << __+t6
        << __+t2
        << _+cat1
        << __+t4
        << __+t2;
        
        QTest::newRow( "add todo with several categories" ) << sourceStructure << sourceParentPath
        << sourceSiblingPaths << insertedStructure
        << outputStructure;
    }
    
    void shouldReactToSourceRowInserts()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);
        
        //Source model
        QStandardItemModel source;
        //Kick up category manager
        CategoryManager::instance().setModel(&source);
        ModelUtils::create(&source, sourceStructure);
        
        //create treeModel
        TodoCategoriesModel categoriesModel;
        ModelTest t1(&categoriesModel);
        
        categoriesModel.setSourceModel(&source);
        
        // What row number will we expect?
        QFETCH(ModelPath::List, sourceSiblingPaths);
        
        QModelIndexList parentIndexes;
        QList<int> expectedRows;
        
        foreach (const ModelPath &sourceSiblingPath, sourceSiblingPaths) {
            QModelIndex sourceSibling = ModelUtils::locateItem(&source, sourceSiblingPath);
            QModelIndexList proxySiblings = categoriesModel.mapFromSourceAll(sourceSibling);
            Q_ASSERT(proxySiblings.size()==1);
            parentIndexes << proxySiblings.first().parent();
            expectedRows << categoriesModel.rowCount(parentIndexes.last());
        }
        
        //WHEN
        QFETCH(ModelPath, sourceParentPath);
        QFETCH(ModelStructure, insertedStructure);
        
        // Collect data to ensure we signalled the outside properly
        QSignalSpy aboutToInsertSpy(&categoriesModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
        QSignalSpy insertSpy(&categoriesModel, SIGNAL(rowsInserted(QModelIndex,int,int)));
        
        ModelUtils::create(&source, insertedStructure, sourceParentPath);
        
        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);
        
        QCOMPARE(categoriesModel, output);
        
        QCOMPARE(aboutToInsertSpy.size(), parentIndexes.count());
        QCOMPARE(insertSpy.size(), parentIndexes.count());
        
        for (int i=0; i<parentIndexes.count(); i++) {
            const QModelIndex parentIndex = parentIndexes.at(i);
            const int expectedRow = expectedRows.at(i);
            
            QCOMPARE(aboutToInsertSpy.at(i).at(0).value<QModelIndex>(), parentIndex);
            QCOMPARE(aboutToInsertSpy.at(i).at(1).toInt(), expectedRow);
            QCOMPARE(aboutToInsertSpy.at(i).at(2).toInt(), expectedRow);
            
            QCOMPARE(insertSpy.at(i).at(0).value<QModelIndex>(), parentIndex);
            QCOMPARE(insertSpy.at(i).at(1).toInt(), expectedRow);
            QCOMPARE(insertSpy.at(i).at(2).toInt(), expectedRow);
        }
    }
    
    void shouldReactToSourceDataChanges_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelPath>( "itemToChange" );
        QTest::addColumn<QVariant>( "value" );
        QTest::addColumn<int>( "role" );
        QTest::addColumn<ModelStructure>( "outputStructure" );
        
        // Base items
        V nocat(NoCategory);
        V cats(Categories);
        C c1(1, 0, "c1");
        Cat cat1("cat1");
        Cat cat2("cat2");
        T t1(3, 1, "t1", QString(), "t1", InProgress, NoTag, QString(), "cat1");
        T t2(4, 1, "t2", QString(), "t2", InProgress, NoTag, QString(), "cat1");
        T t3(5, 1, "t3", QString(), "t3");
        T t4(6, 1, "t4", QString(), "t4", InProgress, NoTag, QString(), "cat1, cat2");
        
        // Create the source structure once and for all
        ModelStructure sourceStructure;
        sourceStructure << c1
        << _+t1
        << _+t2;
        
        ModelPath itemToChange = c1 % t1;
        QVariant value = Zanshin::ProjectTodo;
        int role = Zanshin::ItemTypeRole;
        
        ModelStructure outputStructure;
        outputStructure << nocat
        << cats
        << _+cat1
        << __+t2;
        
        QTest::newRow( "promote item" ) << sourceStructure << itemToChange
        << value << role
        << outputStructure;
        
        
        sourceStructure.clear();
        sourceStructure << c1
        << _+t1
        << _+t3;
        
        itemToChange = c1 % t3;
        QStringList categoriesAdded;
        categoriesAdded << "cat1";
        
        value = categoriesAdded;
        role = Zanshin::CategoriesRole;
        
        outputStructure.clear();
        outputStructure << nocat
        << cats
        << _+cat1
        << __+t1
        << __+t3;
        
        QTest::newRow( "Todo with no category which gets a new category" ) << sourceStructure << itemToChange
        << value << role
        << outputStructure;
        
        sourceStructure.clear();
        sourceStructure << c1
        << _+t1
        << _+t4;
        
        itemToChange = c1 % t1;
        categoriesAdded.clear();
        
        value = categoriesAdded;
        role = Zanshin::CategoriesRole;
        
        outputStructure.clear();
        outputStructure << nocat
        << _+t1
        << cats
        << _+cat1
        << __+t4
        << _+cat2
        << __+t4;
        
        QTest::newRow( "Todo with categories which looses all its categories" ) << sourceStructure << itemToChange
        << value << role
        << outputStructure;
        
        sourceStructure.clear();
        sourceStructure << c1
        << _+t1
        << _+t4;
        
        itemToChange = c1 % t1;
        categoriesAdded.clear();
        categoriesAdded << "cat1";
        categoriesAdded << "cat2";
        
        value = categoriesAdded;
        role = Zanshin::CategoriesRole;
        
        outputStructure.clear();
        outputStructure << nocat
        << cats
        << _+cat1
        << __+t1
        << __+t4
        << _+cat2
        << __+t4
        << __+t1;
        
        QTest::newRow( "Todo with categories which gets a new category" ) << sourceStructure << itemToChange
        << value << role
        << outputStructure;
        
        sourceStructure.clear();
        sourceStructure << c1
        << _+t1
        << _+t4;
        
        itemToChange = c1 % t4;
        categoriesAdded.clear();
        categoriesAdded << "cat1";
        
        value = categoriesAdded;
        role = Zanshin::CategoriesRole;
        
        outputStructure.clear();
        outputStructure << nocat
        << cats
        << _+cat1
        << __+t1
        << __+t4
        << _+cat2;
        
        QTest::newRow( "Todo with categories which looses one category" ) << sourceStructure << itemToChange
        << value << role
        << outputStructure;
    }
    
    void shouldReactToSourceDataChanges()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);
        
        //Source model
        QStandardItemModel source;
        //Kick up category manager
        CategoryManager::instance().setModel(&source);
        ModelUtils::create(&source, sourceStructure);
        
        //create categoriesModel
        TodoCategoriesModel categoriesModel;
        ModelTest t1(&categoriesModel);
        
        categoriesModel.setSourceModel(&source);
        
        //WHEN
        QFETCH(ModelPath, itemToChange);
        QModelIndex index = ModelUtils::locateItem(&source, itemToChange);
        
        QFETCH(QVariant, value);
        QFETCH(int, role);
        
        source.setData(index, value, role);
        
        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);
        
        QCOMPARE(categoriesModel, output);
    }
    
    
    void shouldNotDuplicateTodo_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelPath>( "sourceParentPath" );
        QTest::addColumn<ModelStructure>( "insertedStructure" );
        QTest::addColumn<ModelStructure>( "outputStructure" );
        
        // Base items
        V nocat(NoCategory);
        V cats(Categories);
        C c1(1, 0, "c1");
        Cat cat1("cat1");
        Cat cat2("cat2");
        T t1(3, 1, "t1", QString(), "t1", InProgress, ProjectTag, QString(), "cat1");
        T t2(4, 1, "t2", "t1", "t2");
        T t3(5, 1, "t3", "t1", "t3", InProgress, ProjectTag, QString(), "cat2");
        T t4(6, 1, "t4", "t3", "t4");
        
        // Create the source structure once and for all
        ModelStructure sourceStructure;
        sourceStructure << c1
        << _+t1;
        
        ModelPath sourceParentPath = c1;
        
        ModelStructure insertedStructure;
        insertedStructure << t2;
        
        ModelStructure outputStructure;
        outputStructure << nocat
        << cats
        << _+cat1
        << __+t2;
        
        QTest::newRow( "add one todo" ) << sourceStructure
        << sourceParentPath
        << insertedStructure
        << outputStructure;
        
        sourceStructure.clear();
        sourceStructure << c1
        << _+t1;
        
        sourceParentPath = c1;
        
        insertedStructure.clear();
        insertedStructure << t2
        << t3
        << t4;
        
        outputStructure.clear();
        outputStructure << nocat
        << cats
        << _+cat1
        << __+t2
        << __+t4
        << _+cat2
        << __+t4;
        
        QTest::newRow( "add complexe structure" ) << sourceStructure
        << sourceParentPath
        << insertedStructure
        << outputStructure;
    }
    void shouldNotDuplicateTodo()
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
        
        //Kick up category manager
        CategoryManager::instance().setModel(&metadataModel);
        
        metadataModel.setSourceModel(&source);
        
        //create categoriesModel
        TodoCategoriesModel categoriesModel;
        ModelTest t2(&categoriesModel);
        
        categoriesModel.setSourceModel(&metadataModel);
        
        //WHEN
        QFETCH(ModelPath, sourceParentPath);
        QFETCH(ModelStructure, insertedStructure);
        
        ModelUtils::create(&source, insertedStructure, sourceParentPath, &behavior);
        
        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);
        
        QCOMPARE(categoriesModel, output);
    }
    
    void shouldReactToModelReset_data()
    {
        QTest::addColumn<ModelStructure>("sourceStructure");
        
        // Base items
        V inbox(Inbox);
        C c1(1, 0, "c1");
        T t1(1, 1, "t1", QString(), "t1", InProgress, ProjectTag, QString(), "cat1");
        T t2(2, 1, "t2", "t1", "t2", InProgress, NoTag, QString(), "cat1, cat2");
        
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
        
        //Kick up category manager
        CategoryManager::instance().setModel(&source);
        
        ModelUtils::create(&source, sourceStructure);
        
        //create categoriesModel
        TodoCategoriesModel categoriesModel;
        ModelTest t2(&categoriesModel);
        
        categoriesModel.setSourceModel(&source);
        
        //WHEN
        source.clearData();
        
        //THEN
        MockModel output;
        TodoCategoriesModel* categoriesModelOutput = new TodoCategoriesModel();
        
        categoriesModelOutput->setSourceModel(&output);
        
        QAbstractItemModel* abstractModel = categoriesModelOutput;
        QCOMPARE(categoriesModel, *abstractModel);
        delete categoriesModelOutput;
    }
    */
};

QTEST_KDEMAIN(TopicsModelSpec, GUI)

#include "topicsmodelspec.moc"
