/* This file is part of Zanshin Todo.

   Copyright 2011 Kevin Ottens <ervin@kde.org>

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

#include "categorymanager.h"
#include "todometadatamodel.h"
#include "testlib/testlib.h"
#include "testlib/mockmodel.h"
#include "testlib/modelbuilderbehavior.h"
#include "testlib/helperutils.h"
#include <reparentingmodel/reparentingmodel.h>
#include <reparentingmodel/categoriesstrategy.h>

#include <QtGui/QTreeView>
#include <QtCore/QEventLoop>

using namespace Zanshin::Test;

Q_DECLARE_METATYPE(QModelIndex)

class TodoCategoriesModelSpec : public QObject
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

    void shouldRememberItsSourceModel()
    {
        //GIVEN
        QStandardItemModel baseModel;
        ReparentingModel proxyModel(new CategoriesStrategy);
        ModelTest mt(&proxyModel);

        //WHEN
        proxyModel.setSourceModel(&baseModel);

        //THEN
        QVERIFY(proxyModel.sourceModel() == &baseModel);
    }

    void shouldFillModel_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
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
        T t4(6, 2, "t4", QString(), "t4", InProgress, NoTag, QString(), "cat2");
        T t5(7, 2, "t5", QString(), "t5", InProgress, NoTag);
        T t6(8, 1, "t6", QString(), "t6", InProgress, NoTag, QString(), "cat1");

        {
            // Create the source structure once and for all
            ModelStructure sourceStructure;
            sourceStructure << t1
                            << t3
                            << t4
                            << t5
                            << t6;

            ModelStructure outputStructure;
            outputStructure << nocat
                            << _+t5
                            << cats
                            << _+cat2
                            << __+t4
                            << _+cat1
                            << __+t6;

            QTest::newRow( "basic" ) << sourceStructure << outputStructure;
        }
        {
            // Create the source structure once and for all
            ModelStructure sourceStructure;
            sourceStructure << c1
                            << _+t1
                            << _+t6
                            << c2
                            << _+t3
                            << _+t4
                            << _+t5;

            ModelStructure outputStructure;
            outputStructure << nocat
                            << _+t5
                            << cats
                            << _+cat1
                            << __+t6
                            << _+cat2
                            << __+t4;

            QTest::newRow( "filter collections" ) << sourceStructure << outputStructure;
        }
        //TODO
//         {
//             // Create the source structure once and for all
//             ModelStructure sourceStructure;
//             sourceStructure << t2
//                             << t5;
// 
//             ModelStructure outputStructure;
//             outputStructure << nocat
//                             << _+t5
//                             << cats
//                             << _+cat1
//                             << __+t2
//                             << _+cat2
//                             << __+t2;
// 
//             QTest::newRow( "multiparents" ) << sourceStructure << outputStructure;
//         }

    }

    void shouldFillModel()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        ModelUtils::create(&source, sourceStructure);

        //create categoriesModel
        ReparentingModel categoriesModel(new CategoriesStrategy);
        ModelTest t1(&categoriesModel);

        categoriesModel.setSourceModel(&source);

        Helper::printModel(&categoriesModel);

        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);
        Helper::printModel(&output);

        QCOMPARE(categoriesModel, output);
    }
    

    void shouldReactToSourceRowRemovals_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelPath::List>( "itemsToRemove" );
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
        T t5(7, 2, "t5", QString(), "t5", InProgress, NoTag);
        T t6(8, 2, "t6", QString(), "t6", InProgress, NoTag, QString(), "cat2");

        {
            // Create the source structure once and for all
            ModelStructure sourceStructure;
            sourceStructure << c1
                            << _+t1
                            << c2
                            << _+t3
                            << _+t4
                            << _+t5
                            << _+t6;


            ModelPath::List itemsToRemove;
            itemsToRemove << c2 % t6;

            ModelStructure outputStructure;
            outputStructure << nocat
                            << _+t5
                            << cats
                            << _+cat1
                            << __+t4
                            << _+cat2;

            QTest::newRow( "delete todo" ) << sourceStructure << itemsToRemove << outputStructure;
        }
        {
            // Create the source structure once and for all
            ModelStructure sourceStructure;
            sourceStructure << c1
                            << _+t1
                            << _+t2
                            << c2
                            << _+t3
                            << _+t4
                            << _+t5;


            ModelPath::List itemsToRemove;
            itemsToRemove << c1 % t2;

            ModelStructure outputStructure;
            outputStructure << nocat
                            << _+t5
                            << cats
                            << _+cat1
                            << __+t4
                            << _+cat2;

            QTest::newRow( "delete multiparent todo" ) << sourceStructure << itemsToRemove << outputStructure;

            itemsToRemove.clear();
            itemsToRemove << c1;

            QTest::newRow( "delete collection" ) << sourceStructure << itemsToRemove << outputStructure;
        }

    }

    void shouldReactToSourceRowRemovals()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        ModelUtils::create(&source, sourceStructure);

        //create categoriesModel
        ReparentingModel categoriesModel(new CategoriesStrategy);
        ModelTest t1(&categoriesModel);

        categoriesModel.setSourceModel(&source);

        Helper::printModel(&source);

        //WHEN
        QFETCH(ModelPath::List, itemsToRemove);
        ModelUtils::destroy(&source, itemsToRemove);

        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);

        Helper::printModel(&categoriesModel);

        QCOMPARE(categoriesModel, output);
    }

    void shouldReparentBasedOnCategories_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
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
        T t5(7, 2, "t5", QString(), "t5", InProgress, NoTag);
        T t6(8, 2, "t6", QString(), "t6", InProgress, NoTag, QString(), "cat2");

        {
            // Create the source structure once and for all
            ModelStructure sourceStructure;
            sourceStructure << c1
                            << _+t1
                            << c2
                            << _+t3
                            << _+t4
                            << _+t5
                            << _+t6;

            ModelStructure outputStructure;
            outputStructure << nocat
                            << _+t5
                            << cats
                            << _+cat1
                            << __+t4
                            << _+cat2
                            << __+t6;

            QTest::newRow( "nominal case" ) << sourceStructure << outputStructure;
        }
//         {
//             // Create the source structure once and for all
//             ModelStructure sourceStructure;
//             sourceStructure << c1
//                             << _+t1
//                             << _+t2
//                             << c2
//                             << _+t3
//                             << _+t4
//                             << _+t5;
// 

// 
//             ModelStructure outputStructure;
//             outputStructure << nocat
//                             << _+t5
//                             << cats
//                             << _+cat1
//                             << __+t2
//                             << __+t4
//                             << _+cat2
//                             << __+t2;
// 
//             QTest::newRow( "multiparent case" ) << sourceStructure << outputStructure;
//         }
    }

    void shouldReparentBasedOnCategories()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        ModelUtils::create(&source, sourceStructure);

        //WHEN
        //create categoriesModel
        ReparentingModel categoriesModel(new CategoriesStrategy);
        ModelTest t1(&categoriesModel);

        categoriesModel.setSourceModel(&source);

        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);
        Helper::printModel(&categoriesModel);
        Helper::printModel(&output);
        QCOMPARE(categoriesModel, output);
    }

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
        T t5(7, 2, "t5", QString(), "t5", InProgress, NoTag);
        T t6(8, 1, "t6", QString(), "t6", InProgress, NoTag, QString(), "cat2");

        {
            // Create the source structure once and for all
            ModelStructure sourceStructure;
            sourceStructure << c1
                            << _+t1
                            << _+t6
                            << c2
                            << _+t3
                            << _+t5;


            ModelPath sourceParentPath = c2;
            ModelPath::List sourceSiblingPaths;
            sourceSiblingPaths << c1 % t6;

            ModelStructure insertedStructure;
            insertedStructure << t4;

            ModelStructure outputStructure;
            outputStructure << nocat
                            << _+t5
                            << cats
                            << _+cat2
                            << __+t6
                            << _+cat1
                            << __+t4;

            QTest::newRow( "add simple todo" ) << sourceStructure << sourceParentPath
                                                                << sourceSiblingPaths << insertedStructure
                                                                << outputStructure;
        }
//         {
//             // Create the source structure once and for all
//             ModelStructure sourceStructure;
//             sourceStructure << c1
//                             << _+t1
//                             << _+t6
//                             << c2
//                             << _+t3
//                             << _+t4
//                             << _+t5;
// 
// 
//             ModelPath sourceParentPath = c1;
//             ModelPath::List sourceSiblingPaths;
//             sourceSiblingPaths << c1 % t6 << c2 % t4;
// 
//             ModelStructure insertedStructure;
//             insertedStructure << t2;
// 
//             ModelStructure outputStructure;
//             outputStructure << nocat
//                             << _+t5
//                             << cats
//                             << _+cat2
//                             << __+t6
//                             << __+t2
//                             << _+cat1
//                             << __+t4
//                             << __+t2;
// 
//             QTest::newRow( "add todo with several categories" ) << sourceStructure << sourceParentPath
//                                                                 << sourceSiblingPaths << insertedStructure
//                                                                 << outputStructure;
//         }
    }

    void shouldReactToSourceRowInserts()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        ModelUtils::create(&source, sourceStructure);

        //create treeModel
        ReparentingModel categoriesModel(new CategoriesStrategy);
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

        //FIXME check is invalid, we get an inserted signal for the created category and one for the todo itself
//         QCOMPARE(aboutToInsertSpy.size(), parentIndexes.count());
//         QCOMPARE(insertSpy.size(), parentIndexes.count());
// 
//         for (int i=0; i<parentIndexes.count(); i++) {
//             const QModelIndex parentIndex = parentIndexes.at(i);
//             const int expectedRow = expectedRows.at(i);
// 
//             QCOMPARE(aboutToInsertSpy.at(i).at(0).value<QModelIndex>(), parentIndex);
//             QCOMPARE(aboutToInsertSpy.at(i).at(1).toInt(), expectedRow);
//             QCOMPARE(aboutToInsertSpy.at(i).at(2).toInt(), expectedRow);
// 
//             QCOMPARE(insertSpy.at(i).at(0).value<QModelIndex>(), parentIndex);
//             QCOMPARE(insertSpy.at(i).at(1).toInt(), expectedRow);
//             QCOMPARE(insertSpy.at(i).at(2).toInt(), expectedRow);
//         }
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

//         //FIXME we're evaluating the item not the categories role

//         sourceStructure.clear();
//         sourceStructure << c1
//                         << _+t1
//                         << _+t3;
// 
//         itemToChange = c1 % t3;
//         QStringList categoriesAdded;
//         categoriesAdded << "cat1";
// 
//         value = categoriesAdded;
//         role = Zanshin::CategoriesRole;
// 
//         outputStructure.clear();
//         outputStructure << nocat
//                         << cats
//                         << _+cat1
//                         << __+t1
//                         << __+t3;
// 
//         QTest::newRow( "Todo with no category which gets a new category" ) << sourceStructure << itemToChange
//                                                                            << value << role
//                                                                            << outputStructure;

//         sourceStructure.clear();
//         sourceStructure << c1
//                         << _+t1
//                         << _+t4;
// 
//         itemToChange = c1 % t1;
//         categoriesAdded.clear();
// 
//         value = categoriesAdded;
//         role = Zanshin::CategoriesRole;
// 
//         outputStructure.clear();
//         outputStructure << nocat
//                         << _+t1
//                         << cats
//                         << _+cat1
//                         << __+t4
//                         << _+cat2
//                         << __+t4;
// 
//         QTest::newRow( "Todo with categories which looses all its categories" ) << sourceStructure << itemToChange
//                                                                                 << value << role
//                                                                                 << outputStructure;
// 
//         sourceStructure.clear();
//         sourceStructure << c1
//                         << _+t1
//                         << _+t4;
// 
//         itemToChange = c1 % t1;
//         categoriesAdded.clear();
//         categoriesAdded << "cat1";
//         categoriesAdded << "cat2";
// 
//         value = categoriesAdded;
//         role = Zanshin::CategoriesRole;
// 
//         outputStructure.clear();
//         outputStructure << nocat
//                         << cats
//                         << _+cat1
//                         << __+t1
//                         << __+t4
//                         << _+cat2
//                         << __+t4
//                         << __+t1;
// 
//         QTest::newRow( "Todo with categories which gets a new category" ) << sourceStructure << itemToChange
//                                                                           << value << role
//                                                                           << outputStructure;
// 
//         sourceStructure.clear();
//         sourceStructure << c1
//                         << _+t1
//                         << _+t4;
// 
//         itemToChange = c1 % t4;
//         categoriesAdded.clear();
//         categoriesAdded << "cat1";
// 
//         value = categoriesAdded;
//         role = Zanshin::CategoriesRole;
// 
//         outputStructure.clear();
//         outputStructure << nocat
//                         << cats
//                         << _+cat1
//                         << __+t1
//                         << __+t4
//                         << _+cat2;
// 
//         QTest::newRow( "Todo with categories which looses one category" ) << sourceStructure << itemToChange
//                                                                           << value << role
//                                                                           << outputStructure;
    }

    void shouldReactToSourceDataChanges()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        ModelUtils::create(&source, sourceStructure);

        //create categoriesModel
        ReparentingModel categoriesModel(new CategoriesStrategy);
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

        Helper::printModel(&categoriesModel);

        QCOMPARE(categoriesModel, output);
    }


//     void shouldNotDuplicateTodo_data()
//     {
//         QTest::addColumn<ModelStructure>( "sourceStructure" );
//         QTest::addColumn<ModelPath>( "sourceParentPath" );
//         QTest::addColumn<ModelStructure>( "insertedStructure" );
//         QTest::addColumn<ModelStructure>( "outputStructure" );
// 
//         //TODO do we really want to apply categories of parent projects?
//         // Base items
//         V nocat(NoCategory);
//         V cats(Categories);
//         C c1(1, 0, "c1");
//         Cat cat1("cat1");
//         Cat cat2("cat2");
//         T t1(3, 1, "t1", QString(), "t1", InProgress, ProjectTag, QString(), "cat1");
//         T t2(4, 1, "t2", "t1", "t2");
//         T t3(5, 1, "t3", "t1", "t3", InProgress, ProjectTag, QString(), "cat2");
//         T t4(6, 1, "t4", "t3", "t4");
// 
//         // Create the source structure once and for all
//         ModelStructure sourceStructure;
//         sourceStructure << c1
//                         << _+t1;
// 
//         ModelPath sourceParentPath = c1;
// 
//         ModelStructure insertedStructure;
//         insertedStructure << t2;
// 
//         ModelStructure outputStructure;
//         outputStructure << nocat
//                         << cats
//                         << _+cat1
//                         << __+t2;
// 
//         QTest::newRow( "add one todo" ) << sourceStructure
//                                         << sourceParentPath
//                                         << insertedStructure
//                                         << outputStructure;
// /*
//         sourceStructure.clear();
//         sourceStructure << c1
//                         << _+t1;
// 
//         sourceParentPath = c1;
// 
//         insertedStructure.clear();
//         insertedStructure << t2
//                           << t3
//                           << t4;
// 
//         outputStructure.clear();
//         outputStructure << nocat
//                         << cats
//                         << _+cat1
//                         << __+t2
//                         << __+t4
//                         << _+cat2
//                         << __+t4;
// 
//         QTest::newRow( "add complexe structure" ) << sourceStructure
//                                                   << sourceParentPath
//                                                   << insertedStructure
//                                                   << outputStructure;*/
//     }
//     void shouldNotDuplicateTodo()
//     {
//         //GIVEN
//         QFETCH(ModelStructure, sourceStructure);
// 
//         //Source model
//         QStandardItemModel source;
// 
//         StandardModelBuilderBehavior behavior;
//         behavior.setMetadataCreationEnabled(false);
//         ModelUtils::create(&source, sourceStructure, ModelPath(), &behavior);
// 
//         //create metadataModel
//         TodoMetadataModel metadataModel;
//         ModelTest t1(&metadataModel);
// 
//         //Kick up category manager
// //         CategoryManager::instance().setModel(&metadataModel);
// 
//         metadataModel.setSourceModel(&source);
// 
//         //create categoriesModel
//         ReparentingModel categoriesModel(new CategoriesStrategy);
//         ModelTest t2(&categoriesModel);
// 
//         categoriesModel.setSourceModel(&metadataModel);
// 
//         //WHEN
//         QFETCH(ModelPath, sourceParentPath);
//         QFETCH(ModelStructure, insertedStructure);
// 
//         ModelUtils::create(&source, insertedStructure, sourceParentPath, &behavior);
// 
//         //THEN
//         QFETCH(ModelStructure, outputStructure);
//         QStandardItemModel output;
//         ModelUtils::create(&output, outputStructure);
// 
//         QCOMPARE(categoriesModel, output);
//     }

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

        ModelUtils::create(&source, sourceStructure);

        //create categoriesModel
        ReparentingModel categoriesModel(new CategoriesStrategy);
        ModelTest t2(&categoriesModel);

        categoriesModel.setSourceModel(&source);

        //WHEN
        source.clearData();

        //THEN
        MockModel output;
        ReparentingModel* categoriesModelOutput = new ReparentingModel(new CategoriesStrategy);

        categoriesModelOutput->setSourceModel(&output);

        QAbstractItemModel* abstractModel = categoriesModelOutput;
        QCOMPARE(categoriesModel, *abstractModel);
        delete categoriesModelOutput;
    }

    void shouldReactToCategoryRemoval_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<QString>( "categoryToRemove" );
        QTest::addColumn<ModelStructure>( "outputStructure" );

        // Base items
        V nocat(NoCategory);
        V cats(Categories);
        Cat cat1("cat1");
        Cat cat2("cat1"+Cat::pathSeparator()+"cat2");
        T t1(3, 1, "t1", QString(), "t1", InProgress, NoTag, QString(), cat1.name);
        T t2(4, 1, "t2", QString(), "t2", InProgress, NoTag, QString(), cat2.name);
//         T t3(5, 1, "t3", QString(), "t3");
        {
            ModelStructure sourceStructure;
            sourceStructure << t1;
            ModelStructure outputStructure;
            outputStructure << nocat
                            << _+t1
                            << cats;

            QTest::newRow( "remove category" ) << sourceStructure <<  cat1.name << outputStructure;
        }
        {
            ModelStructure sourceStructure;
            sourceStructure << t1
                            << t2;
//                             << t3;
            ModelStructure outputStructure;
            outputStructure << nocat
//                             << _+t3
                            << _+t2
                            << cats
                            << _+cat1
                            << __+t1;

            QTest::newRow( "remove sub-category" ) << sourceStructure <<  cat2.name << outputStructure;
        }
    }

    void shouldReactToCategoryRemoval()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        //Kick up category manager
        ModelUtils::create(&source, sourceStructure);

        //create categoriesModel
        ReparentingModel categoriesModel(new CategoriesStrategy);
        ModelTest t1(&categoriesModel);

        categoriesModel.setSourceModel(&source);

        //WHEN
        QFETCH(QString, categoryToRemove);
        CategoryManager::instance().removeCategories(0, categoryToRemove);

        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);

        Helper::printModel(&categoriesModel);

        QCOMPARE(categoriesModel, output);
    }
    
};

QTEST_KDEMAIN(TodoCategoriesModelSpec, GUI)

#include "todocategoriesmodelspec.moc"
