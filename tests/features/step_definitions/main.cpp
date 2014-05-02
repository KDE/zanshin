#include <boost/test/unit_test.hpp>
#include <cucumber-cpp/defs.hpp>

#include <QApplication>
#include <QDebug>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
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
        : proxyModel(new QSortFilterProxyModel),
          dataSourceQueries(new Akonadi::DataSourceQueries),
          queries(new Akonadi::TaskQueries()),
          repository(new Akonadi::TaskRepository())
    {
        proxyModel->setDynamicSortFilter(true);
    }

    ~ZanshinContext()
    {
        delete proxyModel;
        delete dataSourceQueries;
        delete queries;
        delete repository;
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

    Domain::DataSourceQueries *dataSourceQueries;
    Domain::TaskQueries *queries;
    Domain::TaskRepository *repository;
    QList<QPersistentModelIndex> indices;

private:
    QSortFilterProxyModel *proxyModel;
};

GIVEN("^I got a task list$") {
    ScenarioScope<ZanshinContext> context;
    auto queries = context->queries->findAll();
    context->setModel(new Presentation::TaskListModel(queries, context->repository));
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


WHEN("^I list the model$") {
    ScenarioScope<ZanshinContext> context;
    for (int row = 0; row < context->model()->rowCount(); row++) {
        context->indices << context->model()->index(row, 0);
    }
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
