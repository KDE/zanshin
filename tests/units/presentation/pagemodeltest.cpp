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

#include <QStringListModel>

#include "presentation/pagemodel.h"

class FakePageModel : public Presentation::PageModel
{
    Q_OBJECT
public:
    explicit FakePageModel(QObject *parent = 0)
        : Presentation::PageModel(0, 0, 0, 0, parent),
          createCount(0),
          itemModel(0)
    {
    }

    void addTask(const QString &) {}
    void removeItem(const QModelIndex &) {}

private:
    QAbstractItemModel *createCentralListModel()
    {
        createCount++;
        itemModel = new QStringListModel(this);
        return itemModel;
    }

public:
    int createCount;
    QAbstractItemModel *itemModel;
};

class PageModelTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldLazilyCreateModelOnlyOnce()
    {
        // GIVEN
        FakePageModel model;
        QCOMPARE(model.createCount, 0);
        QVERIFY(!model.itemModel);

        // WHEN
        QAbstractItemModel *itemModel = model.centralListModel();

        // THEN
        QCOMPARE(model.createCount, 1);
        QCOMPARE(itemModel, model.itemModel);

        // WHEN
        itemModel = model.centralListModel();

        // THEN
        QCOMPARE(model.createCount, 1);
        QCOMPARE(itemModel, model.itemModel);
    }
};

QTEST_MAIN(PageModelTest)

#include "pagemodeltest.moc"
