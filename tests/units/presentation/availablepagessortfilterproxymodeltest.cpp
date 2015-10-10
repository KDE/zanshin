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
#include <QtTest>

#include <mockitopp/mockitopp.hpp>

#include "utils/mockobject.h"

#include "domain/contextqueries.h"
#include "domain/contextrepository.h"
#include "domain/projectqueries.h"
#include "domain/projectrepository.h"
#include "domain/note.h"
#include "domain/tag.h"
#include "domain/tagqueries.h"
#include "domain/tagrepository.h"
#include "domain/task.h"
#include "domain/taskrepository.h"

#include "presentation/availablepagessortfilterproxymodel.h"
#include "presentation/availabletaskpagesmodel.h"
#include "presentation/contextpagemodel.h"
#include "presentation/inboxpagemodel.h"
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
        result += txt.isEmpty() ? QString(" ") : txt;
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
        sourceModel.appendRow(new QStandardItem("Projects"));
        sourceModel.item(0, 0)->appendRow(new QStandardItem("D"));
        sourceModel.item(0, 0)->appendRow(new QStandardItem("A"));
        sourceModel.item(0, 0)->appendRow(new QStandardItem("F"));
        sourceModel.appendRow(new QStandardItem("Contexts"));
        sourceModel.item(1, 0)->appendRow(new QStandardItem("K"));
        sourceModel.item(1, 0)->appendRow(new QStandardItem("D"));
        sourceModel.item(1, 0)->appendRow(new QStandardItem("E"));
        sourceModel.appendRow(new QStandardItem("Tags"));

        QCOMPARE(sourceModel.index(0, 0).data().toString(), QString("Projects"));
        QCOMPARE(extractChildRowsTexts(&sourceModel, 0), QString("D;A;F"));
        QCOMPARE(sourceModel.index(1, 0).data().toString(), QString("Contexts"));
        QCOMPARE(extractChildRowsTexts(&sourceModel, 1), QString("K;D;E"));
        QCOMPARE(sourceModel.index(2, 0).data().toString(), QString("Tags"));

        // WHEN putting an AvailablePagesSortFilterProxyModel on top
        Presentation::AvailablePagesSortFilterProxyModel proxy;
        proxy.setSourceModel(&sourceModel);

        // THEN the projects and contexts should be sorted (but not the toplevel items)
        QCOMPARE(proxy.index(0, 0).data().toString(), QString("Projects"));
        QCOMPARE(extractChildRowsTexts(&proxy, 0), QString("A;D;F"));
        QCOMPARE(proxy.index(1, 0).data().toString(), QString("Contexts"));
        QCOMPARE(extractChildRowsTexts(&proxy, 1), QString("D;E;K"));
        QCOMPARE(proxy.index(2, 0).data().toString(), QString("Tags"));
    }

private:

};

QTEST_MAIN(AvailablePagesSortFilterProxyModelTest)

#include "availablepagessortfilterproxymodeltest.moc"
