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
        auto source2 = Domain::DataSource::Ptr::create();
        source2->setName("Source 2");
        auto topLevelProvider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto topLevelResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(topLevelProvider);
        topLevelProvider->append(source1);
        topLevelProvider->append(source2);

        // Two other sources under source1
        auto source3 = Domain::DataSource::Ptr::create();
        source3->setName("Source 3");
        auto source4 = Domain::DataSource::Ptr::create();
        source4->setName("Source 4");
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

        Presentation::AvailableSourcesModel sources(&sourceQueriesMock.getInstance(),
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

        QCOMPARE(model->data(source1Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), source1->iconName());
        QCOMPARE(model->data(source2Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("folder"));
        QCOMPARE(model->data(source3Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("folder"));
        QCOMPARE(model->data(source4Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QString("folder"));
    }
};

QTEST_MAIN(AvailableSourcesModelTest)

#include "availablesourcesmodeltest.moc"
