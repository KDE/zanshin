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

#include <testlib/qtest_zanshin.h>

#include "utils/mockobject.h"

#define ZANSHIN_I_SWEAR_I_AM_IN_A_PRESENTATION_TEST

#include "domain/datasourcequeries.h"
#include "domain/datasourcerepository.h"

#include "presentation/availablesourcesmodel.h"
#include "presentation/querytreemodelbase.h"
#include "presentation/errorhandler.h"

#include "testlib/fakejob.h"

Q_DECLARE_METATYPE(QModelIndex);

using namespace mockitopp;
using namespace mockitopp::matcher;

class FakeErrorHandler : public Presentation::ErrorHandler
{
public:
    void doDisplayMessage(const QString &message) override
    {
        m_message = message;
    }

    QString m_message;
};

class AvailableSourcesModelTest : public QObject
{
    Q_OBJECT
public:
    explicit AvailableSourcesModelTest(QObject *parent = nullptr)
        : QObject(parent)
    {
        qRegisterMetaType<QModelIndex>();
    }

private slots:
    void shouldListAvailableSources()
    {
        // GIVEN

        // Two top level sources
        auto source1 = Domain::DataSource::Ptr::create();
        source1->setName(QStringLiteral("Source 1"));
        source1->setIconName(QStringLiteral("foo-icon"));
        source1->setSelected(true);
        auto source2 = Domain::DataSource::Ptr::create();
        source2->setName(QStringLiteral("Source 2"));
        source2->setSelected(false);
        source2->setContentTypes(Domain::DataSource::Tasks);
        auto topLevelProvider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto topLevelResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(topLevelProvider);
        topLevelProvider->append(source1);
        topLevelProvider->append(source2);

        // Two other sources under source1
        auto source3 = Domain::DataSource::Ptr::create();
        source3->setName(QStringLiteral("Source 3"));
        source3->setSelected(false);
        source3->setContentTypes(Domain::DataSource::Notes);
        auto source4 = Domain::DataSource::Ptr::create();
        source4->setSelected(true);
        source4->setName(QStringLiteral("Source 4"));
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


        Utils::MockObject<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findTopLevel).when().thenReturn(topLevelResult);
        sourceQueriesMock(&Domain::DataSourceQueries::findChildren).when(source1).thenReturn(source1Result);
        sourceQueriesMock(&Domain::DataSourceQueries::findChildren).when(source2).thenReturn(source2Result);
        sourceQueriesMock(&Domain::DataSourceQueries::findChildren).when(source3).thenReturn(source3Result);
        sourceQueriesMock(&Domain::DataSourceQueries::findChildren).when(source4).thenReturn(source4Result);
        // We'll simulate a default source change later on
        sourceQueriesMock(&Domain::DataSourceQueries::isDefaultSource).when(source1).thenReturn(false);
        sourceQueriesMock(&Domain::DataSourceQueries::isDefaultSource).when(source2).thenReturn(true)
                                                                                    .thenReturn(false);
        sourceQueriesMock(&Domain::DataSourceQueries::isDefaultSource).when(source3).thenReturn(false);
        sourceQueriesMock(&Domain::DataSourceQueries::isDefaultSource).when(source4).thenReturn(false)
                                                                                    .thenReturn(false)
                                                                                    .thenReturn(true);

        Utils::MockObject<Domain::DataSourceRepository> sourceRepositoryMock;

        Presentation::AvailableSourcesModel sources(sourceQueriesMock.getInstance(),
                                                    sourceRepositoryMock.getInstance(),
                                                    nullptr);

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
        QCOMPARE(model->data(source2Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QStringLiteral("folder"));
        QCOMPARE(model->data(source3Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QStringLiteral("folder"));
        QCOMPARE(model->data(source4Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QStringLiteral("folder"));

        QCOMPARE(model->data(source1Index, Presentation::QueryTreeModelBase::IsDefaultRole).toBool(), false);
        QCOMPARE(model->data(source2Index, Presentation::QueryTreeModelBase::IsDefaultRole).toBool(), true);
        QCOMPARE(model->data(source3Index, Presentation::QueryTreeModelBase::IsDefaultRole).toBool(), false);
        QCOMPARE(model->data(source4Index, Presentation::QueryTreeModelBase::IsDefaultRole).toBool(), false);

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

        // WHEN
        QSignalSpy spy(model, &QAbstractItemModel::dataChanged);
        sourceQueriesMock(&Domain::DataSourceQueries::changeDefaultSource).when(source4).thenReturn();
        sources.setDefaultItem(source4Index);

        // THEN
        QCOMPARE(model->data(source1Index, Presentation::QueryTreeModelBase::IsDefaultRole).toBool(), false);
        QCOMPARE(model->data(source2Index, Presentation::QueryTreeModelBase::IsDefaultRole).toBool(), false);
        QCOMPARE(model->data(source3Index, Presentation::QueryTreeModelBase::IsDefaultRole).toBool(), false);
        QCOMPARE(model->data(source4Index, Presentation::QueryTreeModelBase::IsDefaultRole).toBool(), true);

        // Not overly efficient way of signaling the change, but doesn't happen often
        QCOMPARE(spy.count(), 4);
        QCOMPARE(spy.at(0).at(0).toModelIndex(), source1Index);
        QCOMPARE(spy.at(0).at(1).toModelIndex(), source1Index);
        QCOMPARE(spy.at(1).at(0).toModelIndex(), source3Index);
        QCOMPARE(spy.at(1).at(1).toModelIndex(), source3Index);
        QCOMPARE(spy.at(2).at(0).toModelIndex(), source4Index);
        QCOMPARE(spy.at(2).at(1).toModelIndex(), source4Index);
        QCOMPARE(spy.at(3).at(0).toModelIndex(), source2Index);
        QCOMPARE(spy.at(3).at(1).toModelIndex(), source2Index);

        QVERIFY(sourceQueriesMock(&Domain::DataSourceQueries::changeDefaultSource).when(source4).exactly(1));
    }

    void shouldGetAnErrorMessageWhenSetDataSourceFailed()
    {
        // GIVEN

        // Two top level sources
        auto source1 = Domain::DataSource::Ptr::create();
        source1->setName(QStringLiteral("Source 1"));
        source1->setIconName(QStringLiteral("foo-icon"));
        source1->setSelected(false);
        source1->setContentTypes(Domain::DataSource::Tasks);
        auto topLevelProvider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto topLevelResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(topLevelProvider);
        topLevelProvider->append(source1);

        // Nothing under source1
        auto source1Provider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto source1Result = Domain::QueryResult<Domain::DataSource::Ptr>::create(source1Provider);

        Utils::MockObject<Domain::DataSourceQueries> sourceQueriesMock;
        sourceQueriesMock(&Domain::DataSourceQueries::findTopLevel).when().thenReturn(topLevelResult);
        sourceQueriesMock(&Domain::DataSourceQueries::findChildren).when(source1).thenReturn(source1Result);

        Utils::MockObject<Domain::DataSourceRepository> sourceRepositoryMock;

        Presentation::AvailableSourcesModel sources(sourceQueriesMock.getInstance(),
                                                    sourceRepositoryMock.getInstance(),
                                                    nullptr);
        FakeErrorHandler errorHandler;
        sources.setErrorHandler(&errorHandler);

        // WHEN
        QAbstractItemModel *model = sources.sourceListModel();

        // THEN
        const QModelIndex source1Index = model->index(0, 0);

        // WHEN
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
        sourceRepositoryMock(&Domain::DataSourceRepository::update).when(source1).thenReturn(job);

        QVERIFY(model->setData(source1Index, Qt::Unchecked, Qt::CheckStateRole));

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot modify source Source 1: Foo"));
    }

    void shouldExecBackendSettingsDialog()
    {
        // GIVEN
        Utils::MockObject<Domain::DataSourceRepository> sourceRepositoryMock;
        sourceRepositoryMock(&Domain::DataSourceRepository::showConfigDialog).when().thenReturn();

        Presentation::AvailableSourcesModel sources(Domain::DataSourceQueries::Ptr(),
                                                    sourceRepositoryMock.getInstance());

        // WHEN
        sources.showConfigDialog();

        // THEN
        QVERIFY(sourceRepositoryMock(&Domain::DataSourceRepository::showConfigDialog).when().exactly(1));
    }
};

ZANSHIN_TEST_MAIN(AvailableSourcesModelTest)

#include "availablesourcesmodeltest.moc"
