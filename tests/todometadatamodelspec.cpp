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

class TodoMetadataModelTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase()
    {
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
};

QTEST_KDEMAIN(TodoMetadataModelTest, GUI)

#include "todometadatamodeltest.moc"
