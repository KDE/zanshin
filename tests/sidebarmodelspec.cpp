/* This file is part of Zanshin Todo.

   Copyright 2008-2011 Mario Bensi <mbensi@ipsquad.net>

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

#include "sidebarmodel.h"
#include "todometadatamodel.h"

#include <reparentingmodel/reparentingmodel.h>
#include <reparentingmodel/projectstrategy.h>

#include "testlib/testlib.h"
#include "testlib/modelbuilderbehavior.h"

using namespace Zanshin::Test;

class SideBarModelTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldReactToSourceRowRemovals_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelPath::List>( "itemsToRemove" );
        QTest::addColumn<ModelStructure>( "outputStructure" );

        V inbox(Inbox);
        C c1(1, 0, "c1");
        T t1(1, 1, "t1", QString(), "t1");
        T t2(2, 1, "t2", "t1", "t2");
        T t3(3, 1, "t3", QString(), "t3", InProgress, ProjectTag);
        T t4(4, 3, "t4", "t3", "t4");

        ModelStructure sourceStructure;
        sourceStructure << c1
                        << _+t1
                        << _+t2;

        ModelPath::List itemsToRemove;
        itemsToRemove << c1 % t2;

        ModelStructure outputStructure;
        outputStructure << inbox
                        << c1;

        QTest::newRow( "Remove all children under a project" ) << sourceStructure
                                                               << itemsToRemove
                                                               << outputStructure;

        sourceStructure.clear();
        sourceStructure << c1
                        << _+t3
                        << _+t4;

        itemsToRemove.clear();
        itemsToRemove << c1 % t4;

        outputStructure.clear();
        outputStructure << inbox
                        << c1
                        << _+t3;

        QTest::newRow( "Remove all children under a project with tag" ) << sourceStructure
                                                                        << itemsToRemove
                                                                        << outputStructure;

    }

    void shouldReactToSourceRowRemovals()
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

        //create todoTreeModel
        ReparentingModel todoTreeModel(new ProjectStrategy);
        ModelTest t2(&todoTreeModel);

        todoTreeModel.setSourceModel(&metadataModel);

        // create sidebarmodel
        SideBarModel treeSideBarModel;
        ModelTest t3(&treeSideBarModel);

        treeSideBarModel.setSourceModel(&todoTreeModel);

        //WHEN
        QFETCH(ModelPath::List, itemsToRemove);

        ModelUtils::destroy(&source, itemsToRemove);

        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        StandardModelBuilderBehavior outputBehavior;
        outputBehavior.setSingleColumnEnabled(true);
        ModelUtils::create(&output, outputStructure, ModelPath(), &outputBehavior);

        QCOMPARE(treeSideBarModel, output);
    }
};

QTEST_KDEMAIN(SideBarModelTest, GUI)

#include "sidebarmodelspec.moc"
