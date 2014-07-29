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

#include <QtTest>

#include <QHeaderView>
#include <QTreeView>
#include <QAbstractItemModel>
#include <QStringListModel>

#include "widgets/itemdelegate.h"
#include "widgets/pageview.h"

Q_DECLARE_METATYPE(QAbstractItemModel*)

class PageViewTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldHaveDefaultState()
    {
        Widgets::PageView page;

        auto centralView = page.findChild<QTreeView*>("centralView");
        QVERIFY(centralView);
        QVERIFY(centralView->isVisibleTo(&page));
        QVERIFY(!centralView->header()->isVisibleTo(&page));
        QVERIFY(qobject_cast<Widgets::ItemDelegate*>(centralView->itemDelegate()));
        QVERIFY(centralView->alternatingRowColors());
    }

    void shouldDisplayListFromPageModel()
    {
        // GIVEN
        QStringListModel model(QStringList() << "A" << "B" << "C" );

        QObject stubPageModel;
        stubPageModel.setProperty("centralListModel", QVariant::fromValue(static_cast<QAbstractItemModel*>(&model)));

        Widgets::PageView page;
        auto centralView = page.findChild<QTreeView*>("centralView");
        QVERIFY(centralView);
        QVERIFY(!centralView->model());

        // WHEN
        page.setModel(&stubPageModel);

        // THEN
        QCOMPARE(centralView->model(), &model);
    }
};

QTEST_MAIN(PageViewTest)

#include "pageviewtest.moc"
