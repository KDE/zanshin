/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2015 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

#include "presentation/availablepagesmodel.h"
#include "presentation/availablepagessortfilterproxymodel.h"
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
private Q_SLOTS:

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
