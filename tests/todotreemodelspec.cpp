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

#include "todotreemodel.h"
#include "testlib/testlib.h"
#include "testlib/modeltest.h"

using namespace Zanshin::Test;

class TodoTreeModelSpec : public QObject
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
        TodoTreeModel treeModel;
        ModelTest t1(&treeModel);

        //WHEN
        treeModel.setSourceModel(&baseModel);

        //THEN
        QVERIFY(treeModel.sourceModel() == &baseModel);
    }

};

QTEST_KDEMAIN(TodoTreeModelSpec, GUI)

#include "todotreemodelspec.moc"
