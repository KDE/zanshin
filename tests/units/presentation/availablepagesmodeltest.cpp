/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include <memory>

#include <QMimeData>

#include <KLocalizedString>

#include "utils/mockobject.h"
#include "utils/datetime.h"

#include "presentation/alltaskspagemodel.h"
#include "presentation/availablepagesmodel.h"
#include "presentation/contextpagemodel.h"
#include "presentation/errorhandler.h"
#include "presentation/inboxpagemodel.h"
#include "presentation/projectpagemodel.h"
#include "presentation/querytreemodelbase.h"
#include "presentation/workdaypagemodel.h"

#include "testlib/fakejob.h"

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

static const int s_projectsRow = 2;
static const int s_contextRow = 3;
static const int s_allTasksRow = 4;

class AvailablePagesModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void shouldListAvailablePages()
    {
        // GIVEN

        // Two selected data sources
        auto source1 = Domain::DataSource::Ptr::create();
        source1->setName("source1");
        auto source2 = Domain::DataSource::Ptr::create();
        source2->setName("source2");
        auto sourceProvider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(sourceProvider);
        sourceProvider->append(source1);
        sourceProvider->append(source2);

        // Two projects under source 1
        auto project11 = Domain::Project::Ptr::create();
        project11->setName(QStringLiteral("Project 11"));
        auto project12 = Domain::Project::Ptr::create();
        project12->setName(QStringLiteral("Project 12"));
        auto project1Provider = Domain::QueryResultProvider<Domain::Project::Ptr>::Ptr::create();
        auto project1Result = Domain::QueryResult<Domain::Project::Ptr>::create(project1Provider);
        project1Provider->append(project12);
        project1Provider->append(project11); // note: reversed order, to test sorting

        // Two projects under source 2
        auto project21 = Domain::Project::Ptr::create();
        project21->setName(QStringLiteral("Project 21"));
        auto project22 = Domain::Project::Ptr::create();
        project22->setName(QStringLiteral("Project 22"));
        auto project2Provider = Domain::QueryResultProvider<Domain::Project::Ptr>::Ptr::create();
        auto project2Result = Domain::QueryResult<Domain::Project::Ptr>::create(project2Provider);
        project2Provider->append(project22);
        project2Provider->append(project21); // note: reversed order, to test sorting

        // Two contexts
        auto context1 = Domain::Context::Ptr::create();
        context1->setName(QStringLiteral("context 1"));
        auto context2 = Domain::Context::Ptr::create();
        context2->setName(QStringLiteral("context 2"));
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);
        contextProvider->append(context1);
        contextProvider->append(context2);

        // One task (used for dropping later on)
        Domain::Task::Ptr taskToDrop(new Domain::Task);

        Utils::MockObject<Domain::DataSourceQueries> dataSourceQueriesMock;
        dataSourceQueriesMock(&Domain::DataSourceQueries::findAllSelected).when().thenReturn(sourceResult);
        dataSourceQueriesMock(&Domain::DataSourceQueries::findProjects).when(source1).thenReturn(project1Result);
        dataSourceQueriesMock(&Domain::DataSourceQueries::findProjects).when(source2).thenReturn(project2Result);

        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        Presentation::AvailablePagesModel pages(dataSourceQueriesMock.getInstance(),
                                                    Domain::ProjectQueries::Ptr(),
                                                    projectRepositoryMock.getInstance(),
                                                    contextQueriesMock.getInstance(),
                                                    contextRepositoryMock.getInstance(),
                                                    Domain::TaskQueries::Ptr(),
                                                    taskRepositoryMock.getInstance());

        // WHEN
        QAbstractItemModel *model = pages.pageListModel();

        // THEN
        const QModelIndex inboxIndex = model->index(0, 0);
        const QModelIndex workdayIndex = model->index(1, 0);
        const QModelIndex projectsIndex = model->index(s_projectsRow, 0);
        const QModelIndex source1Index = model->index(0, 0, projectsIndex);
        const QModelIndex project11Index = model->index(0, 0, source1Index);
        const QModelIndex project12Index = model->index(1, 0, source1Index);
        const QModelIndex source2Index = model->index(1, 0, projectsIndex);
        const QModelIndex project21Index = model->index(0, 0, source2Index);
        const QModelIndex project22Index = model->index(1, 0, source2Index);
        const QModelIndex contextsIndex = model->index(s_contextRow, 0);
        const QModelIndex context1Index = model->index(0, 0, contextsIndex);
        const QModelIndex context2Index = model->index(1, 0, contextsIndex);
        const QModelIndex allTasksIndex = model->index(s_allTasksRow, 0);

        QCOMPARE(model->rowCount(), 5);
        QCOMPARE(model->rowCount(inboxIndex), 0);
        QCOMPARE(model->rowCount(workdayIndex), 0);
        QCOMPARE(model->rowCount(projectsIndex), 2);
        QCOMPARE(model->rowCount(source1Index), 2);
        QCOMPARE(model->rowCount(project11Index), 0);
        QCOMPARE(model->rowCount(project12Index), 0);
        QCOMPARE(model->rowCount(source2Index), 2);
        QCOMPARE(model->rowCount(project21Index), 0);
        QCOMPARE(model->rowCount(project22Index), 0);
        QCOMPARE(model->rowCount(contextsIndex), 2);
        QCOMPARE(model->rowCount(context1Index), 0);
        QCOMPARE(model->rowCount(context2Index), 0);
        QCOMPARE(model->rowCount(allTasksIndex), 0);

        const Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable
                                         | Qt::ItemIsEnabled
                                         | Qt::ItemIsEditable;
        QCOMPARE(model->flags(inboxIndex), (defaultFlags & ~(Qt::ItemIsEditable)) | Qt::ItemIsDropEnabled);
        QCOMPARE(model->flags(workdayIndex), (defaultFlags & ~(Qt::ItemIsEditable)) | Qt::ItemIsDropEnabled);
        QCOMPARE(model->flags(projectsIndex), Qt::NoItemFlags);
        QCOMPARE(model->flags(source1Index), Qt::NoItemFlags);
        QCOMPARE(model->flags(project11Index), defaultFlags | Qt::ItemIsDropEnabled);
        QCOMPARE(model->flags(project12Index), defaultFlags | Qt::ItemIsDropEnabled);
        QCOMPARE(model->flags(source2Index), Qt::NoItemFlags);
        QCOMPARE(model->flags(project21Index), defaultFlags | Qt::ItemIsDropEnabled);
        QCOMPARE(model->flags(project22Index), defaultFlags | Qt::ItemIsDropEnabled);
        QCOMPARE(model->flags(contextsIndex), Qt::NoItemFlags);
        QCOMPARE(model->flags(context1Index), defaultFlags | Qt::ItemIsDropEnabled);
        QCOMPARE(model->flags(context2Index), defaultFlags | Qt::ItemIsDropEnabled);
        QCOMPARE(model->flags(allTasksIndex), (defaultFlags & ~(Qt::ItemIsEditable)));

        QCOMPARE(model->data(inboxIndex).toString(), i18n("Inbox"));
        QCOMPARE(model->data(workdayIndex).toString(), i18n("Workday"));
        QCOMPARE(model->data(projectsIndex).toString(), i18n("Projects"));
        QCOMPARE(model->data(source1Index).toString(), source1->name());
        QCOMPARE(model->data(project11Index).toString(), project11->name());
        QCOMPARE(model->data(project12Index).toString(), project12->name());
        QCOMPARE(model->data(source2Index).toString(), source2->name());
        QCOMPARE(model->data(project21Index).toString(), project21->name());
        QCOMPARE(model->data(project22Index).toString(), project22->name());
        QCOMPARE(model->data(contextsIndex).toString(), i18n("Contexts"));
        QCOMPARE(model->data(context1Index).toString(), context1->name());
        QCOMPARE(model->data(context2Index).toString(), context2->name());
        QCOMPARE(model->data(allTasksIndex).toString(), i18n("All Tasks"));

        QVERIFY(!model->data(inboxIndex, Qt::EditRole).isValid());
        QVERIFY(!model->data(workdayIndex, Qt::EditRole).isValid());
        QVERIFY(!model->data(projectsIndex, Qt::EditRole).isValid());
        QVERIFY(!model->data(source1Index, Qt::EditRole).isValid());
        QCOMPARE(model->data(project11Index, Qt::EditRole).toString(), project11->name());
        QCOMPARE(model->data(project12Index, Qt::EditRole).toString(), project12->name());
        QVERIFY(!model->data(source2Index, Qt::EditRole).isValid());
        QCOMPARE(model->data(project21Index, Qt::EditRole).toString(), project21->name());
        QCOMPARE(model->data(project22Index, Qt::EditRole).toString(), project22->name());
        QVERIFY(!model->data(contextsIndex, Qt::EditRole).isValid());
        QCOMPARE(model->data(context1Index, Qt::EditRole).toString(), context1->name());
        QCOMPARE(model->data(context2Index, Qt::EditRole).toString(), context2->name());
        QVERIFY(!model->data(allTasksIndex, Qt::EditRole).isValid());

        QCOMPARE(model->data(inboxIndex, Presentation::QueryTreeModelBase::IconNameRole).toString(), QStringLiteral("mail-folder-inbox"));
        QCOMPARE(model->data(workdayIndex, Presentation::QueryTreeModelBase::IconNameRole).toString(), QStringLiteral("go-jump-today"));
        QCOMPARE(model->data(projectsIndex, Presentation::QueryTreeModelBase::IconNameRole).toString(), QStringLiteral("folder"));
        QCOMPARE(model->data(source1Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QStringLiteral("folder"));
        QCOMPARE(model->data(project11Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QStringLiteral("view-pim-tasks"));
        QCOMPARE(model->data(project12Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QStringLiteral("view-pim-tasks"));
        QCOMPARE(model->data(source2Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QStringLiteral("folder"));
        QCOMPARE(model->data(project21Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QStringLiteral("view-pim-tasks"));
        QCOMPARE(model->data(project22Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QStringLiteral("view-pim-tasks"));
        QCOMPARE(model->data(contextsIndex, Presentation::QueryTreeModelBase::IconNameRole).toString(), QStringLiteral("folder"));
        QCOMPARE(model->data(context1Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QStringLiteral("view-pim-notes"));
        QCOMPARE(model->data(context2Index, Presentation::QueryTreeModelBase::IconNameRole).toString(), QStringLiteral("view-pim-notes"));
        QCOMPARE(model->data(allTasksIndex, Presentation::QueryTreeModelBase::IconNameRole).toString(), QStringLiteral("view-pim-tasks"));

        QVERIFY(!model->data(inboxIndex, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(workdayIndex, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(projectsIndex, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(source1Index, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(project11Index, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(project12Index, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(source2Index, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(project21Index, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(project22Index, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(contextsIndex, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(context1Index, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(context2Index, Qt::CheckStateRole).isValid());
        QVERIFY(!model->data(allTasksIndex, Qt::CheckStateRole).isValid());

        // WHEN
        projectRepositoryMock(&Domain::ProjectRepository::update).when(project11).thenReturn(new FakeJob(this));
        projectRepositoryMock(&Domain::ProjectRepository::update).when(project12).thenReturn(new FakeJob(this));
        projectRepositoryMock(&Domain::ProjectRepository::update).when(project21).thenReturn(new FakeJob(this));
        projectRepositoryMock(&Domain::ProjectRepository::update).when(project22).thenReturn(new FakeJob(this));
        contextRepositoryMock(&Domain::ContextRepository::update).when(context1).thenReturn(new FakeJob(this));
        contextRepositoryMock(&Domain::ContextRepository::update).when(context2).thenReturn(new FakeJob(this));

        QVERIFY(!model->setData(inboxIndex, "Foo"));
        QVERIFY(!model->setData(projectsIndex, "Foo"));
        QVERIFY(!model->setData(source1Index, "Foo"));
        QVERIFY(model->setData(project11Index, "New Project 11"));
        QVERIFY(model->setData(project12Index, "New Project 12"));
        QVERIFY(!model->setData(source2Index, "Foo"));
        QVERIFY(model->setData(project21Index, "New Project 21"));
        QVERIFY(model->setData(project22Index, "New Project 22"));
        QVERIFY(!model->setData(contextsIndex, "Foo"));
        QVERIFY(model->setData(context1Index, "New Context 1"));
        QVERIFY(model->setData(context2Index, "New Context 2"));

        // THEN
        QVERIFY(projectRepositoryMock(&Domain::ProjectRepository::update).when(project11).exactly(1));
        QVERIFY(projectRepositoryMock(&Domain::ProjectRepository::update).when(project12).exactly(1));
        QVERIFY(projectRepositoryMock(&Domain::ProjectRepository::update).when(project21).exactly(1));
        QVERIFY(projectRepositoryMock(&Domain::ProjectRepository::update).when(project22).exactly(1));
        QVERIFY(contextRepositoryMock(&Domain::ContextRepository::update).when(context1).exactly(1));
        QVERIFY(contextRepositoryMock(&Domain::ContextRepository::update).when(context2).exactly(1));

        QCOMPARE(project11->name(), QStringLiteral("New Project 11"));
        QCOMPARE(project12->name(), QStringLiteral("New Project 12"));
        QCOMPARE(project21->name(), QStringLiteral("New Project 21"));
        QCOMPARE(project22->name(), QStringLiteral("New Project 22"));
        QCOMPARE(context1->name(), QStringLiteral("New Context 1"));
        QCOMPARE(context2->name(), QStringLiteral("New Context 2"));

        // WHEN
        projectRepositoryMock(&Domain::ProjectRepository::associate).when(project11, taskToDrop).thenReturn(new FakeJob(this));
        auto data = std::make_unique<QMimeData>();
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Task::List() << taskToDrop));
        model->dropMimeData(data.get(), Qt::MoveAction, -1, -1, project11Index);

        // THEN
        QVERIFY(projectRepositoryMock(&Domain::ProjectRepository::associate).when(project11, taskToDrop).exactly(1));

        // WHEN a task is dropped on a context
        contextRepositoryMock(&Domain::ContextRepository::associate).when(context1, taskToDrop).thenReturn(new FakeJob(this));
        data.reset(new QMimeData);
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Task::List() << taskToDrop));
        model->dropMimeData(data.get(), Qt::MoveAction, -1, -1, context1Index);

        // THEN
        QVERIFY(contextRepositoryMock(&Domain::ContextRepository::associate).when(context1, taskToDrop).exactly(1));

        // WHEN
        projectRepositoryMock(&Domain::ProjectRepository::dissociate).when(taskToDrop).thenReturn(new FakeJob(this));
        taskRepositoryMock(&Domain::TaskRepository::dissociateAll).when(taskToDrop).thenReturn(new FakeJob(this));
        data.reset(new QMimeData);
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Task::List() << taskToDrop));
        model->dropMimeData(data.get(), Qt::MoveAction, -1, -1, inboxIndex);
        QTest::qWait(150);

        // THEN
        QVERIFY(projectRepositoryMock(&Domain::ProjectRepository::dissociate).when(taskToDrop).exactly(1));
        QVERIFY(taskRepositoryMock(&Domain::TaskRepository::dissociateAll).when(taskToDrop).exactly(1));

        // WHEN
        Domain::Task::Ptr taskToDrop2(new Domain::Task);
        projectRepositoryMock(&Domain::ProjectRepository::associate).when(project11, taskToDrop2).thenReturn(new FakeJob(this));
        data.reset(new QMimeData);
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Task::List() << taskToDrop2));
        model->dropMimeData(data.get(), Qt::MoveAction, -1, -1, project11Index);

        // THEN
        QVERIFY(projectRepositoryMock(&Domain::ProjectRepository::associate).when(project11, taskToDrop2).exactly(1));

        // WHEN two tasks are dropped on a context
        Domain::Task::Ptr taskToDrop3(new Domain::Task);
        Domain::Task::Ptr taskToDrop4(new Domain::Task);
        contextRepositoryMock(&Domain::ContextRepository::associate).when(context1, taskToDrop3).thenReturn(new FakeJob(this));
        contextRepositoryMock(&Domain::ContextRepository::associate).when(context1, taskToDrop4).thenReturn(new FakeJob(this));
        data.reset(new QMimeData);
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Task::List() << taskToDrop3 << taskToDrop4));
        model->dropMimeData(data.get(), Qt::MoveAction, -1, -1, context1Index);

        // THEN
        QVERIFY(contextRepositoryMock(&Domain::ContextRepository::associate).when(context1, taskToDrop3).exactly(1));
        QVERIFY(contextRepositoryMock(&Domain::ContextRepository::associate).when(context1, taskToDrop4).exactly(1));

        // WHEN a task is drop on the workday
        Domain::Task::Ptr taskToDrop5(new Domain::Task);
        taskRepositoryMock(&Domain::TaskRepository::update).when(taskToDrop5).thenReturn(new FakeJob(this));
        data.reset(new QMimeData);
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Task::List() << taskToDrop5));
        model->dropMimeData(data.get(), Qt::MoveAction, -1, -1, workdayIndex);

        // THEN
        QCOMPARE(taskToDrop5->startDate(), Utils::DateTime::currentDate());

        // WHEN two task are drop on the workday
        Domain::Task::Ptr taskToDrop6(new Domain::Task);
        Domain::Task::Ptr taskToDrop7(new Domain::Task);
        taskRepositoryMock(&Domain::TaskRepository::update).when(taskToDrop6).thenReturn(new FakeJob(this));
        taskRepositoryMock(&Domain::TaskRepository::update).when(taskToDrop7).thenReturn(new FakeJob(this));
        data.reset(new QMimeData);
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Task::List() << taskToDrop6 << taskToDrop7));
        model->dropMimeData(data.get(), Qt::MoveAction, -1, -1, workdayIndex);

        // THEN
        QCOMPARE(taskToDrop6->startDate(), Utils::DateTime::currentDate());
        QCOMPARE(taskToDrop7->startDate(), Utils::DateTime::currentDate());
    }



    void shouldCreateInboxPage()
    {
        // GIVEN

        // Empty sources provider
        auto sourceProvider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(sourceProvider);
        // Empty context provider
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);

        // context mocking
        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        // sources mocking
        Utils::MockObject<Domain::DataSourceQueries> dataSourceQueriesMock;
        dataSourceQueriesMock(&Domain::DataSourceQueries::findAllSelected).when().thenReturn(sourceResult);

        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;

        Presentation::AvailablePagesModel pages(dataSourceQueriesMock.getInstance(),
                                                    Domain::ProjectQueries::Ptr(),
                                                    projectRepositoryMock.getInstance(),
                                                    contextQueriesMock.getInstance(),
                                                    contextRepositoryMock.getInstance(),
                                                    Domain::TaskQueries::Ptr(),
                                                    Domain::TaskRepository::Ptr());

        // WHEN
        QAbstractItemModel *model = pages.pageListModel();

        // THEN
        const QModelIndex inboxIndex = model->index(0, 0);

        QObject *inboxPage = pages.createPageForIndex(inboxIndex);
        QVERIFY(qobject_cast<Presentation::InboxPageModel*>(inboxPage));
    }

    void shouldCreateWorkdayPage()
    {
        // GIVEN

        // Empty sources provider
        auto sourceProvider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(sourceProvider);
        // Empty context provider
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);

        // context mocking
        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        // sources mocking
        Utils::MockObject<Domain::DataSourceQueries> dataSourceQueriesMock;
        dataSourceQueriesMock(&Domain::DataSourceQueries::findAllSelected).when().thenReturn(sourceResult);

        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;

        Presentation::AvailablePagesModel pages(dataSourceQueriesMock.getInstance(),
                                                    Domain::ProjectQueries::Ptr(),
                                                    projectRepositoryMock.getInstance(),
                                                    contextQueriesMock.getInstance(),
                                                    contextRepositoryMock.getInstance(),
                                                    Domain::TaskQueries::Ptr(),
                                                    Domain::TaskRepository::Ptr());

        // WHEN
        QAbstractItemModel *model = pages.pageListModel();

        // THEN
        const QModelIndex workdayIndex = model->index(1, 0);

        QObject *workdayPage = pages.createPageForIndex(workdayIndex);
        QVERIFY(qobject_cast<Presentation::WorkdayPageModel*>(workdayPage));
    }

    void shouldCreateAllTasksPage()
    {
        // GIVEN

        // Empty sources provider
        auto sourceProvider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(sourceProvider);
        // Empty context provider
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);

        // context mocking
        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        // sources mocking
        Utils::MockObject<Domain::DataSourceQueries> dataSourceQueriesMock;
        dataSourceQueriesMock(&Domain::DataSourceQueries::findAllSelected).when().thenReturn(sourceResult);

        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;

        Presentation::AvailablePagesModel pages(dataSourceQueriesMock.getInstance(),
                                                    Domain::ProjectQueries::Ptr(),
                                                    projectRepositoryMock.getInstance(),
                                                    contextQueriesMock.getInstance(),
                                                    contextRepositoryMock.getInstance(),
                                                    Domain::TaskQueries::Ptr(),
                                                    Domain::TaskRepository::Ptr());

        // WHEN
        QAbstractItemModel *model = pages.pageListModel();

        // THEN
        const QModelIndex allTasksIndex = model->index(s_allTasksRow, 0);

        QObject *workdayPage = pages.createPageForIndex(allTasksIndex);
        QVERIFY(qobject_cast<Presentation::AllTasksPageModel*>(workdayPage));
    }

    void shouldCreateProjectsPage()
    {
        // GIVEN

        // One selected data source
        auto source = Domain::DataSource::Ptr::create();
        source->setName("source");
        auto sourceProvider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(sourceProvider);
        sourceProvider->append(source);

        // Two projects
        auto project1 = Domain::Project::Ptr::create();
        project1->setName(QStringLiteral("Project 11"));
        auto project2 = Domain::Project::Ptr::create();
        project2->setName(QStringLiteral("Project 12"));
        auto projectProvider = Domain::QueryResultProvider<Domain::Project::Ptr>::Ptr::create();
        auto projectResult = Domain::QueryResult<Domain::Project::Ptr>::create(projectProvider);
        projectProvider->append(project1);
        projectProvider->append(project2);

        // No contexts
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);

        // data source mocking
        Utils::MockObject<Domain::DataSourceQueries> dataSourceQueriesMock;
        dataSourceQueriesMock(&Domain::DataSourceQueries::findAllSelected).when().thenReturn(sourceResult);
        dataSourceQueriesMock(&Domain::DataSourceQueries::findProjects).when(source).thenReturn(projectResult);

        // projects mocking
        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;

        // contexts mocking
        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Presentation::AvailablePagesModel pages(dataSourceQueriesMock.getInstance(),
                                                    Domain::ProjectQueries::Ptr(),
                                                    projectRepositoryMock.getInstance(),
                                                    contextQueriesMock.getInstance(),
                                                    Domain::ContextRepository::Ptr(),
                                                    Domain::TaskQueries::Ptr(),
                                                    Domain::TaskRepository::Ptr());

        // WHEN
        QAbstractItemModel *model = pages.pageListModel();

        // THEN
        const QModelIndex projectsIndex = model->index(s_projectsRow, 0);
        const QModelIndex sourceIndex = model->index(0, 0, projectsIndex);
        const QModelIndex project1Index = model->index(0, 0, sourceIndex);
        const QModelIndex project2Index = model->index(1, 0, sourceIndex);

        QObject *projectsPage = pages.createPageForIndex(projectsIndex);
        QObject *sourcesPage = pages.createPageForIndex(sourceIndex);
        QObject *project1Page = pages.createPageForIndex(project1Index);
        QObject *project2Page = pages.createPageForIndex(project2Index);

        QVERIFY(!projectsPage);
        QVERIFY(!sourcesPage);
        QVERIFY(qobject_cast<Presentation::ProjectPageModel*>(project1Page));
        QCOMPARE(qobject_cast<Presentation::ProjectPageModel*>(project1Page)->project(), project1);
        QVERIFY(qobject_cast<Presentation::ProjectPageModel*>(project2Page));
        QCOMPARE(qobject_cast<Presentation::ProjectPageModel*>(project2Page)->project(), project2);
    }

    void shouldCreateContextsPage()
    {
        // GIVEN

        // Two contexts
        auto context1 = Domain::Context::Ptr::create();
        context1->setName(QStringLiteral("context 1"));
        auto context2 = Domain::Context::Ptr::create();
        context2->setName(QStringLiteral("context 2"));
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);
        contextProvider->append(context1);
        contextProvider->append(context2);

        // Empty sources provider
        auto sourceProvider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(sourceProvider);

        // contexts mocking
        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        // sources mocking
        Utils::MockObject<Domain::DataSourceQueries> dataSourceQueriesMock;
        dataSourceQueriesMock(&Domain::DataSourceQueries::findAllSelected).when().thenReturn(sourceResult);

        // projects mocking
        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;


        Presentation::AvailablePagesModel pages(dataSourceQueriesMock.getInstance(),
                                                    Domain::ProjectQueries::Ptr(),
                                                    projectRepositoryMock.getInstance(),
                                                    contextQueriesMock.getInstance(),
                                                    contextRepositoryMock.getInstance(),
                                                    Domain::TaskQueries::Ptr(),
                                                    Domain::TaskRepository::Ptr());

        // WHEN
        QAbstractItemModel *model = pages.pageListModel();

        // THEN
        const QModelIndex contextsIndex = model->index(s_contextRow, 0);
        const QModelIndex context1Index = model->index(0, 0, contextsIndex);
        const QModelIndex context2Index = model->index(1, 0, contextsIndex);

        QObject *contextsPage = pages.createPageForIndex(contextsIndex);
        QObject *context1Page = pages.createPageForIndex(context1Index);
        QObject *context2Page = pages.createPageForIndex(context2Index);

        QVERIFY(!contextsPage);
        QVERIFY(qobject_cast<Presentation::ContextPageModel*>(context1Page));
        QCOMPARE(qobject_cast<Presentation::ContextPageModel*>(context1Page)->context(), context1);
        QVERIFY(qobject_cast<Presentation::ContextPageModel*>(context2Page));
        QCOMPARE(qobject_cast<Presentation::ContextPageModel*>(context2Page)->context(), context2);
    }

    void shouldAddProjects()
    {
        // GIVEN

        auto source = Domain::DataSource::Ptr::create();

        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;
        projectRepositoryMock(&Domain::ProjectRepository::create).when(any<Domain::Project::Ptr>(),
                                                                       any<Domain::DataSource::Ptr>())
                                                                 .thenReturn(new FakeJob(this));

        Presentation::AvailablePagesModel pages(Domain::DataSourceQueries::Ptr(),
                                                    Domain::ProjectQueries::Ptr(),
                                                    projectRepositoryMock.getInstance(),
                                                    Domain::ContextQueries::Ptr(),
                                                    Domain::ContextRepository::Ptr(),
                                                    Domain::TaskQueries::Ptr(),
                                                    Domain::TaskRepository::Ptr());

        // WHEN
        pages.addProject(QStringLiteral("Foo"), source);

        // THEN
        QVERIFY(projectRepositoryMock(&Domain::ProjectRepository::create).when(any<Domain::Project::Ptr>(),
                                                                               any<Domain::DataSource::Ptr>())
                                                                         .exactly(1));
    }

    void shouldGetAnErrorMessageWhenAddProjectFailed()
    {
        // GIVEN

        auto source = Domain::DataSource::Ptr::create();
        source->setName(QStringLiteral("Source1"));

        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
        projectRepositoryMock(&Domain::ProjectRepository::create).when(any<Domain::Project::Ptr>(),
                                                                       any<Domain::DataSource::Ptr>())
                                                                 .thenReturn(job);

        Presentation::AvailablePagesModel pages(Domain::DataSourceQueries::Ptr(),
                                                    Domain::ProjectQueries::Ptr(),
                                                    projectRepositoryMock.getInstance(),
                                                    Domain::ContextQueries::Ptr(),
                                                    Domain::ContextRepository::Ptr(),
                                                    Domain::TaskQueries::Ptr(),
                                                    Domain::TaskRepository::Ptr());
        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);

        // WHEN
        pages.addProject(QStringLiteral("Foo"), source);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot add project Foo in dataSource Source1: Foo"));
    }

    void shouldAddContexts()
    {
        // GIVEN
        auto source = Domain::DataSource::Ptr::create();
        source->setName(QStringLiteral("Source1"));

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;
        contextRepositoryMock(&Domain::ContextRepository::create).when(any<Domain::Context::Ptr>(), any<Domain::DataSource::Ptr>())
                                                                 .thenReturn(new FakeJob(this));

        Presentation::AvailablePagesModel pages(Domain::DataSourceQueries::Ptr(),
                                                    Domain::ProjectQueries::Ptr(),
                                                    Domain::ProjectRepository::Ptr(),
                                                    Domain::ContextQueries::Ptr(),
                                                    contextRepositoryMock.getInstance(),
                                                    Domain::TaskQueries::Ptr(),
                                                    Domain::TaskRepository::Ptr());

        // WHEN
        pages.addContext(QStringLiteral("Foo"), source);

        // THEN
        QVERIFY(contextRepositoryMock(&Domain::ContextRepository::create).when(any<Domain::Context::Ptr>(), any<Domain::DataSource::Ptr>())
                                                                       .exactly(1));
    }

    void shouldGetAnErrorMessageWhenAddContextFailed()
    {
        // GIVEN
        auto source = Domain::DataSource::Ptr::create();
        source->setName(QStringLiteral("Source1"));

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
        contextRepositoryMock(&Domain::ContextRepository::create).when(any<Domain::Context::Ptr>(), any<Domain::DataSource::Ptr>())
                                                                 .thenReturn(job);

        Presentation::AvailablePagesModel pages(Domain::DataSourceQueries::Ptr(),
                                                    Domain::ProjectQueries::Ptr(),
                                                    Domain::ProjectRepository::Ptr(),
                                                    Domain::ContextQueries::Ptr(),
                                                    contextRepositoryMock.getInstance(),
                                                    Domain::TaskQueries::Ptr(),
                                                    Domain::TaskRepository::Ptr());
        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);

        // WHEN
        pages.addContext(QStringLiteral("Foo"), source);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot add context Foo: Foo"));
    }

    void shouldRemoveProject()
    {
        // GIVEN

        // One selected data source
        auto source = Domain::DataSource::Ptr::create();
        source->setName("source");
        auto sourceProvider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(sourceProvider);
        sourceProvider->append(source);

        // Two projects
        auto project1 = Domain::Project::Ptr::create();
        project1->setName(QStringLiteral("Project 1"));
        auto project2 = Domain::Project::Ptr::create();
        project2->setName(QStringLiteral("Project 2"));
        auto projectProvider = Domain::QueryResultProvider<Domain::Project::Ptr>::Ptr::create();
        auto projectResult = Domain::QueryResult<Domain::Project::Ptr>::create(projectProvider);
        projectProvider->append(project1);
        projectProvider->append(project2);

        // No contexts
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);

        // data source mocking
        Utils::MockObject<Domain::DataSourceQueries> dataSourceQueriesMock;
        dataSourceQueriesMock(&Domain::DataSourceQueries::findAllSelected).when().thenReturn(sourceResult);
        dataSourceQueriesMock(&Domain::DataSourceQueries::findProjects).when(source).thenReturn(projectResult);

        // projects mocking
        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;

        // contexts mocking
        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Presentation::AvailablePagesModel pages(dataSourceQueriesMock.getInstance(),
                                                    Domain::ProjectQueries::Ptr(),
                                                    projectRepositoryMock.getInstance(),
                                                    contextQueriesMock.getInstance(),
                                                    Domain::ContextRepository::Ptr(),
                                                    Domain::TaskQueries::Ptr(),
                                                    Domain::TaskRepository::Ptr());

        QAbstractItemModel *model = pages.pageListModel();

        const QModelIndex projectsIndex = model->index(s_projectsRow, 0);
        const QModelIndex sourceIndex = model->index(0, 0, projectsIndex);
        const QModelIndex project1Index = model->index(0, 0, sourceIndex);

        projectRepositoryMock(&Domain::ProjectRepository::remove).when(project1).thenReturn(new FakeJob(this));

        // WHEN
        pages.removeItem(project1Index);

        // THEN
        QVERIFY(projectRepositoryMock(&Domain::ProjectRepository::remove).when(project1).exactly(1));
    }

    void shouldGetAnErrorMessageWhenRemoveProjectFailed()
    {
        // GIVEN

        // One selected data source
        auto source = Domain::DataSource::Ptr::create();
        source->setName("source");
        auto sourceProvider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(sourceProvider);
        sourceProvider->append(source);

        // Two projects
        auto project1 = Domain::Project::Ptr::create();
        project1->setName(QStringLiteral("Project 1"));
        auto project2 = Domain::Project::Ptr::create();
        project2->setName(QStringLiteral("Project 2"));
        auto projectProvider = Domain::QueryResultProvider<Domain::Project::Ptr>::Ptr::create();
        auto projectResult = Domain::QueryResult<Domain::Project::Ptr>::create(projectProvider);
        projectProvider->append(project1);
        projectProvider->append(project2);

        // No contexts
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);

        // data source mocking
        Utils::MockObject<Domain::DataSourceQueries> dataSourceQueriesMock;
        dataSourceQueriesMock(&Domain::DataSourceQueries::findAllSelected).when().thenReturn(sourceResult);
        dataSourceQueriesMock(&Domain::DataSourceQueries::findProjects).when(source).thenReturn(projectResult);

        // projects mocking
        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;

        // contexts mocking
        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Presentation::AvailablePagesModel pages(dataSourceQueriesMock.getInstance(),
                                                    Domain::ProjectQueries::Ptr(),
                                                    projectRepositoryMock.getInstance(),
                                                    contextQueriesMock.getInstance(),
                                                    Domain::ContextRepository::Ptr(),
                                                    Domain::TaskQueries::Ptr(),
                                                    Domain::TaskRepository::Ptr());
        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);

        QAbstractItemModel *model = pages.pageListModel();

        const QModelIndex projectsIndex = model->index(s_projectsRow, 0);
        const QModelIndex sourceIndex = model->index(0, 0, projectsIndex);
        const QModelIndex project1Index = model->index(0, 0, sourceIndex);

        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
        projectRepositoryMock(&Domain::ProjectRepository::remove).when(project1).thenReturn(job);

        // WHEN
        pages.removeItem(project1Index);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot remove project Project 1: Foo"));
    }

    void shouldRemoveContext()
    {
        // GIVEN

        // Two contexts
        auto context1 = Domain::Context::Ptr::create();
        context1->setName(QStringLiteral("context 1"));
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);
        contextProvider->append(context1);

        // Empty sources provider
        auto sourceProvider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(sourceProvider);

        // contexts mocking
        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        // sources mocking
        Utils::MockObject<Domain::DataSourceQueries> dataSourceQueriesMock;
        dataSourceQueriesMock(&Domain::DataSourceQueries::findAllSelected).when().thenReturn(sourceResult);


        Presentation::AvailablePagesModel pages(dataSourceQueriesMock.getInstance(),
                                                    Domain::ProjectQueries::Ptr(),
                                                    Domain::ProjectRepository::Ptr(),
                                                    contextQueriesMock.getInstance(),
                                                    contextRepositoryMock.getInstance(),
                                                    Domain::TaskQueries::Ptr(),
                                                    Domain::TaskRepository::Ptr());

        QAbstractItemModel *model = pages.pageListModel();

        const QModelIndex contextsIndex = model->index(s_contextRow, 0);
        const QModelIndex context1Index = model->index(0, 0, contextsIndex);

        contextRepositoryMock(&Domain::ContextRepository::remove).when(context1).thenReturn(new FakeJob(this));

        // WHEN
        pages.removeItem(context1Index);

        // THEN
        QVERIFY(contextRepositoryMock(&Domain::ContextRepository::remove).when(context1).exactly(1));
    }

    void shouldGetAnErrorMessageWhenRemoveContextFailed()
    {
        // GIVEN

        // Two contexts
        auto context1 = Domain::Context::Ptr::create();
        context1->setName(QStringLiteral("context 1"));
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);
        contextProvider->append(context1);

        // Empty sources provider
        auto sourceProvider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(sourceProvider);

        // contexts mocking
        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        // sources mocking
        Utils::MockObject<Domain::DataSourceQueries> dataSourceQueriesMock;
        dataSourceQueriesMock(&Domain::DataSourceQueries::findAllSelected).when().thenReturn(sourceResult);


        Presentation::AvailablePagesModel pages(dataSourceQueriesMock.getInstance(),
                                                    Domain::ProjectQueries::Ptr(),
                                                    Domain::ProjectRepository::Ptr(),
                                                    contextQueriesMock.getInstance(),
                                                    contextRepositoryMock.getInstance(),
                                                    Domain::TaskQueries::Ptr(),
                                                    Domain::TaskRepository::Ptr());
        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);

        QAbstractItemModel *model = pages.pageListModel();

        const QModelIndex contextsIndex = model->index(s_contextRow, 0);
        const QModelIndex context1Index = model->index(0, 0, contextsIndex);

        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
        contextRepositoryMock(&Domain::ContextRepository::remove).when(context1).thenReturn(job);

        // WHEN
        pages.removeItem(context1Index);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot remove context context 1: Foo"));
    }

    void shouldGetAnErrorMessageWhenUpdateProjectFailed()
    {
        // GIVEN

        // One selected data source
        auto source = Domain::DataSource::Ptr::create();
        source->setName("source1");
        auto sourceProvider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(sourceProvider);
        sourceProvider->append(source);

        // Two projects under the source
        auto project1 = Domain::Project::Ptr::create();
        project1->setName(QStringLiteral("Project 1"));
        auto project2 = Domain::Project::Ptr::create();
        project2->setName(QStringLiteral("Project 2"));
        auto projectProvider = Domain::QueryResultProvider<Domain::Project::Ptr>::Ptr::create();
        auto projectResult = Domain::QueryResult<Domain::Project::Ptr>::create(projectProvider);
        projectProvider->append(project1);
        projectProvider->append(project2);

        // Two contexts
        auto context1 = Domain::Context::Ptr::create();
        context1->setName(QStringLiteral("context 1"));
        auto context2 = Domain::Context::Ptr::create();
        context2->setName(QStringLiteral("context 2"));
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);
        contextProvider->append(context1);
        contextProvider->append(context2);

        Utils::MockObject<Domain::DataSourceQueries> dataSourceQueriesMock;
        dataSourceQueriesMock(&Domain::DataSourceQueries::findAllSelected).when().thenReturn(sourceResult);
        dataSourceQueriesMock(&Domain::DataSourceQueries::findProjects).when(source).thenReturn(projectResult);

        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        Presentation::AvailablePagesModel pages(dataSourceQueriesMock.getInstance(),
                                                    Domain::ProjectQueries::Ptr(),
                                                    projectRepositoryMock.getInstance(),
                                                    contextQueriesMock.getInstance(),
                                                    contextRepositoryMock.getInstance(),
                                                    Domain::TaskQueries::Ptr(),
                                                    Domain::TaskRepository::Ptr());

        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);
        QAbstractItemModel *model = pages.pageListModel();
        const QModelIndex projectsIndex = model->index(s_projectsRow, 0);
        const QModelIndex sourceIndex = model->index(0, 0, projectsIndex);
        const QModelIndex project1Index = model->index(0, 0, sourceIndex);

        // WHEN
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
        projectRepositoryMock(&Domain::ProjectRepository::update).when(project1).thenReturn(job);

        QVERIFY(model->setData(project1Index, "New Project 1"));

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot modify project Project 1: Foo"));
    }

    void shouldGetAnErrorMessageWhenUpdateContextFailed()
    {
        // GIVEN

        // One selected data source
        auto source = Domain::DataSource::Ptr::create();
        source->setName("source1");
        auto sourceProvider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(sourceProvider);
        sourceProvider->append(source);

        // Two projects under the source
        auto project1 = Domain::Project::Ptr::create();
        project1->setName(QStringLiteral("Project 1"));
        auto project2 = Domain::Project::Ptr::create();
        project2->setName(QStringLiteral("Project 2"));
        auto projectProvider = Domain::QueryResultProvider<Domain::Project::Ptr>::Ptr::create();
        auto projectResult = Domain::QueryResult<Domain::Project::Ptr>::create(projectProvider);
        projectProvider->append(project1);
        projectProvider->append(project2);

        // Two contexts
        auto context1 = Domain::Context::Ptr::create();
        context1->setName(QStringLiteral("context 1"));
        auto context2 = Domain::Context::Ptr::create();
        context2->setName(QStringLiteral("context 2"));
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);
        contextProvider->append(context1);
        contextProvider->append(context2);

        Utils::MockObject<Domain::DataSourceQueries> dataSourceQueriesMock;
        dataSourceQueriesMock(&Domain::DataSourceQueries::findAllSelected).when().thenReturn(sourceResult);
        dataSourceQueriesMock(&Domain::DataSourceQueries::findProjects).when(source).thenReturn(projectResult);

        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        Presentation::AvailablePagesModel pages(dataSourceQueriesMock.getInstance(),
                                                    Domain::ProjectQueries::Ptr(),
                                                    projectRepositoryMock.getInstance(),
                                                    contextQueriesMock.getInstance(),
                                                    contextRepositoryMock.getInstance(),
                                                    Domain::TaskQueries::Ptr(),
                                                    Domain::TaskRepository::Ptr());

        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);
        QAbstractItemModel *model = pages.pageListModel();
        const QModelIndex contextsIndex = model->index(s_contextRow, 0);
        const QModelIndex context1Index = model->index(0, 0, contextsIndex);

        // WHEN
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
        contextRepositoryMock(&Domain::ContextRepository::update).when(context1).thenReturn(job);

        QVERIFY(model->setData(context1Index, "New Context 1"));

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot modify context context 1: Foo"));
    }

    void shouldGetAnErrorMessageWhenAssociateProjectFailed()
    {
        // GIVEN

        // One selected data source
        auto source = Domain::DataSource::Ptr::create();
        source->setName("source1");
        auto sourceProvider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(sourceProvider);
        sourceProvider->append(source);

        // Two projects under the source
        auto project1 = Domain::Project::Ptr::create();
        project1->setName(QStringLiteral("Project 1"));
        auto project2 = Domain::Project::Ptr::create();
        project2->setName(QStringLiteral("Project 2"));
        auto projectProvider = Domain::QueryResultProvider<Domain::Project::Ptr>::Ptr::create();
        auto projectResult = Domain::QueryResult<Domain::Project::Ptr>::create(projectProvider);
        projectProvider->append(project1);
        projectProvider->append(project2);

        // Two contexts
        auto context1 = Domain::Context::Ptr::create();
        context1->setName(QStringLiteral("context 1"));
        auto context2 = Domain::Context::Ptr::create();
        context2->setName(QStringLiteral("context 2"));
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);
        contextProvider->append(context1);
        contextProvider->append(context2);

        // One task (used for dropping later on)
        Domain::Task::Ptr taskToDrop(new Domain::Task);
        taskToDrop->setTitle(QStringLiteral("taskDropped"));

        Utils::MockObject<Domain::DataSourceQueries> dataSourceQueriesMock;
        dataSourceQueriesMock(&Domain::DataSourceQueries::findAllSelected).when().thenReturn(sourceResult);
        dataSourceQueriesMock(&Domain::DataSourceQueries::findProjects).when(source).thenReturn(projectResult);

        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;

        Presentation::AvailablePagesModel pages(dataSourceQueriesMock.getInstance(),
                                                    Domain::ProjectQueries::Ptr(),
                                                    projectRepositoryMock.getInstance(),
                                                    contextQueriesMock.getInstance(),
                                                    contextRepositoryMock.getInstance(),
                                                    Domain::TaskQueries::Ptr(),
                                                    taskRepositoryMock.getInstance());

        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);
        QAbstractItemModel *model = pages.pageListModel();
        const QModelIndex projectsIndex = model->index(s_projectsRow, 0);
        const QModelIndex sourceIndex = model->index(0, 0, projectsIndex);
        const QModelIndex project1Index = model->index(0, 0, sourceIndex);

        // WHEN
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
        projectRepositoryMock(&Domain::ProjectRepository::associate).when(project1, taskToDrop).thenReturn(job);
        auto data = std::make_unique<QMimeData>();
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Task::List() << taskToDrop));
        model->dropMimeData(data.get(), Qt::MoveAction, -1, -1, project1Index);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot add taskDropped to project Project 1: Foo"));
    }

    void shouldGetAnErrorMessageWhenAssociateContextFailed()
    {
        // GIVEN

        // One selected data source
        auto source = Domain::DataSource::Ptr::create();
        source->setName("source1");
        auto sourceProvider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(sourceProvider);
        sourceProvider->append(source);

        // Two projects under the source
        auto project1 = Domain::Project::Ptr::create();
        project1->setName(QStringLiteral("Project 1"));
        auto project2 = Domain::Project::Ptr::create();
        project2->setName(QStringLiteral("Project 2"));
        auto projectProvider = Domain::QueryResultProvider<Domain::Project::Ptr>::Ptr::create();
        auto projectResult = Domain::QueryResult<Domain::Project::Ptr>::create(projectProvider);
        projectProvider->append(project1);
        projectProvider->append(project2);

        // Two contexts
        auto context1 = Domain::Context::Ptr::create();
        context1->setName(QStringLiteral("context 1"));
        auto context2 = Domain::Context::Ptr::create();
        context2->setName(QStringLiteral("context 2"));
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);
        contextProvider->append(context1);
        contextProvider->append(context2);

        // One task (used for dropping later on)
        Domain::Task::Ptr taskToDrop(new Domain::Task);
        taskToDrop->setTitle(QStringLiteral("taskDropped"));

        Utils::MockObject<Domain::DataSourceQueries> dataSourceQueriesMock;
        dataSourceQueriesMock(&Domain::DataSourceQueries::findAllSelected).when().thenReturn(sourceResult);
        dataSourceQueriesMock(&Domain::DataSourceQueries::findProjects).when(source).thenReturn(projectResult);

        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;

        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;

        Presentation::AvailablePagesModel pages(dataSourceQueriesMock.getInstance(),
                                                    Domain::ProjectQueries::Ptr(),
                                                    projectRepositoryMock.getInstance(),
                                                    contextQueriesMock.getInstance(),
                                                    contextRepositoryMock.getInstance(),
                                                    Domain::TaskQueries::Ptr(),
                                                    taskRepositoryMock.getInstance());

        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);
        QAbstractItemModel *model = pages.pageListModel();
        const QModelIndex contextsIndex = model->index(s_contextRow, 0);
        const QModelIndex context1Index = model->index(0, 0, contextsIndex);

        // WHEN
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
        contextRepositoryMock(&Domain::ContextRepository::associate).when(context1, taskToDrop).thenReturn(job);
        auto data = std::make_unique<QMimeData>();
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Task::List() << taskToDrop));
        model->dropMimeData(data.get(), Qt::MoveAction, -1, -1, context1Index);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot add taskDropped to context context 1: Foo"));
    }

    void shouldGetAnErrorMessageWhenDissociateFailed()
    {
        // GIVEN

        // Empty source provider
        auto sourceProvider = Domain::QueryResultProvider<Domain::DataSource::Ptr>::Ptr::create();
        auto sourceResult = Domain::QueryResult<Domain::DataSource::Ptr>::create(sourceProvider);

        // Empty context provider
        auto contextProvider = Domain::QueryResultProvider<Domain::Context::Ptr>::Ptr::create();
        auto contextResult = Domain::QueryResult<Domain::Context::Ptr>::create(contextProvider);

        // One task (used for dropping later on)
        Domain::Task::Ptr taskToDrop(new Domain::Task);
        taskToDrop->setTitle(QStringLiteral("taskDropped"));

        // context mocking
        Utils::MockObject<Domain::ContextQueries> contextQueriesMock;
        contextQueriesMock(&Domain::ContextQueries::findAll).when().thenReturn(contextResult);

        Utils::MockObject<Domain::ContextRepository> contextRepositoryMock;

        // sources mocking
        Utils::MockObject<Domain::DataSourceQueries> dataSourceQueriesMock;
        dataSourceQueriesMock(&Domain::DataSourceQueries::findAllSelected).when().thenReturn(sourceResult);

        Utils::MockObject<Domain::ProjectRepository> projectRepositoryMock;
        Utils::MockObject<Domain::TaskRepository> taskRepositoryMock;

        Presentation::AvailablePagesModel pages(dataSourceQueriesMock.getInstance(),
                                                    Domain::ProjectQueries::Ptr(),
                                                    projectRepositoryMock.getInstance(),
                                                    contextQueriesMock.getInstance(),
                                                    contextRepositoryMock.getInstance(),
                                                    Domain::TaskQueries::Ptr(),
                                                    taskRepositoryMock.getInstance());

        FakeErrorHandler errorHandler;
        pages.setErrorHandler(&errorHandler);
        QAbstractItemModel *model = pages.pageListModel();
        const QModelIndex inboxIndex = model->index(0, 0);

        // WHEN
        auto job = new FakeJob(this);
        job->setExpectedError(KJob::KilledJobError, QStringLiteral("Foo"));
        projectRepositoryMock(&Domain::ProjectRepository::dissociate).when(taskToDrop).thenReturn(job);
        taskRepositoryMock(&Domain::TaskRepository::dissociateAll).when(taskToDrop).thenReturn(new FakeJob(this));
        auto data = std::make_unique<QMimeData>();
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(Domain::Task::List() << taskToDrop));
        model->dropMimeData(data.get(), Qt::MoveAction, -1, -1, inboxIndex);

        // THEN
        QTest::qWait(150);
        QCOMPARE(errorHandler.m_message, QStringLiteral("Cannot move taskDropped to Inbox: Foo"));
    }
};

ZANSHIN_TEST_MAIN(AvailablePagesModelTest)

#include "availablepagesmodeltest.moc"
