/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include <QStringListModel>

#include "presentation/pagemodel.h"

class FakePageModel : public Presentation::PageModel
{
    Q_OBJECT
public:
    explicit FakePageModel(QObject *parent = nullptr)
        : Presentation::PageModel(parent),
          createCount(0),
          itemModel(nullptr)
    {
    }

    Domain::Task::Ptr addItem(const QString &, const QModelIndex &) override { return Domain::Task::Ptr::create(); }
    void removeItem(const QModelIndex &) override {}
    void promoteItem(const QModelIndex &) override {}

private:
    QAbstractItemModel *createCentralListModel() override
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

ZANSHIN_TEST_MAIN(PageModelTest)

#include "pagemodeltest.moc"
