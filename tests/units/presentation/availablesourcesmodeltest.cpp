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

#include <mockitopp/mockitopp.hpp>

#include "domain/datasourcequeries.h"
#include "domain/datasourcerepository.h"

#include "presentation/availablesourcesmodel.h"
#include "presentation/querytreemodelbase.h"

#include "testlib/fakejob.h"

using namespace mockitopp;
using namespace mockitopp::matcher;

class AvailableSourcesModelTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldListAvailableSources()
    {
        // GIVEN

        // Two top level sources
        auto source1 = Domain::DataSource::Ptr::create();
        source1->setName("Source 1");
        source1->setIconName("foo-icon");
        source1->setSelected(true);
        auto source2 = Domain::DataSource::Ptr::create();
        source2->setName("Source 2");
        source2->setSelected(false);
        source2->setContentTypes(Domain::DataSource::Tasks);
        auto topLevelProvider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto topLevelResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(topLevelProvider);
        topLevelProvider->append(source1);
        topLevelProvider->append(source2);

        // Two other sources under source1
        auto source3 = Domain::DataSource::Ptr::create();
        source3->setName("Source 3");
        source3->setSelected(false);
        source3->setContentTypes(Domain::DataSource::Notes);
        auto source4 = Domain::DataSource::Ptr::create();
        source4->setSelected(true);
        source4->setName("Source 4");
        source4->setContentTypes(Domain::DataSource::Notes | Domain::DataSource::Tasks);
        auto source1Provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto source1Result = Domain::QueryResult<Domain::DataSource::Ptr>::create(source1Provider);
        source1Provider->append(source3);
        source1Provider->append(source4);

        // Nothing under source2, source3 or source4
        auto source2Provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto source2Result = Domain::QueryResult<Domain::DataSource::Ptr>::create(source2Provider);
        auto source3Provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto source3Result = Domain::QueryResult<Domain::DataSource::Ptr>::create(source3Provider);
        auto source4Provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto source4Result = Domain::QueryResult<Domain::DataSource::Ptr>::create(source4Provider);


        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findTopLevel).when().thenReturn(topLevelResult);
        sourceQueriesMock(&Domain::DataSourceQueries::findChildren).when(source1).thenReturn(source1Result);
        sourceQueriesMock(&Domain::DataSourceQueries::findChildren).when(source2).thenReturn(source2Result);
        sourceQueriesMock(&Domain::DataSourceQueries::findChildren).when(source3).thenReturn(source3Result);
        sourceQueriesMock(&Domain::DataSourceQueries::findChildren).when(source4).thenReturn(source4Result);

        mock_object<Domain::DataSourceRepository> sourceRepositoryMock;

        Presentation::AvailableSourcesModel sources(&sourceQueriesMock.getInstance(),
                                                    &sourceRepositoryMock.getInstance(),
                                                    0);

        // WHEN
        QAbstractItemModel *model = sources.sourceListModel();

        // THEN
        const QModelIndex source1Index = model->index(0, 0);
        const QModelIndex source2Index = model->index(1, 0);
        const QModelIndex source3Index = model->index(0, 0, source1Index);
        const QModelIndex source4Index = model->index(1, 0, source1Index);

        QCOMPARE(model->rowCount(), 2);
        QCOMPARE(model->rowCount(source1Index), 2);
        QCOMPARE(model->rowCount(source2Index), 0);
        QCOMPARE(model->rowCount(source3Index), 0);
        QCOMPARE(model->rowCount(source4Index), 0);

        const Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable
                                         | Qt::ItemIsEnabled;
        QCOMPARE(model->flags(source1Index), defaultFlags);
        QCOMPARE(model->flags(source2Index), defaultFlags | Qt::ItemIsUserCheckable);
        QCOMPARE(model->flags(source3Index), defaultFlags | Qt::ItemIsUserCheckable);
        QCOMPARE(model->flags(source4Index), defaultFlags | Qt::ItemIsUserCheckable);

        QCOMPARE(model->data(source1Index).toString(), source1->name());
        QCOMPARE(model->data(source2Index).toString(), source2->name());
        QCOMPARE(model->data(source3Index).toString(), source3->name());
        QCOMPARE(model->data(source4Index).toString(), source4->name());

        QCOMPARE(model->data(source1Index, Qt::EditRole).toString(), source1->name());
        QCOMPARE(model->data(source2Index, Qt::EditRole).toString(), source2->name());
        QCOMPARE(model->data(source3Index, Qt::EditRole).toString(), source3->name());
        QCOMPARE(model->data(source4Index, Qt::EditRole).toString(), source4->name());

        QVERIFY(!model->data(source1Index, Qt::CheckStateRole).isValid());
        QCOMPARE(model->data(source2Index, Qt::CheckStateRole).toBool(), source2->isSelected());
        QCOMPARE(model->data(source3Index, Qt::CheckStateRole).toBool(), source3->isSelected());
        QCOMPARE(model->data(source4Index, Qt::CheckStateRole).toBool(), source4->isSelected());

        QCOMPARE(model->data(source1Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), source1->iconName());
        QCOMPARE(model->data(source2Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("folder"));
        QCOMPARE(model->data(source3Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("folder"));
        QCOMPARE(model->data(source4Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("folder"));

        // WHEN
        sourceRepositoryMock(&Domain::DataSourceRepository::update).when(source2).thenReturn(new FakeJob(this));
        sourceRepositoryMock(&Domain::DataSourceRepository::update).when(source4).thenReturn(new FakeJob(this));

        QVERIFY(!model->setData(source1Index, Qt::Unchecked, Qt::CheckStateRole));
        QVERIFY(model->setData(source2Index, Qt::Checked, Qt::CheckStateRole));
        QVERIFY(model->setData(source4Index, Qt::Unchecked, Qt::CheckStateRole));

        // THEN
        QVERIFY(sourceRepositoryMock(&Domain::DataSourceRepository::update).when(source2).exactly(1));
        QVERIFY(sourceRepositoryMock(&Domain::DataSourceRepository::update).when(source4).exactly(1));

        QVERIFY(source2->isSelected());
        QVERIFY(!source4->isSelected());
    }

    void shouldListAvailableSearchSources()
    {
        // GIVEN

        // Two top level sources
        auto source1 = Domain::DataSource::Ptr::create();
        source1->setName("Source 1");
        source1->setIconName("foo-icon");
        source1->setSelected(true);
        auto source2 = Domain::DataSource::Ptr::create();
        source2->setName("Source 2");
        source2->setSelected(false);
        source2->setContentTypes(Domain::DataSource::Tasks);
        auto topLevelProvider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto topLevelResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(topLevelProvider);
        topLevelProvider->append(source1);
        topLevelProvider->append(source2);

        // Two other sources under source1
        auto source3 = Domain::DataSource::Ptr::create();
        source3->setName("Source 3");
        source3->setSelected(false);
        source3->setContentTypes(Domain::DataSource::Notes);
        auto source4 = Domain::DataSource::Ptr::create();
        source4->setSelected(true);
        source4->setName("Source 4");
        source4->setContentTypes(Domain::DataSource::Notes | Domain::DataSource::Tasks);
        auto source1Provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto source1Result = Domain::QueryResult<Domain::DataSource::Ptr>::create(source1Provider);
        source1Provider->append(source3);
        source1Provider->append(source4);

        // Nothing under source2, source3 or source4
        auto source2Provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto source2Result = Domain::QueryResult<Domain::DataSource::Ptr>::create(source2Provider);
        auto source3Provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto source3Result = Domain::QueryResult<Domain::DataSource::Ptr>::create(source3Provider);
        auto source4Provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto source4Result = Domain::QueryResult<Domain::DataSource::Ptr>::create(source4Provider);

        mock_object<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findSearchTopLevel).when().thenReturn(topLevelResult);
        sourceQueriesMock(&Domain::DataSourceQueries::findSearchChildren).when(source1).thenReturn(source1Result);
        sourceQueriesMock(&Domain::DataSourceQueries::findSearchChildren).when(source2).thenReturn(source2Result);
        sourceQueriesMock(&Domain::DataSourceQueries::findSearchChildren).when(source3).thenReturn(source3Result);
        sourceQueriesMock(&Domain::DataSourceQueries::findSearchChildren).when(source4).thenReturn(source4Result);

        mock_object<Domain::DataSourceRepository> sourceRepositoryMock;

        Presentation::AvailableSourcesModel sources(&sourceQueriesMock.getInstance(),
                                                    &sourceRepositoryMock.getInstance(),
                                                    0);

        // WHEN
        QAbstractItemModel *model = sources.searchListModel();

        // THEN
        const QModelIndex source1Index = model->index(0, 0);
        const QModelIndex source2Index = model->index(1, 0);
        const QModelIndex source3Index = model->index(0, 0, source1Index);
        const QModelIndex source4Index = model->index(1, 0, source1Index);

        QCOMPARE(model->rowCount(), 2);
        QCOMPARE(model->rowCount(source1Index), 2);
        QCOMPARE(model->rowCount(source2Index), 0);
        QCOMPARE(model->rowCount(source3Index), 0);
        QCOMPARE(model->rowCount(source4Index), 0);

        const Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable
                                         | Qt::ItemIsEnabled;
        QCOMPARE(model->flags(source1Index), defaultFlags);
        QCOMPARE(model->flags(source2Index), defaultFlags);
        QCOMPARE(model->flags(source3Index), defaultFlags);
        QCOMPARE(model->flags(source4Index), defaultFlags);

        QCOMPARE(model->data(source1Index).toString(), source1->name());
        QCOMPARE(model->data(source2Index).toString(), source2->name());
        QCOMPARE(model->data(source3Index).toString(), source3->name());
        QCOMPARE(model->data(source4Index).toString(), source4->name());

        QCOMPARE(model->data(source1Index, Qt::EditRole).toString(), source1->name());
        QCOMPARE(model->data(source2Index, Qt::EditRole).toString(), source2->name());
        QCOMPARE(model->data(source3Index, Qt::EditRole).toString(), source3->name());
        QCOMPARE(model->data(source4Index, Qt::EditRole).toString(), source4->name());

        QVERIFY(!model->data(source1Index, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(source2Index, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(source3Index, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(source4Index, Qt::CheckStateRole).isValid());

        QCOMPARE(model->data(source1Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), source1->iconName());
        QCOMPARE(model->data(source2Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("folder"));
        QCOMPARE(model->data(source3Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("folder"));
        QCOMPARE(model->data(source4Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("folder"));

        QVERIFY(!model->setData(source1Index, Qt::Unchecked, Qt::CheckStateRole));
        QVERIFY(!model->setData(source2Index, Qt::Checked, Qt::CheckStateRole));
        QVERIFY(!model->setData(source4Index, Qt::Unchecked, Qt::CheckStateRole));
    }

    void shouldChangeSourceToListed()
    {
        // GIVEN

        auto source = Domain::DataSource::Ptr::create();
        source->setName("Source");
        source->setIconName("folder");
        source->setContentTypes(Domain::DataSource::Tasks);
        source->setSelected(false);
        source->setListStatus(Domain::DataSource::Unlisted);


        mock_object<Domain::DataSourceQueries> sourceQueriesMock;

        mock_object<Domain::DataSourceRepository> sourceRepositoryMock;
        sourceRepositoryMock(&Domain::DataSourceRepository::update).when(source).thenReturn(new FakeJob(this));

        Presentation::AvailableSourcesModel sources(&sourceQueriesMock.getInstance(),
                                                    &sourceRepositoryMock.getInstance());

        // WHEN
        sources.listSource(source);

        // THEN
        QVERIFY(sourceRepositoryMock(&Domain::DataSourceRepository::update).when(source).exactly(1));
        QVERIFY(source->isSelected());
        QCOMPARE(source->listStatus(), Domain::DataSource::Listed);
    }

    void shouldChangeSourceToUnlisted()
    {
        // GIVEN

        auto source = Domain::DataSource::Ptr::create();
        source->setName("Source");
        source->setIconName("folder");
        source->setContentTypes(Domain::DataSource::Tasks);
        source->setSelected(true);
        source->setListStatus(Domain::DataSource::Bookmarked);


        mock_object<Domain::DataSourceQueries> sourceQueriesMock;

        mock_object<Domain::DataSourceRepository> sourceRepositoryMock;
        sourceRepositoryMock(&Domain::DataSourceRepository::update).when(source).thenReturn(new FakeJob(this));

        Presentation::AvailableSourcesModel sources(&sourceQueriesMock.getInstance(),
                                                    &sourceRepositoryMock.getInstance());

        // WHEN
        sources.unlistSource(source);

        // THEN
        QVERIFY(sourceRepositoryMock(&Domain::DataSourceRepository::update).when(source).exactly(1));
        QVERIFY(!source->isSelected());
        QCOMPARE(source->listStatus(), Domain::DataSource::Unlisted);
    }

    void shouldToggleSourceToBookmarkStatus_data()
    {
        QTest::addColumn<bool>("wasSelected");
        QTest::addColumn<bool>("wasBookmarked");
        QTest::newRow("unselected, not bookmarked") << false << false;
        QTest::newRow("selected, not bookmarked") << true << false;
        QTest::newRow("unselected, bookmarked") << false << true;
        QTest::newRow("selected, bookmarked") << true << true;
    }

    void shouldToggleSourceToBookmarkStatus()
    {
        // GIVEN
        QFETCH(bool, wasSelected);
        QFETCH(bool, wasBookmarked);

        auto source = Domain::DataSource::Ptr::create();
        source->setName("Source");
        source->setIconName("folder");
        source->setContentTypes(Domain::DataSource::Tasks);
        source->setSelected(wasSelected);
        if (wasBookmarked)
            source->setListStatus(Domain::DataSource::Bookmarked);
        else
            source->setListStatus(Domain::DataSource::Listed);


        mock_object<Domain::DataSourceQueries> sourceQueriesMock;

        mock_object<Domain::DataSourceRepository> sourceRepositoryMock;
        sourceRepositoryMock(&Domain::DataSourceRepository::update).when(source).thenReturn(new FakeJob(this));

        Presentation::AvailableSourcesModel sources(&sourceQueriesMock.getInstance(),
                                                    &sourceRepositoryMock.getInstance());

        // WHEN
        sources.bookmarkSource(source);

        // THEN
        QVERIFY(sourceRepositoryMock(&Domain::DataSourceRepository::update).when(source).exactly(1));
        QCOMPARE(source->isSelected(), wasSelected);
        if (wasBookmarked)
            QCOMPARE(source->listStatus(), Domain::DataSource::Listed);
        else
            QCOMPARE(source->listStatus(), Domain::DataSource::Bookmarked);
    }
};

QTEST_MAIN(AvailableSourcesModelTest)

#include "availablesourcesmodeltest.moc"
