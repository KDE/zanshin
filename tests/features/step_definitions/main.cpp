#include <boost/test/unit_test.hpp>
#include <cucumber-cpp/defs.hpp>

#include <QApplication>
#include <QDebug>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTest>

#include <KConfig>
#include <KConfigGroup>
#include <KGlobal>

#include "akonadi/akonadiartifactqueries.h"
#include "akonadi/akonadidatasourcequeries.h"
#include "akonadi/akonadinoterepository.h"
#include "akonadi/akonaditaskqueries.h"
#include "akonadi/akonaditaskrepository.h"
#include "presentation/inboxpagemodel.h"
#include "presentation/tasklistmodel.h"
#include "presentation/datasourcelistmodel.h"

static int argc = 0;
static QApplication app(argc, 0);

namespace cucumber {
    namespace internal {
        template<>
        inline QString fromString(const std::string& s) {
            return QString::fromUtf8(s.data());
        }
    }
}

using namespace cucumber;


QString indexToTitle(const QModelIndex &index)
{
    return index.data().toString();
}

bool indexToDone(const QModelIndex &index)
{
    return index.data(Qt::CheckStateRole).toBool();
}

class ZanshinContext
{
private:
    ZanshinContext(const ZanshinContext &);
public:
    ZanshinContext()
        : presentation(0),
          proxyModel(new QSortFilterProxyModel),
          artifactQueries(new Akonadi::ArtifactQueries),
          dataSourceQueries(new Akonadi::DataSourceQueries),
          queries(new Akonadi::TaskQueries()),
          taskRepository(new Akonadi::TaskRepository()),
          noteRepository(new Akonadi::NoteRepository())
    {
        proxyModel->setDynamicSortFilter(true);
    }

    ~ZanshinContext()
    {
        delete proxyModel;
        delete artifactQueries;
        delete dataSourceQueries;
        delete queries;
        delete taskRepository;
        delete presentation;
    }

    void setModel(QAbstractItemModel *model)
    {
        proxyModel->setSourceModel(model);
        proxyModel->setSortRole(Qt::DisplayRole);
        proxyModel->sort(0);
    }

    QAbstractItemModel *model()
    {
        return proxyModel;
    }

    Domain::ArtifactQueries *artifactQueries;
    Domain::DataSourceQueries *dataSourceQueries;
    Domain::TaskQueries *queries;
    Domain::TaskRepository *taskRepository;
    Domain::NoteRepository *noteRepository;
    QList<QPersistentModelIndex> indices;
    QObject *presentation;

private:
    QSortFilterProxyModel *proxyModel;
};

GIVEN("^I got a task list$") {
    ScenarioScope<ZanshinContext> context;
    auto queries = context->queries->findAll();
    context->setModel(new Presentation::TaskListModel(queries, context->taskRepository));
    QTest::qWait(500);
}

GIVEN("^I got a task data source list model$") {
    ScenarioScope<ZanshinContext> context;
    auto queries = context->dataSourceQueries->findTasks();
    context->setModel(new Presentation::DataSourceListModel(queries));
    QTest::qWait(500);
}

GIVEN("^I got a note data source list model$") {
    ScenarioScope<ZanshinContext> context;
    auto queries = context->dataSourceQueries->findNotes();
    context->setModel(new Presentation::DataSourceListModel(queries));
    QTest::qWait(500);
}

GIVEN("^I'm looking at the inbox view$") {
    ScenarioScope<ZanshinContext> context;
    context->presentation = new Presentation::InboxPageModel(context->artifactQueries,
                                                         context->dataSourceQueries,
                                                         context->queries,
                                                         context->taskRepository,
                                                         context->noteRepository);
    QTest::qWait(500);
}


WHEN("^I look at the central list$") {
    ScenarioScope<ZanshinContext> context;

    auto model = context->presentation->property("centralListModel").value<QAbstractItemModel*>();
    QTest::qWait(500);
    context->setModel(model);

    for (int row = 0; row < context->model()->rowCount(); row++) {
        context->indices << context->model()->index(row, 0);
    }
}

WHEN("^I list the model$") {
    ScenarioScope<ZanshinContext> context;
    for (int row = 0; row < context->model()->rowCount(); row++) {
        context->indices << context->model()->index(row, 0);
    }
}

WHEN("^the setting key (\\S+) changes to (\\d+)$") {
    REGEX_PARAM(QString, keyName);
    REGEX_PARAM(qint64, id);

    KConfigGroup config(KGlobal::config(), "General");
    config.writeEntry(keyName, id);
}

WHEN("^the user changes the default (\\S+) data source to (.*)$") {
    REGEX_PARAM(QString, sourceType);
    REGEX_PARAM(QString, sourceName);

    ScenarioScope<ZanshinContext> context;
    auto sourcesResult = sourceType == "task" ? context->dataSourceQueries->findTasks()
                       : sourceType == "note" ? context->dataSourceQueries->findNotes()
                       : Domain::QueryResult<Domain::DataSource::Ptr>::Ptr();
    QTest::qWait(500);
    auto sources = sourcesResult->data();
    auto source = *std::find_if(sources.begin(), sources.end(),
                                [=] (const Domain::DataSource::Ptr &source) {
                                    return source->name() == sourceName;
                                });


    auto propertyName = sourceType == "task" ? "defaultTaskDataSource"
                      : sourceType == "note" ? "defaultNoteDataSource"
                      : 0;
    context->presentation->setProperty(propertyName, QVariant::fromValue(source));
}


THEN("^the list is") {
    TABLE_PARAM(tableParam);

    ScenarioScope<ZanshinContext> context;
    auto roleNames = context->model()->roleNames();
    QSet<int> usedRoles;

    QStandardItemModel referenceModel;
    for (const auto row : tableParam.hashes()) {
        QStandardItem *item = new QStandardItem;
        for (const auto it : row) {
            const QByteArray roleName = it.first.data();
            const QString value = QString::fromUtf8(it.second.data());
            const int role = roleNames.key(roleName, -1);
            BOOST_REQUIRE(role != -1);
            item->setData(value, role);
            usedRoles.insert(role);
        }
        referenceModel.appendRow(item);
    }

    QSortFilterProxyModel proxy;
    proxy.setSourceModel(&referenceModel);
    proxy.setSortRole(Qt::DisplayRole);
    proxy.sort(0);

    for (int row = 0; row < context->indices.size(); row++) {
        QModelIndex expectedIndex = proxy.index(row, 0);
        QModelIndex resultIndex = context->indices.at(row);

        for (auto role : usedRoles) {
            BOOST_REQUIRE(expectedIndex.data(role).toString() == resultIndex.data(role).toString());
        }
    }
    BOOST_REQUIRE(proxy.rowCount() == context->indices.size());
}

THEN("^the default (\\S+) data source is (.*)$") {
    REGEX_PARAM(QString, sourceType);
    REGEX_PARAM(QString, expectedName);

    auto propertyName = sourceType == "task" ? "defaultTaskDataSource"
                      : sourceType == "note" ? "defaultNoteDataSource"
                      : 0;

    ScenarioScope<ZanshinContext> context;
    auto source = context->presentation->property(propertyName).value<Domain::DataSource::Ptr>();
    BOOST_REQUIRE(!source.isNull());
    BOOST_REQUIRE(source->name() == expectedName);
}

THEN("^the setting key (\\S+) is (\\d+)$") {
    REGEX_PARAM(QString, keyName);
    REGEX_PARAM(qint64, expectedId);

    KConfigGroup config(KGlobal::config(), "General");
    const qint64 id = config.readEntry(keyName, -1);
    BOOST_REQUIRE(id == expectedId);
}
