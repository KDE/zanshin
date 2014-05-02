#include <boost/test/unit_test.hpp>
#include <cucumber-cpp/defs.hpp>

#include <QApplication>
#include <QDebug>
#include <QTest>

#include "akonadi/akonadidatasourcequeries.h"
#include "akonadi/akonaditaskqueries.h"
#include "akonadi/akonaditaskrepository.h"
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
        : model(0),
          dataSourceQueries(new Akonadi::DataSourceQueries),
          queries(new Akonadi::TaskQueries()),
          repository(new Akonadi::TaskRepository())
    {
    }

    ~ZanshinContext()
    {
        delete model;
        delete dataSourceQueries;
        delete queries;
        delete repository;
    }

    QAbstractItemModel *model;
    Domain::DataSourceQueries *dataSourceQueries;
    Domain::TaskQueries *queries;
    Domain::TaskRepository *repository;
    QList<QPersistentModelIndex> indices;
    QStringList titles;
    QList<bool> doneStates;
};

GIVEN("^I got a task list$") {
    ScenarioScope<ZanshinContext> context;
    auto queries = context->queries->findAll();
    context->model = new Presentation::TaskListModel(queries, context->repository);
    QTest::qWait(500);
}

GIVEN("^I got a task data source list model$") {
    ScenarioScope<ZanshinContext> context;
    auto queries = context->dataSourceQueries->findTasks();
    context->model = new Presentation::DataSourceListModel(queries);
    QTest::qWait(500);
}

GIVEN("^I got a note data source list model$") {
    ScenarioScope<ZanshinContext> context;
    auto queries = context->dataSourceQueries->findNotes();
    context->model = new Presentation::DataSourceListModel(queries);
    QTest::qWait(500);
}


WHEN("^I list the model$") {
    ScenarioScope<ZanshinContext> context;
    for (int row = 0; row < context->model->rowCount(); row++) {
        context->indices << context->model->index(row, 0);
        context->titles << indexToTitle(context->indices.last());
        context->doneStates << indexToDone(context->indices.last());
    }
}


THEN("^the list is") {
    TABLE_PARAM(tableParam);

    QStringList list;
    for (const auto it : tableParam.hashes()) {
        list << cucumber::internal::fromString<QString>(std::string(it.at("display")));
    }
    list.sort();

    ScenarioScope<ZanshinContext> context;
    auto titles = context->titles;
    titles.sort();
    BOOST_CHECK(list == titles);
}
