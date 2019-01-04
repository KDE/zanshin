/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
   Copyright 2015 David Faure <faure@kde.org>

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

#include <QStandardItem>
#include <testlib/qtest_zanshin.h>

#include <mockitopp/mockitopp.hpp>

#include "utils/mockobject.h"

#include "domain/contextqueries.h"
#include "domain/contextrepository.h"
#include "domain/projectqueries.h"
#include "domain/projectrepository.h"
#include "domain/task.h"
#include "domain/taskrepository.h"

#include "presentation/availablepagessortfilterproxymodel.h"
#include "presentation/availabletaskpagesmodel.h"
#include "presentation/contextpagemodel.h"
#include "presentation/projectpagemodel.h"
#include "presentation/querytreemodelbase.h"

#include "testlib/fakejob.h"

using namespace mockitopp;
using namespace mockitopp::matcher;

static QString extractChildRowsTexts( QAbstractItemModel *model, int row, const QModelIndex &parent = QModelIndex() ) {
    QString result;
    const QModelIndex index = model->index(row, 0, parent);
    const int children = model->rowCount(index);
    for (int row = 0; row < children; ++row) {
        const QString txt = model->index(row, 0, index).data().toString();
        result += txt.isEmpty() ? QStringLiteral(" ") : txt;
        if ( row + 1 < children )
            result += ';';
    }
    return result;
}

class AvailablePagesSortFilterProxyModelTest : public QObject
{
    Q_OBJECT
private slots:

    void shouldSortSecondLevel()
    {
        // GIVEN a tree model as source
        QStandardItemModel sourceModel;
        sourceModel.appendRow(new QStandardItem(QStringLiteral("Projects")));
        sourceModel.item(0, 0)->appendRow(new QStandardItem(QStringLiteral("D")));
        sourceModel.item(0, 0)->appendRow(new QStandardItem(QStringLiteral("A")));
        sourceModel.item(0, 0)->appendRow(new QStandardItem(QStringLiteral("F")));
        sourceModel.appendRow(new QStandardItem(QStringLiteral("Contexts")));
        sourceModel.item(1, 0)->appendRow(new QStandardItem(QStringLiteral("K")));
        sourceModel.item(1, 0)->appendRow(new QStandardItem(QStringLiteral("D")));
        sourceModel.item(1, 0)->appendRow(new QStandardItem(QStringLiteral("E")));
        sourceModel.appendRow(new QStandardItem(QStringLiteral("Tags")));

        QCOMPARE(sourceModel.index(0, 0).data().toString(), QStringLiteral("Projects"));
        QCOMPARE(extractChildRowsTexts(&sourceModel, 0), QStringLiteral("D;A;F"));
        QCOMPARE(sourceModel.index(1, 0).data().toString(), QStringLiteral("Contexts"));
        QCOMPARE(extractChildRowsTexts(&sourceModel, 1), QStringLiteral("K;D;E"));
        QCOMPARE(sourceModel.index(2, 0).data().toString(), QStringLiteral("Tags"));

        // WHEN putting an AvailablePagesSortFilterProxyModel on top
        Presentation::AvailablePagesSortFilterProxyModel proxy;
        proxy.setSourceModel(&sourceModel);

        // THEN the projects and contexts should be sorted (but not the toplevel items)
        QCOMPARE(proxy.index(0, 0).data().toString(), QStringLiteral("Projects"));
        QCOMPARE(extractChildRowsTexts(&proxy, 0), QStringLiteral("A;D;F"));
        QCOMPARE(proxy.index(1, 0).data().toString(), QStringLiteral("Contexts"));
        QCOMPARE(extractChildRowsTexts(&proxy, 1), QStringLiteral("D;E;K"));
        QCOMPARE(proxy.index(2, 0).data().toString(), QStringLiteral("Tags"));
    }

private:

};

ZANSHIN_TEST_MAIN(AvailablePagesSortFilterProxyModelTest)

#include "availablepagessortfilterproxymodeltest.moc"
