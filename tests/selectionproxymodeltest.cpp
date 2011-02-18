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


class SelectionProxyModelTest : public QObject
{
    Q_OBJECT
private slots:
    void setSelectionModelBasicTest()
    {
        QStandardItemModel baseModel;
        QItemSelectionModel selectionModel(&baseModel);
        SelectionProxyModel proxyModel;
        proxyModel.setSelectionModel(&selectionModel);
        proxyModel.setSourceModel(&baseModel);
        QVERIFY(proxyModel.sourceModel() == &baseModel);
    }

    void shouldNotCrashWithAnEmptySelectionModel()
    {
        QStandardItemModel treeModel;
        SelectionProxyModel proxyModel;
        proxyModel.setSourceModel(&treeModel);
        QVERIFY(proxyModel.sourceModel() == &treeModel);
    }

    void shouldCallSetSourceModelBeforeSetSelectionModel()
    {
        QStandardItemModel baseModel;
        QItemSelectionModel selectionModel(&baseModel);
        SelectionProxyModel proxyModel;
        proxyModel.setSourceModel(&baseModel);
        proxyModel.setSelectionModel(&selectionModel);
        QVERIFY(proxyModel.sourceModel() == &baseModel);
    }
};

QTEST_KDEMAIN(SelectionProxyModelTest, GUI)

#include "selectionproxymodeltest.moc"
