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

#include <QtGui/QStandardItemModel>

#include "todometadatamodel.h"
#include "testlib/testlib.h"
#include "testlib/modelbuilderbehavior.h"

using namespace Zanshin::Test;

Q_DECLARE_METATYPE(QModelIndex)

class TodoMetadataModelTest : public QObject
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
        TodoMetadataModel todoMetadataModel(&baseModel);
        ModelTest mt(&todoMetadataModel);

        //WHEN
        todoMetadataModel.setSourceModel(&baseModel);

        //THEN
        QVERIFY(todoMetadataModel.sourceModel() == &baseModel);
    }

    void shouldRetrieveItemState_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<QVariant>( "state" );

        C c1(1, 0, "c1");
        T t1(1, 1, "t1", QString(), "t1", InProgress);
        T t2(2, 1, "t2", QString(), "t2", Done);
        T t3(3, 1, "t3", QString(), "t3", Done, ProjectTag);

        ModelStructure sourceStructure;
        sourceStructure << t1;

        QVariant state = Qt::Unchecked;

        QTest::newRow( "check state role in progress" ) << sourceStructure << state;

        sourceStructure.clear();
        sourceStructure << t2;

        state = Qt::Checked;
        QTest::newRow( "check state role done" ) << sourceStructure << state;

        sourceStructure.clear();
        sourceStructure << t3;

        state = QVariant();
        QTest::newRow( "check state role on project" ) << sourceStructure << state;

        sourceStructure.clear();
        sourceStructure << c1;

        state = QVariant();
        QTest::newRow( "check state role on collection" ) << sourceStructure << state;
    }

    void shouldRetrieveTheItemState()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        StandardModelBuilderBehavior behavior;
        behavior.setMetadataCreationEnabled(false);
        ModelUtils::create(&source, sourceStructure, ModelPath(), &behavior);

        //create metadataModel
        TodoMetadataModel todoMetadataModel;
        ModelTest t1(&todoMetadataModel);

        //WHEN
        todoMetadataModel.setSourceModel(&source);

        //THEN
        QFETCH(QVariant, state);
        QModelIndex index = todoMetadataModel.index(0,0);

        QCOMPARE(index.data(Qt::CheckStateRole), state);
    }

    void shouldRetrieveItemUid_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<QString>( "uid" );

        C c1(1, 0, "c1");
        T t1(1, 1, "t1", QString(), "t1");

        ModelStructure sourceStructure;
        sourceStructure << c1;

        QString uid;
        QTest::newRow( "check uid of a collection" ) << sourceStructure << uid;

        sourceStructure.clear();
        sourceStructure << t1;

        uid = "t1";
        QTest::newRow( "check uid for a todo" ) << sourceStructure << uid;
    }

    void shouldRetrieveItemUid()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        StandardModelBuilderBehavior behavior;
        behavior.setMetadataCreationEnabled(false);
        ModelUtils::create(&source, sourceStructure, ModelPath(), &behavior);

        //create metadataModel
        TodoMetadataModel todoMetadataModel;
        ModelTest t1(&todoMetadataModel);

        //WHEN
        todoMetadataModel.setSourceModel(&source);

        //THEN
        QFETCH(QString, uid);
        QModelIndex index = todoMetadataModel.index(0,0);

        QCOMPARE(index.data(Zanshin::UidRole).toString(), uid);
    }

    void shouldRetrieveItemParentUid_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<QString>( "parentUid" );

        C c1(1, 0, "c1");
        T t1(1, 1, "t1", QString(), "t1");
        T t2(2, 1, "t2", "t1", "t2");

        ModelStructure sourceStructure;
        sourceStructure << c1;

        QString parentUid;
        QTest::newRow( "check without parent" ) << sourceStructure << parentUid;

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t1;

        QTest::newRow( "check parentUid with collection as parent" ) << sourceStructure << parentUid;

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t1
                        << _+t2;

        parentUid = "t1";

        QTest::newRow( "check parentUid with todo as parent" ) << sourceStructure << parentUid;
    }

    void shouldRetrieveItemParentUid()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        StandardModelBuilderBehavior behavior;
        behavior.setMetadataCreationEnabled(false);
        ModelUtils::create(&source, sourceStructure, ModelPath(), &behavior);

        //create metadataModel
        TodoMetadataModel todoMetadataModel;
        ModelTest t1(&todoMetadataModel);

        //WHEN
        todoMetadataModel.setSourceModel(&source);

        //THEN
        QFETCH(QString, parentUid);
        QModelIndex index = todoMetadataModel.index(0,0);
        while (todoMetadataModel.rowCount(index) > 0) {
            index = todoMetadataModel.index(todoMetadataModel.rowCount(index)-1, 0, index);
        }

        QCOMPARE(index.data(Zanshin::ParentUidRole).toString(), parentUid);
    }

    void shouldRetrieveItemAncestorsUid_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<QStringList>( "ancestors" );

        C c1(1, 0, "c1");
        T t1(1, 1, "t1", QString(), "t1");
        T t2(2, 1, "t2", "t1", "t2");
        T t3(3, 2, "t3", "t2", "t3");
        T t4(4, 3, "t4", "t3", "t4");
        T t5(5, 4, "t5", "t4", "t5");
        T t6(6, 5, "t6", "t5", "t6");
        T t7(7, 6, "t7", "t6", "t7");
        T t8(8, 7, "t8", "t7", "t8");
        T t9(9, 8, "t9", "t8", "t9");

        ModelStructure sourceStructure;
        sourceStructure << c1;

        QStringList ancestors;
        QTest::newRow( "check ancestor on collection" ) << sourceStructure << ancestors;

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t1;

        QTest::newRow( "check ancestor with collection as parent" ) << sourceStructure << ancestors;

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t1
                        << _+t2;

        ancestors << "t1";

        QTest::newRow( "check ancestor with one level" ) << sourceStructure << ancestors;

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t1
                        << _+t2
                        << _+t3;

        ancestors.clear();
        ancestors << "t2"
                  << "t1";

        QTest::newRow( "check ancestor with two level" ) << sourceStructure << ancestors;

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t1
                        << _+t2
                        << _+t3
                        << _+t4
                        << _+t5
                        << _+t6
                        << _+t7
                        << _+t8
                        << _+t9;

        ancestors.clear();
        ancestors << "t8"
                  << "t7"
                  << "t6"
                  << "t5"
                  << "t4"
                  << "t3"
                  << "t2"
                  << "t1";

        QTest::newRow( "check ancestor with height level" ) << sourceStructure << ancestors;
    }

    void shouldRetrieveItemAncestors()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        StandardModelBuilderBehavior behavior;
        behavior.setMetadataCreationEnabled(false);
        ModelUtils::create(&source, sourceStructure, ModelPath(), &behavior);

        //create metadataModel
        TodoMetadataModel todoMetadataModel;
        ModelTest t1(&todoMetadataModel);

        //WHEN
        todoMetadataModel.setSourceModel(&source);

        //THEN
        QFETCH(QStringList, ancestors);
        QModelIndex index = todoMetadataModel.index(0,0);
        while (todoMetadataModel.rowCount(index) > 0) {
            index = todoMetadataModel.index(todoMetadataModel.rowCount(index)-1, 0, index);
        }

        QCOMPARE(index.data(Zanshin::AncestorsUidRole).toStringList(), ancestors);
    }

    void shouldRetrieveItemType_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<int>( "type" );

        C c1(1, 0, "c1");
        T t1(1, 1, "t1", QString(), "t1", InProgress, ProjectTag);
        T t2(2, 1, "t2", "t1", "t2");
        T t3(3, 2, "t3", "t3", "t3");
        V inbox(Inbox);
        V categoryRoot(Categories);
        V nocategory(NoCategory);
        Cat category("cat");

        ModelStructure sourceStructure;
        sourceStructure << c1;

        int type = Zanshin::Collection;

        QTest::newRow( "check collection type" ) << sourceStructure << type;

        sourceStructure.clear();
        sourceStructure << t1;

        type = Zanshin::ProjectTodo;

        QTest::newRow( "check project type" ) << sourceStructure << type;

        sourceStructure.clear();
        sourceStructure << t2;

        type = Zanshin::StandardTodo;

        QTest::newRow( "check todo type" ) << sourceStructure << type;

        sourceStructure.clear();
        sourceStructure << t1
                        << t2;

        type = Zanshin::ProjectTodo;

        QTest::newRow( "check todo type when it has a child" ) << sourceStructure << type;

        sourceStructure.clear();
        sourceStructure << inbox;

        type = 0;

        QTest::newRow( "check inbox type" ) << sourceStructure << type;

        sourceStructure.clear();
        sourceStructure << categoryRoot;

        type = 0;

        QTest::newRow( "check category root type" ) << sourceStructure << type;

        sourceStructure.clear();
        sourceStructure << nocategory;

        type = 0;

        QTest::newRow( "check no category type" ) << sourceStructure << type;

        sourceStructure.clear();
        sourceStructure << category;

        type = 0;

        QTest::newRow( "check category type" ) << sourceStructure << type;
    }

    void shouldRetrieveItemType()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        StandardModelBuilderBehavior behavior;
        behavior.setMetadataCreationEnabled(false);
        ModelUtils::create(&source, sourceStructure, ModelPath(), &behavior);

        //create metadataModel
        TodoMetadataModel todoMetadataModel;
        ModelTest t1(&todoMetadataModel);

        //WHEN
        todoMetadataModel.setSourceModel(&source);

        //THEN
        QFETCH(int, type);
        QModelIndex index = todoMetadataModel.index(0,0);

        QCOMPARE(index.data(Zanshin::ItemTypeRole).toInt(), type);
    }

    void shouldRetrieveItemCategories_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<QStringList>( "categories" );

        C c1(1, 0, "c1");
        T t1(1, 1, "t1", QString(), "t1");
        T t2(2, 1, "t2", "t1", "t2", Done, NoTag, QString(), "cat1");
        T t3(3, 2, "t3", "t2", "t3", Done, NoTag, QString(), "cat2");
        T t4(4, 3, "t4", "t3", "t4", Done, NoTag, QString(), "cat2");
        T t5(5, 4, "t5", "t4", "t5", Done, NoTag, QString(), "cat3");
        T t6(6, 5, "t6", "t5", "t6", Done, NoTag, QString(), "cat3");
        T t7(7, 6, "t7", "t6", "t7", Done, NoTag, QString(), "cat4");
        T t8(8, 7, "t8", "t7", "t8", Done, NoTag, QString(), "cat5");
        T t9(9, 8, "t9", "t8", "t9");

        ModelStructure sourceStructure;
        sourceStructure << c1;

        QStringList categories;
        QTest::newRow( "get category list on collection" ) << sourceStructure << categories;

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t1;

        QTest::newRow( "check todo without category set" ) << sourceStructure << categories;

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t1
                        << _+t2;

        categories << "cat1";

        QTest::newRow( "check todo with category set" ) << sourceStructure << categories;

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t1
                        << _+t2
                        << _+t3;

        categories.clear();
        categories << "cat1"
                   << "cat2";

        QTest::newRow( "check todo with the different category set" ) << sourceStructure << categories;

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t1
                        << _+t2
                        << _+t3
                        << _+t4;

        categories.clear();
        categories << "cat2"
                   << "cat1";

        QTest::newRow( "check todo with the same category set" ) << sourceStructure << categories;

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t1
                        << _+t2
                        << _+t3
                        << _+t4
                        << _+t5
                        << _+t6
                        << _+t7
                        << _+t8
                        << _+t9;

        categories.clear();
        categories << "cat5"
                   << "cat4"
                   << "cat3"
                   << "cat2"
                   << "cat1";

        QTest::newRow( "check todo with some category duplicate" ) << sourceStructure << categories;
    }

    void shouldRetrieveItemCategories()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        StandardModelBuilderBehavior behavior;
        behavior.setMetadataCreationEnabled(false);
        ModelUtils::create(&source, sourceStructure, ModelPath(), &behavior);

        //create metadataModel
        TodoMetadataModel todoMetadataModel;
        ModelTest t1(&todoMetadataModel);

        //WHEN
        todoMetadataModel.setSourceModel(&source);

        //THEN
        QFETCH(QStringList, categories);
        QModelIndex index = todoMetadataModel.index(0,0);
        while (todoMetadataModel.rowCount(index) > 0) {
            index = todoMetadataModel.index(todoMetadataModel.rowCount(index)-1, 0, index);
        }

        QCOMPARE(index.data(Zanshin::CategoriesRole).toStringList(), categories);
    }

    void shouldRetrieveItemDataType_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<int>( "column" );
        QTest::addColumn<int>( "type" );

        T t1(1, 0, "t1", QString(), "t1");

        ModelStructure sourceStructure;
        sourceStructure << t1;

        int column = 1;
        int type = Zanshin::ProjectType;

        QTest::newRow( "check data type for column 1" ) << sourceStructure << column << type;

        column = 2;
        type = Zanshin::CategoryType;

        QTest::newRow( "check data type for column 2" ) << sourceStructure << column << type;

        column = 0;
        type = Zanshin::StandardType;

        QTest::newRow( "check data type for column 0" ) << sourceStructure << column << type;

        column = 3;
        type = Zanshin::StandardType;

        QTest::newRow( "check data type for column 3" ) << sourceStructure << column << type;
    }

    void shouldRetrieveItemDataType()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        StandardModelBuilderBehavior behavior;
        behavior.setMetadataCreationEnabled(false);
        ModelUtils::create(&source, sourceStructure, ModelPath(), &behavior);

        //create metadataModel
        TodoMetadataModel todoMetadataModel;
        ModelTest t1(&todoMetadataModel);

        //WHEN
        todoMetadataModel.setSourceModel(&source);

        //THEN
        QFETCH(int, column);
        QFETCH(int, type);
        QModelIndex index = todoMetadataModel.index(0, column);
        QCOMPARE(index.data(Zanshin::DataTypeRole).toInt(), type);
    }

    void shouldRetrieveItemChildUids_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<QStringList>( "childUids" );

        C c1(1, 0, "c1");
        T t1(1, 1, "t1", QString(), "t1");
        T t2(2, 1, "t2", "t1", "t2");
        T t3(3, 1, "t3", "t1", "t3");
        T t4(4, 1, "t4", "t1", "t4");
        T t5(5, 4, "t5", "t4", "t5");

        ModelStructure sourceStructure;
        sourceStructure << c1;

        QStringList childUids;
        QTest::newRow( "get child list on empty collection" ) << sourceStructure << childUids;

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t1;

        QTest::newRow( "get child list on collection with one child" ) << sourceStructure << childUids;

        sourceStructure.clear();
        sourceStructure << t1
                        << t2;

        childUids << "t2";

        QTest::newRow( "get child list on todo with one child" ) << sourceStructure << childUids;

        sourceStructure.clear();
        sourceStructure << t1
                        << t2
                        << t3
                        << t4;

        childUids.clear();
        childUids << "t2"
                  << "t3"
                  << "t4";

        QTest::newRow( "get child list on todo with several children" ) << sourceStructure << childUids;

        sourceStructure.clear();
        sourceStructure << t1
                        << t2
                        << t3
                        << t4
                        << t5;

        childUids.clear();
        childUids << "t2"
                  << "t3"
                  << "t4";

        QTest::newRow( "check if the list is recursive" ) << sourceStructure << childUids;

    }

    void shouldRetrieveItemChildUids()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        StandardModelBuilderBehavior behavior;
        behavior.setMetadataCreationEnabled(false);
        ModelUtils::create(&source, sourceStructure, ModelPath(), &behavior);

        //create metadataModel
        TodoMetadataModel todoMetadataModel;
        ModelTest t1(&todoMetadataModel);

        //WHEN
        todoMetadataModel.setSourceModel(&source);

        //THEN
        QFETCH(QStringList, childUids);
        QModelIndex index = todoMetadataModel.index(0,0);

        QCOMPARE(index.data(Zanshin::ChildUidsRole).toStringList(), childUids);
    }

    void shouldRetrieveItemChildIndexes_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<QStringList>( "childUids" );

        C c1(1, 0, "c1");
        T t1(1, 1, "t1", QString(), "t1");
        T t2(2, 1, "t2", "t1", "t2");
        T t3(3, 1, "t3", "t1", "t3");
        T t4(4, 1, "t4", "t1", "t4");
        T t5(5, 4, "t5", "t4", "t5");

        ModelStructure sourceStructure;
        sourceStructure << c1;

        QStringList childUids;
        QTest::newRow( "get child list on empty collection" ) << sourceStructure << childUids;

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t1;

        QTest::newRow( "get child list on collection with one child" ) << sourceStructure << childUids;

        sourceStructure.clear();
        sourceStructure << t1
                        << t2;

        childUids << "t2";

        QTest::newRow( "get child list on todo with one child" ) << sourceStructure << childUids;

        sourceStructure.clear();
        sourceStructure << t1
                        << t2
                        << t3
                        << t4;

        childUids.clear();
        childUids << "t2"
                  << "t3"
                  << "t4";

        QTest::newRow( "get child list on todo with several children" ) << sourceStructure << childUids;

        sourceStructure.clear();
        sourceStructure << t1
                        << t2
                        << t3
                        << t4
                        << t5;

        childUids.clear();
        childUids << "t2"
                  << "t3"
                  << "t4";

        QTest::newRow( "check if the list is recursive" ) << sourceStructure << childUids;

    }

    void shouldRetrieveItemChildIndexes()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        StandardModelBuilderBehavior behavior;
        behavior.setMetadataCreationEnabled(false);
        ModelUtils::create(&source, sourceStructure, ModelPath(), &behavior);

        //create metadataModel
        TodoMetadataModel todoMetadataModel;
        ModelTest t1(&todoMetadataModel);

        //WHEN
        todoMetadataModel.setSourceModel(&source);

        //THEN
        QFETCH(QStringList, childUids);
        QModelIndex index = todoMetadataModel.index(0,0);
        QModelIndexList children = index.data(Zanshin::ChildIndexesRole).value<QModelIndexList>();

        QCOMPARE(childUids.size(), children.size());

        for (int i=0; i<childUids.size(); ++i) {
            QCOMPARE(children[i].data(Zanshin::UidRole).toString(), childUids[i]);
        }
    }

    void shouldRetrieveItemFlags_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<int>( "column" );
        QTest::addColumn<int>( "flags" );

        C c1(1, 0, "c1");
        T t1(1, 1, "t1", QString(), "t1", InProgress, ProjectTag);
        T t2(2, 1, "t2", "t1", "t2");
        V inbox(Inbox);
        V categoryRoot(Categories);
        V nocategory(NoCategory);
        Cat category("cat");

        ModelStructure sourceStructure;
        sourceStructure << c1;

        int flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
        int column = 0;
        QTest::newRow( "get flags on collection" ) << sourceStructure << column << flags;

        sourceStructure.clear();
        sourceStructure << t2;

        flags = Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsDropEnabled;

        QTest::newRow( "get flags on todo" ) << sourceStructure << column << flags;

        flags = Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
        column = 4;

        QTest::newRow( "get flags on todo on column 4" ) << sourceStructure << column << flags;

        sourceStructure.clear();
        sourceStructure << t1;

        column = 0;
        flags = Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

        QTest::newRow( "get flags on project" ) << sourceStructure << column << flags;

        sourceStructure.clear();
        sourceStructure << inbox;

        flags = Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

        QTest::newRow( "get flags on inbox" ) << sourceStructure << column << flags;

        sourceStructure.clear();
        sourceStructure << category;

        flags = Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

        QTest::newRow( "get flags on category" ) << sourceStructure << column << flags;

        sourceStructure.clear();
        sourceStructure << categoryRoot;

        flags = Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

        QTest::newRow( "get flags on category root" ) << sourceStructure << column << flags;

        sourceStructure.clear();
        sourceStructure << nocategory;

        flags = Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

        QTest::newRow( "get flags on NoCategory" ) << sourceStructure << column << flags;
    }

    void shouldRetrieveItemFlags()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        QStandardItemModel source;
        StandardModelBuilderBehavior behavior;
        behavior.setMetadataCreationEnabled(false);
        ModelUtils::create(&source, sourceStructure, ModelPath(), &behavior);

        //create metadataModel
        TodoMetadataModel todoMetadataModel;
        ModelTest t1(&todoMetadataModel);

        //WHEN
        todoMetadataModel.setSourceModel(&source);

        //THEN
        QFETCH(int, column);
        QFETCH(int, flags);

        QModelIndex index = todoMetadataModel.index(0,column);

        QCOMPARE(todoMetadataModel.flags(index), flags);
    }

    void shouldReactToSourceRowInserts_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelPath>( "sourceParentPath" );
        QTest::addColumn<ModelStructure>( "insertedStructure" );
        QTest::addColumn<ModelStructure>( "outputStructure" );

        // Base items
        C c1(1, 0, "c1");
        T t1(1, 1, "t1", QString(), "t1", InProgress, ProjectTag);
        T t2(2, 1, "t2", "t1", "t2");
        T t3(3, 1, "t3", "t2", "t3");
        T t4(4, 1, "t4", QString(), "t4");

        // Create the source structure once and for all
        ModelStructure sourceStructure;
        sourceStructure << c1;

        ModelPath sourceParentPath = c1;
        ModelStructure insertedStructure;
        insertedStructure << t1;

        ModelStructure outputStructure;
        outputStructure << c1
                        << _+t1;

        QTest::newRow( "simple insertion" ) << sourceStructure << sourceParentPath
                                               << insertedStructure
                                               << outputStructure;

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t1;

        sourceParentPath = c1 % t1;

        insertedStructure.clear();
        insertedStructure << t2;

        outputStructure.clear();
        outputStructure << c1
                        << _+t1
                        << __+t2;

        QTest::newRow( "add todo to project" ) << sourceStructure << sourceParentPath
                                               << insertedStructure
                                               << outputStructure;

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t2;

        sourceParentPath = c1 % t2;

        insertedStructure.clear();
        insertedStructure << t3;

        outputStructure.clear();
        outputStructure << c1
                        << _+t2
                        << __+t3;

        QTest::newRow( "add todo to project without project tag" ) << sourceStructure << sourceParentPath
                                                                   << insertedStructure
                                                                   << outputStructure;

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t3;

        sourceParentPath = c1 % t3;

        insertedStructure.clear();
        insertedStructure << t4;

        outputStructure.clear();
        outputStructure << c1
                        << _+t3
                        << __+t4;

        QTest::newRow( "add todo to todo" ) << sourceStructure << sourceParentPath
                                            << insertedStructure
                                            << outputStructure;

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t1;

        sourceParentPath = c1;

        insertedStructure.clear();
        insertedStructure << t2;

        outputStructure.clear();
        outputStructure << c1
                        << _+t1
                        << _+t2;

        QTest::newRow( "add todo to collection with a todo" ) << sourceStructure << sourceParentPath
                                                              << insertedStructure
                                                              << outputStructure;
    }

    void shouldReactToSourceRowInserts()
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

        // What row number will we expect?
        QFETCH(ModelPath, sourceParentPath);
        QModelIndex parentIndex = ModelUtils::locateItem(&metadataModel, sourceParentPath);
        const int expectedRow = metadataModel.rowCount(parentIndex);

        //WHEN
        QFETCH(ModelStructure, insertedStructure);

        // Collect data to ensure we signalled the outside properly
        QSignalSpy aboutToInsertSpy(&metadataModel, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)));
        QSignalSpy insertSpy(&metadataModel, SIGNAL(rowsInserted(QModelIndex, int, int)));
        QSignalSpy changeSpy(&metadataModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)));

        ModelUtils::create(&source, insertedStructure, sourceParentPath, &behavior);

        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        ModelUtils::create(&output, outputStructure);

        QCOMPARE(metadataModel, output);

        QCOMPARE(aboutToInsertSpy.first().at(0).value<QModelIndex>(), parentIndex);
        QCOMPARE(aboutToInsertSpy.first().at(1).toInt(), expectedRow);
        QCOMPARE(aboutToInsertSpy.first().at(2).toInt(), expectedRow);

        QCOMPARE(insertSpy.first().at(0).value<QModelIndex>(), parentIndex);
        QCOMPARE(insertSpy.first().at(1).toInt(), expectedRow);
        QCOMPARE(insertSpy.first().at(2).toInt(), expectedRow);

        if (parentIndex.data(Zanshin::ItemTypeRole).toInt()==Zanshin::ProjectTodo) {
            QCOMPARE(changeSpy.first().at(0).value<QModelIndex>(), parentIndex);
            QCOMPARE(changeSpy.first().at(1).value<QModelIndex>(), parentIndex);
        }
    }

};

QTEST_KDEMAIN(TodoMetadataModelTest, GUI)

#include "todometadatamodeltest.moc"
