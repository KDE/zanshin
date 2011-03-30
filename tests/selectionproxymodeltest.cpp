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

#include "selectionproxymodel.h"
#include "testlib/testlib.h"

using namespace Zanshin::Test;

class SelectionProxyModelTest : public QObject
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
        QItemSelectionModel selectionModel(&baseModel);
        SelectionProxyModel proxyModel;

        //WHEN
        proxyModel.setSelectionModel(&selectionModel);
        proxyModel.setSourceModel(&baseModel);

        //THEN
        QVERIFY(proxyModel.sourceModel() == &baseModel);
    }

    void shouldNotCrashWithAnEmptySelectionModel()
    {
        //GIVEN
        QStandardItemModel treeModel;
        SelectionProxyModel proxyModel;

        //WHEN
        proxyModel.setSourceModel(&treeModel);

        //THEN
        QVERIFY(proxyModel.sourceModel() == &treeModel);
    }

    void shouldCallSetSourceModelBeforeSetSelectionModel()
    {
        //GIVEN
        QStandardItemModel baseModel;
        QItemSelectionModel selectionModel(&baseModel);
        SelectionProxyModel proxyModel;

        //WHEN
        proxyModel.setSourceModel(&baseModel);
        proxyModel.setSelectionModel(&selectionModel);

        //THEN
        QVERIFY(proxyModel.sourceModel() == &baseModel);
    }

    void shouldKeepAscendantAndDescendantOfSelectedItems_data()
    {
        QTest::addColumn<ModelStructure>( "sourceStructure" );
        QTest::addColumn<ModelPath::List>( "itemsToSelect" );
        QTest::addColumn<ModelStructure>( "outputStructure" );

        // Base items
        C c1(1, 0, "c1");
        C c2(2, 0, "c2");
        C c3(3, 2, "c3");
        C c4(4, 3, "c4");
        C c5(5, 3, "c5");
        T t1(1, 3, "t1", QString(), "t1");
        T t2(2, 3, "t2", QString(), "t2");
        T t3(3, 4, "t3", QString(), "t3");
        T t4(4, 4, "t4", QString(), "t4");
        T t5(5, 5, "t5", QString(), "t5");

        // Create the source structure once and for all
        ModelStructure sourceStructure;
        sourceStructure << c1
                        << c2
                        << _+c3
                        << __+t1
                        << __+t2
                        << __+c4
                        << ___+t3
                        << ___+t4
                        << __+c5
                        << ___+t5;


        ModelPath::List itemsToSelect;
        ModelStructure outputStructure;

        QTest::newRow( "empty selection" ) << sourceStructure << itemsToSelect << outputStructure;

        itemsToSelect << c2 % c3 % c4;
        outputStructure << c2
                        << _+c3
                        << __+c4
                        << ___+t3
                        << ___+t4;

        QTest::newRow( "one item selected" ) << sourceStructure << itemsToSelect << outputStructure;

        itemsToSelect.clear();
        itemsToSelect << c2 % c3 % c4
                      << c2 % c3 % c5;
        outputStructure.clear();
        outputStructure << c2
                        << _+c3
                        << __+c4
                        << ___+t3
                        << ___+t4
                        << __+c5
                        << ___+t5;

        QTest::newRow( "two items selected" ) << sourceStructure << itemsToSelect << outputStructure;
    }

    void shouldKeepAscendantAndDescendantOfSelectedItems()
    {
        //GIVEN
        QFETCH(ModelStructure, sourceStructure);

        //Source model
        ModelBuilder builder;
        QStandardItemModel source;
        builder.setModel(&source);
        builder.create(sourceStructure);

        //Selection proxy
        QItemSelectionModel selection(&source);
        SelectionProxyModel proxyModel;
        proxyModel.setSelectionModel(&selection);
        proxyModel.setSourceModel(&source);

        //WHEN
        QFETCH(ModelPath::List, itemsToSelect);

        // We change the selection
        foreach (const ModelPath &path, itemsToSelect) {
            QModelIndex index = ModelUtils::locateItem(&source, path);
            selection.select(index, QItemSelectionModel::Select);
        }

        //THEN
        QFETCH(ModelStructure, outputStructure);
        QStandardItemModel output;
        builder.setModel(&output);
        builder.create(outputStructure);
        QCOMPARE(proxyModel, output);
    }
};

QTEST_KDEMAIN(SelectionProxyModelTest, GUI)

#include "selectionproxymodeltest.moc"
