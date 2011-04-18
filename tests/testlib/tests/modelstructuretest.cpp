/* This file is part of Zanshin Todo.

   Copyright 2008-2011 Kevin Ottens <ervin@kde.org>

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

#include <testlib/dsl.h>
#include <testlib/modelutils.h>

using namespace Zanshin::Test;

class ModelStructureTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldCreateEmptyModels()
    {
        //GIVEN
        QStandardItemModel model;
        ModelStructure structure;

        //WHEN
        ModelUtils::create(&model, structure);

        //THEN
        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(model.columnCount(), 0);
    }

    void shouldCreateTreeStructures()
    {
        //GIVEN
        QStandardItemModel model;
        ModelStructure structure;

        C c1(1, 0, "c1");
        C c2(2, 0, "c2");
        C c3(3, 2, "c3");
        C c4(4, 3, "c4");
        Cat c5("category");
        T t1(1, 3, "t1", QString(), "t1");
        T t2(2, 3, "t2", QString(), "t2");
        T t3(3, 4, "t3", QString(), "t3");
        T t4(4, 4, "t4", QString(), "t4");
        T t5(5, 5, "t5", QString(), "t5");

        //WHEN
        structure << c1
                  << c2 // Make sure we can have several roots
                  << _+c3
                  << __+t1
                  << ___+t2 // To force c2 to be grand-grand-parent
                  << _+c4 // Make sure we can chain a second branch at same depth
                  << __+t3
                  << __+t4
                  << t5 // Make sure we can have roots after branches
                  << c5;

        ModelUtils::create(&model, structure);

        //THEN
        // Verify the roots
        QCOMPARE(model.rowCount(), 4);
        QCOMPARE(model.data(model.index(0, 0)).toString(), QString("c1"));
        QCOMPARE(model.data(model.index(1, 0)).toString(), QString("c2"));
        QCOMPARE(model.data(model.index(2, 0)).toString(), QString("t5"));
        QCOMPARE(model.data(model.index(3, 0)).toString(), QString("category"));

        // Verify c1 leaf
        QModelIndex c1Idx = model.index(0, 0);
        QCOMPARE(model.rowCount(c1Idx), 0);

        // Verify c2 branch
        QModelIndex c2Idx = model.index(1, 0);
        QCOMPARE(model.rowCount(c2Idx), 2);
        QCOMPARE(model.data(model.index(0, 0, c2Idx)).toString(), QString("c3"));
        QCOMPARE(model.data(model.index(1, 0, c2Idx)).toString(), QString("c4"));

        // Verify c3 branch
        QModelIndex c3Idx = model.index(0, 0, c2Idx);
        QCOMPARE(model.rowCount(c3Idx), 1);
        QCOMPARE(model.data(model.index(0, 0, c3Idx)).toString(), QString("t1"));

        QModelIndex t1Idx = model.index(0, 0, c3Idx);
        QCOMPARE(model.rowCount(t1Idx), 1);
        QCOMPARE(model.data(model.index(0, 0, t1Idx)).toString(), QString("t2"));
        QCOMPARE(model.rowCount(model.index(0, 0, t1Idx)), 0); // t2 is a leaf

        // Verify c4 branch
        QModelIndex c4Idx = model.index(1, 0, c2Idx);
        QCOMPARE(model.rowCount(c4Idx), 2);
        QCOMPARE(model.data(model.index(0, 0, c4Idx)).toString(), QString("t3"));
        QCOMPARE(model.rowCount(model.index(0, 0, c4Idx)), 0); // t3 is a leaf
        QCOMPARE(model.data(model.index(1, 0, c4Idx)).toString(), QString("t4"));
        QCOMPARE(model.rowCount(model.index(1, 0, c4Idx)), 0); // t4 is a leaf

        // Verify t5 leaf
        QModelIndex t5Idx = model.index(2, 0);
        QCOMPARE(model.rowCount(t5Idx), 0);
    }
};

QTEST_KDEMAIN(ModelStructureTest, GUI)

#include "modelstructuretest.moc"
