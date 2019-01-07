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

#include <testlib/qtest_gui_zanshin.h>

#include <QAction>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QToolBar>
#include <QTreeView>

#include <KLineEdit>

#include "presentation/metatypes.h"
#include "presentation/querytreemodelbase.h"

#include "widgets/availablesourcesview.h"
#include "widgets/datasourcedelegate.h"

class AvailableSourcesModelStub : public QObject
{
    Q_OBJECT
public:
    explicit AvailableSourcesModelStub(QObject *parent = nullptr)
        : QObject(parent),
          settingsCalled(false)
    {
    }

public slots:
    void showConfigDialog()
    {
        settingsCalled = true;
    }

    void setDefaultItem(const QModelIndex &index)
    {
        defaultIndex = index;
    }

public:
    bool settingsCalled;
    QPersistentModelIndex defaultIndex;
};


class AvailableSourcesViewTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldHaveDefaultState()
    {
        Widgets::AvailableSourcesView available;

        QVERIFY(!available.model());

        auto sourcesView = available.findChild<QTreeView*>(QStringLiteral("sourcesView"));
        QVERIFY(sourcesView);
        QVERIFY(sourcesView->isVisibleTo(&available));
        QVERIFY(!sourcesView->header()->isVisibleTo(&available));
        auto delegate = qobject_cast<Widgets::DataSourceDelegate*>(sourcesView->itemDelegate());
        QVERIFY(delegate);

        auto proxy = qobject_cast<QSortFilterProxyModel*>(sourcesView->model());
        QVERIFY(proxy);
        QVERIFY(proxy->dynamicSortFilter());
        QCOMPARE(proxy->sortColumn(), 0);
        QCOMPARE(proxy->sortOrder(), Qt::AscendingOrder);

        auto actionBar = available.findChild<QToolBar*>(QStringLiteral("actionBar"));
        QVERIFY(actionBar);
        QVERIFY(actionBar->isVisibleTo(&available));

        auto defaultAction = available.findChild<QAction*>(QStringLiteral("defaultAction"));
        QVERIFY(defaultAction);

        auto settingsAction = available.findChild<QAction*>(QStringLiteral("settingsAction"));
        QVERIFY(settingsAction);

        auto actions = available.globalActions();
        QCOMPARE(actions.value(QStringLiteral("options_configure")), settingsAction);
    }

    void shouldDisplayListFromPageModel()
    {
        // GIVEN
        QStringListModel model(QStringList() << QStringLiteral("A") << QStringLiteral("B") << QStringLiteral("C") );

        QObject stubPagesModel;
        stubPagesModel.setProperty("sourceListModel", QVariant::fromValue(static_cast<QAbstractItemModel*>(&model)));

        Widgets::AvailableSourcesView available;
        auto sourcesView = available.findChild<QTreeView*>(QStringLiteral("sourcesView"));
        QVERIFY(sourcesView);
        auto proxy = qobject_cast<QSortFilterProxyModel*>(sourcesView->model());
        QVERIFY(proxy);
        QVERIFY(!proxy->sourceModel());

        // WHEN
        available.setModel(&stubPagesModel);
        QTest::qWait(10);

        // THEN
        QCOMPARE(proxy->sourceModel(), &model);
    }

    void shouldNotCrashWithNullModel()
    {
        // GIVEN
        QStringListModel model(QStringList() << QStringLiteral("A") << QStringLiteral("B") << QStringLiteral("C") );

        QObject stubPagesModel;
        stubPagesModel.setProperty("sourceListModel", QVariant::fromValue(static_cast<QAbstractItemModel*>(&model)));

        Widgets::AvailableSourcesView available;
        auto sourcesView = available.findChild<QTreeView*>(QStringLiteral("sourcesView"));
        QVERIFY(sourcesView);
        auto proxy = qobject_cast<QSortFilterProxyModel*>(sourcesView->model());
        QVERIFY(proxy);
        QVERIFY(!proxy->sourceModel());

        available.setModel(&stubPagesModel);
        QTest::qWait(10);

        // WHEN
        available.setModel(nullptr);
        QTest::qWait(10);

        // THEN
        QVERIFY(!available.isEnabled());
        QVERIFY(!proxy->sourceModel());
    }

    void shouldSetSelectedAsDefault()
    {
        // GIVEN
        QStandardItemModel model;
        auto itemA = new QStandardItem(QStringLiteral("A"));
        auto sourceA = Domain::DataSource::Ptr::create();
        sourceA->setContentTypes(Domain::DataSource::Tasks);
        itemA->setData(QVariant::fromValue(sourceA), Presentation::QueryTreeModelBase::ObjectRole);
        model.appendRow(itemA);

        auto itemB = new QStandardItem(QStringLiteral("B"));
        auto sourceB = Domain::DataSource::Ptr::create();
        sourceB->setContentTypes(Domain::DataSource::Tasks);
        itemB->setData(QVariant::fromValue(sourceB), Presentation::QueryTreeModelBase::ObjectRole);
        model.appendRow(itemB);

        auto itemC = new QStandardItem(QStringLiteral("C"));
        auto sourceC = Domain::DataSource::Ptr::create();
        sourceC->setContentTypes(Domain::DataSource::NoContent);
        itemC->setData(QVariant::fromValue(sourceC), Presentation::QueryTreeModelBase::ObjectRole);
        model.appendRow(itemC);

        AvailableSourcesModelStub stubSourcesModel;
        stubSourcesModel.setProperty("sourceListModel", QVariant::fromValue(static_cast<QAbstractItemModel*>(&model)));

        Widgets::AvailableSourcesView available;
        available.setModel(&stubSourcesModel);

        auto sourcesView = available.findChild<QTreeView*>(QStringLiteral("sourcesView"));
        QVERIFY(sourcesView);
        auto proxy = qobject_cast<QSortFilterProxyModel*>(sourcesView->model());
        QVERIFY(proxy);

        auto defaultAction = available.findChild<QAction*>(QStringLiteral("defaultAction"));
        QVERIFY(defaultAction);

        // WHEN
        auto selectedIndex = QPersistentModelIndex(model.index(1, 0));
        sourcesView->setCurrentIndex(proxy->mapFromSource(selectedIndex));

        // THEN
        QVERIFY(defaultAction->isEnabled());

        // WHEN
        defaultAction->trigger();

        // THEN
        QCOMPARE(stubSourcesModel.defaultIndex, selectedIndex);

        // WHEN
        sourcesView->selectionModel()->clear();

        // THEN
        QVERIFY(!defaultAction->isEnabled());

        // WHEN
        selectedIndex = QPersistentModelIndex(model.index(2, 0));
        sourcesView->setCurrentIndex(proxy->mapFromSource(selectedIndex));

        // THEN
        QVERIFY(!defaultAction->isEnabled());
    }

    void shouldInvokeModelSettingsDialog()
    {
        // GIVEN
        AvailableSourcesModelStub stubSourcesModel;
        QVERIFY(!stubSourcesModel.settingsCalled);

        Widgets::AvailableSourcesView available;
        available.setModel(&stubSourcesModel);

        auto settingsAction = available.findChild<QAction*>(QStringLiteral("settingsAction"));
        QVERIFY(settingsAction);

        // WHEN
        settingsAction->trigger();

        // THEN
        QVERIFY(stubSourcesModel.settingsCalled);
    }
};

ZANSHIN_TEST_MAIN(AvailableSourcesViewTest)

#include "availablesourcesviewtest.moc"
