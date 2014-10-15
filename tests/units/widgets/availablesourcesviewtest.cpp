/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>

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

#include <QtTestGui>

#include <QAbstractItemModel>
#include <QHeaderView>
#include <QStringListModel>
#include <QTreeView>

#include "presentation/metatypes.h"

#include "widgets/availablesourcesview.h"

class AvailableSourcesViewTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldHaveDefaultState()
    {
        Widgets::AvailableSourcesView available;

        QVERIFY(!available.model());

        auto sourcesView = available.findChild<QTreeView*>("sourcesView");
        QVERIFY(sourcesView);
        QVERIFY(sourcesView->isVisibleTo(&available));
        QVERIFY(!sourcesView->header()->isVisibleTo(&available));
    }

    void shouldDisplayListFromPageModel()
    {
        // GIVEN
        QStringListModel model(QStringList() << "A" << "B" << "C" );

        QObject stubPagesModel;
        stubPagesModel.setProperty("sourceListModel", QVariant::fromValue(static_cast<QAbstractItemModel*>(&model)));

        Widgets::AvailableSourcesView available;
        auto sourcesView = available.findChild<QTreeView*>("sourcesView");
        QVERIFY(sourcesView);
        QVERIFY(!sourcesView->model());

        // WHEN
        available.setModel(&stubPagesModel);
        QTest::qWait(10);

        // THEN
        QCOMPARE(sourcesView->model(), &model);
    }
};

QTEST_MAIN(AvailableSourcesViewTest)

#include "availablesourcesviewtest.moc"
