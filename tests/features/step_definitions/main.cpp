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
#include "akonadi/akonadicontextqueries.h"
#include "akonadi/akonadicontextrepository.h"
#include "akonadi/akonadidatasourcequeries.h"
#include "akonadi/akonadidatasourcerepository.h"
#include "akonadi/akonadinoterepository.h"
#include "akonadi/akonadiprojectqueries.h"
#include "akonadi/akonadiprojectrepository.h"
#include "akonadi/akonaditagqueries.h"
#include "akonadi/akonaditaskqueries.h"
#include "akonadi/akonaditaskrepository.h"
#include "presentation/applicationmodel.h"
#include "presentation/inboxpagemodel.h"
#include "presentation/querytreemodelbase.h"
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

class ZanshinContext : public QObject
{
    Q_OBJECT
public:
    explicit ZanshinContext(QObject *parent = 0)
        : QObject(parent),
          app(0),
          presentation(0),
          editor(0),
          proxyModel(new QSortFilterProxyModel(this))
    {
        using namespace Presentation;
        proxyModel->setDynamicSortFilter(true);

        auto appModel = new ApplicationModel(new Akonadi::ArtifactQueries(this),
                                             new Akonadi::ProjectQueries(this),
                                             new Akonadi::ProjectRepository(this),
                                             new Akonadi::ContextQueries(this),
                                             new Akonadi::ContextRepository(this),
                                             new Akonadi::DataSourceQueries(this),
                                             new Akonadi::DataSourceRepository(this),
                                             new Akonadi::TaskQueries(this),
                                             new Akonadi::TaskRepository(this),
                                             new Akonadi::NoteRepository(this),
                                             new Akonadi::TagQueries(this),
                                             0,
                                             this);
        // Since it is lazy loaded force ourselves in a known state
        appModel->defaultNoteDataSource();
        appModel->defaultTaskDataSource();

        app = appModel;
    }

    ~ZanshinContext()
    {
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

    QObject *app;

    QList<QPersistentModelIndex> indices;
    QPersistentModelIndex index;
    QObject *presentation;
    QObject *editor;
    QList<QPersistentModelIndex> dragIndices;

private:
    QSortFilterProxyModel *proxyModel;
};

namespace Zanshin {

QString indexString(const QModelIndex &index, int role = Qt::DisplayRole)
{
    if (role != Qt::DisplayRole)
        return index.data(role).toString();

    QString data = index.data(role).toString();

    if (index.parent().isValid())
        return indexString(index.parent(), role) + " / " + data;
    else
        return data;
}

QModelIndex findIndex(QAbstractItemModel *model,
                      const QString &string,
                      int role = Qt::DisplayRole,
                      const QModelIndex &root = QModelIndex())
{
    for (int row = 0; row < model->rowCount(root); row++) {
        const QModelIndex index = model->index(row, 0, root);
        if (indexString(index, role) == string)
            return index;

        if (model->rowCount(index) > 0) {
            const QModelIndex found = findIndex(model, string, role, index);
            if (found.isValid())
                return found;
        }
    }

    return QModelIndex();
}

void collectIndices(ZanshinContext *context, const QModelIndex &root = QModelIndex())
{
    QAbstractItemModel *model = context->model();
    for (int row = 0; row < model->rowCount(root); row++) {
        const QModelIndex index = model->index(row, 0, root);
        context->indices << index;
        if (model->rowCount(index) > 0)
            collectIndices(context, index);
    }
}

void dumpIndices(const QList<QPersistentModelIndex> &indices)
{
    qDebug() << "Dumping list of size:" << indices.size();
    for (int row = 0; row < indices.size(); row++) {
        qDebug() << row << indexString(indices.at(row));
    }
}

inline bool verify(bool statement, const char *str,
                   const char *file, int line)
{
    if (statement)
        return true;

    qDebug() << "Statement" << str << "returned FALSE";
    qDebug() << "Loc:" << file << line;
    return false;
}

template <typename T>
inline bool compare(T const &t1, T const &t2,
                    const char *actual, const char *expected,
                    const char *file, int line)
{
    if (t1 == t2)
        return true;

    qDebug() << "Compared values are not the same";
    qDebug() << "Actual (" << actual << ") :" << QTest::toString<T>(t1);
    qDebug() << "Expected (" << expected << ") :" << QTest::toString<T>(t2);
    qDebug() << "Loc:" << file << line;
    return false;
}

} // namespace Zanshin

#define COMPARE(actual, expected) \
do {\
    if (!Zanshin::compare(actual, expected, #actual, #expected, __FILE__, __LINE__))\
        BOOST_REQUIRE(false);\
} while (0)

#define COMPARE_OR_DUMP(actual, expected) \
do {\
    if (!Zanshin::compare(actual, expected, #actual, #expected, __FILE__, __LINE__)) {\
        Zanshin::dumpIndices(context->indices); \
        BOOST_REQUIRE(false);\
    }\
} while (0)

#define VERIFY(statement) \
do {\
    if (!Zanshin::verify((statement), #statement, __FILE__, __LINE__))\
        BOOST_REQUIRE(false);\
} while (0)

#define VERIFY_OR_DUMP(statement) \
do {\
    if (!Zanshin::verify((statement), #statement, __FILE__, __LINE__)) {\
        Zanshin::dumpIndices(context->indices); \
        BOOST_REQUIRE(false);\
    }\
} while (0)


GIVEN("^I display the available data sources$") {
    ScenarioScope<ZanshinContext> context;
    auto availableSources = context->app->property("availableSources").value<QObject*>();
    VERIFY(availableSources);

    auto sourceListModel = availableSources->property("sourceListModel").value<QAbstractItemModel*>();
    VERIFY(sourceListModel);

    context->presentation = availableSources;
    context->setModel(sourceListModel);
    QTest::qWait(500);
}

GIVEN("^I display the available (\\S+) data sources$") {
    REGEX_PARAM(QString, sourceType);

    ScenarioScope<ZanshinContext> context;
    auto propertyName = sourceType == "task" ? "taskSourcesModel"
                      : sourceType == "note" ? "noteSourcesModel"
                      : 0;

    context->setModel(context->app->property(propertyName).value<QAbstractItemModel*>());
    QTest::qWait(500);
}

GIVEN("^I display the available pages$") {
    ScenarioScope<ZanshinContext> context;
    context->presentation = context->app->property("availablePages").value<QObject*>();
    context->setModel(context->presentation->property("pageListModel").value<QAbstractItemModel*>());
    QTest::qWait(500);
}

GIVEN("^I display the \"(.*)\" page$") {
    REGEX_PARAM(QString, pageName);

    ScenarioScope<ZanshinContext> context;
    auto availablePages = context->app->property("availablePages").value<QObject*>();
    VERIFY(availablePages);

    auto pageListModel = availablePages->property("pageListModel").value<QAbstractItemModel*>();
    VERIFY(pageListModel);
    QTest::qWait(500);

    QModelIndex pageIndex = Zanshin::findIndex(pageListModel, pageName);
    VERIFY(pageIndex.isValid());

    QObject *page = 0;
    QMetaObject::invokeMethod(availablePages, "createPageForIndex",
                              Q_RETURN_ARG(QObject*, page),
                              Q_ARG(QModelIndex, pageIndex));
    VERIFY(page);

    VERIFY(context->app->setProperty("currentPage", QVariant::fromValue(page)));
    context->presentation = context->app->property("currentPage").value<QObject*>();
    QTest::qWait(500);
}

GIVEN("^there is an item named \"(.+)\" in the central list$") {
    REGEX_PARAM(QString, itemName);

    ScenarioScope<ZanshinContext> context;
    QTest::qWait(500);

    auto model = context->presentation->property("centralListModel").value<QAbstractItemModel*>();
    QTest::qWait(500);
    context->setModel(model);

    context->index = Zanshin::findIndex(context->model(), itemName);
    VERIFY_OR_DUMP(context->index.isValid());
}

GIVEN("^there is an item named \"(.+)\" in the available data sources$") {
    REGEX_PARAM(QString, itemName);

    ScenarioScope<ZanshinContext> context;
    QTest::qWait(500);

    auto availableSources = context->app->property("availableSources").value<QObject*>();
    VERIFY(availableSources);
    QTest::qWait(500);
    auto model = availableSources->property("sourceListModel").value<QAbstractItemModel*>();
    VERIFY(model);
    QTest::qWait(500);
    context->setModel(model);

    context->index = Zanshin::findIndex(context->model(), itemName);
    VERIFY_OR_DUMP(context->index.isValid());
}

GIVEN("^the central list contains items named:") {
    TABLE_PARAM(tableParam);

    ScenarioScope<ZanshinContext> context;
    context->dragIndices.clear();
    QTest::qWait(500);

    auto model = context->presentation->property("centralListModel").value<QAbstractItemModel*>();
    QTest::qWait(500);
    context->setModel(model);

    for (const auto row : tableParam.hashes()) {
        for (const auto it : row) {
            const QString itemName = QString::fromUtf8(it.second.data());
            QModelIndex index = Zanshin::findIndex(context->model(), itemName);
            VERIFY_OR_DUMP(index.isValid());
            context->dragIndices << index;
        }
    }
}

WHEN("^I look at the central list$") {
    ScenarioScope<ZanshinContext> context;

    auto model = context->presentation->property("centralListModel").value<QAbstractItemModel*>();
    QTest::qWait(500);
    context->setModel(model);
}

WHEN("^I check the item$") {
    ScenarioScope<ZanshinContext> context;
    VERIFY(context->model()->setData(context->index, Qt::Checked, Qt::CheckStateRole));
    QTest::qWait(1500);
}

WHEN("^I uncheck the item$") {
    ScenarioScope<ZanshinContext> context;
    VERIFY(context->model()->setData(context->index, Qt::Unchecked, Qt::CheckStateRole));
    QTest::qWait(1500);
}

WHEN("^I remove the item$") {
    ScenarioScope<ZanshinContext> context;
    VERIFY(QMetaObject::invokeMethod(context->presentation, "removeItem", Q_ARG(QModelIndex, context->index)));
    QTest::qWait(500);
}

WHEN("^I add a project named \"(.*)\" in the source named \"(.*)\"$") {
    REGEX_PARAM(QString, projectName);
    REGEX_PARAM(QString, sourceName);

    ScenarioScope<ZanshinContext> context;

    auto sourceList = context->app->property("taskSourcesModel").value<QAbstractItemModel*>();
    VERIFY(sourceList);
    QTest::qWait(500);
    QModelIndex index = Zanshin::findIndex(sourceList, sourceName);
    VERIFY(index.isValid());
    auto source = index.data(Presentation::QueryTreeModelBase::ObjectRole)
                       .value<Domain::DataSource::Ptr>();
    VERIFY(source);

    VERIFY(QMetaObject::invokeMethod(context->presentation, "addProject",
                                     Q_ARG(QString, projectName),
                                     Q_ARG(Domain::DataSource::Ptr, source)));
    QTest::qWait(500);
}

WHEN("^I rename a \"(.*)\" named \"(.*)\" to \"(.*)\"$") {
    qDebug() << "Renaming is happening ! ";
    REGEX_PARAM(QString, objectType);
    REGEX_PARAM(QString, oldName);
    REGEX_PARAM(QString, newName);

    const QString pageNodeName = (objectType == "project") ? "Projects / "
                               : (objectType == "context") ? "Contexts / "
                               : QString();

    VERIFY(!pageNodeName.isEmpty());

    ScenarioScope<ZanshinContext> context;
    auto availablePages = context->app->property("availablePages").value<QObject*>();
    VERIFY(availablePages);

    auto pageListModel = availablePages->property("pageListModel").value<QAbstractItemModel*>();
    VERIFY(pageListModel);
    QTest::qWait(500);

    QModelIndex pageIndex = Zanshin::findIndex(pageListModel, pageNodeName + oldName);
    VERIFY(pageIndex.isValid());

    pageListModel->setData(pageIndex, newName);
}

WHEN("^I remove a \"(.*)\" named \"(.*)\"$") {
    REGEX_PARAM(QString, objectType);
    REGEX_PARAM(QString, objectName);

    const QString pageNodeName = (objectType == "project") ? "Projects / "
                               : (objectType == "context") ? "Contexts / "
                               : QString();

    VERIFY(!pageNodeName.isEmpty());

    ScenarioScope<ZanshinContext> context;
    auto availablePages = context->app->property("availablePages").value<QObject*>();
    VERIFY(availablePages);

    auto pageListModel = availablePages->property("pageListModel").value<QAbstractItemModel*>();
    VERIFY(pageListModel);
    QTest::qWait(500);

    QModelIndex pageIndex = Zanshin::findIndex(pageListModel, pageNodeName + objectName);
    VERIFY(pageIndex.isValid());

    VERIFY(QMetaObject::invokeMethod(availablePages, "removeItem",
                                     Q_ARG(QModelIndex, pageIndex)));
    QTest::qWait(500);
}

WHEN("^I add a \"(.*)\" named \"(.+)\"$") {
    REGEX_PARAM(QString, objectType);
    REGEX_PARAM(QString, objectName);

    QByteArray actionName = (objectType == "context") ? "addContext"
                          : (objectType == "task")    ? "addTask"
                          : (objectType == "tag")     ? "addTag"
                          : QByteArray();

    VERIFY(!actionName.isEmpty());

    ScenarioScope<ZanshinContext> context;
    QTest::qWait(500);

    VERIFY(QMetaObject::invokeMethod(context->presentation,
                                     actionName.data(),
                                     Q_ARG(QString, objectName)));
    QTest::qWait(500);
}

WHEN("^I list the items$") {
    ScenarioScope<ZanshinContext> context;
    context->indices.clear();
    Zanshin::collectIndices(context.get());
}

WHEN("^I open the item in the editor$") {
    ScenarioScope<ZanshinContext> context;
    auto artifact = context->index.data(Presentation::QueryTreeModelBase::ObjectRole)
                                  .value<Domain::Artifact::Ptr>();
    VERIFY(artifact);
    context->editor = context->app->property("editor").value<QObject*>();
    VERIFY(context->editor);
    VERIFY(context->editor->setProperty("artifact", QVariant::fromValue(artifact)));
}

WHEN("^I mark it done in the editor$") {
    ScenarioScope<ZanshinContext> context;
    VERIFY(context->editor->setProperty("done", true));
}

WHEN("^I change the editor (.*) to \"(.*)\"$") {
    REGEX_PARAM(QString, field);
    REGEX_PARAM(QString, string);

    const QVariant value = (field == "text") ? string
                         : (field == "title") ? string
                         : (field == "start date") ? QDateTime::fromString(string, Qt::ISODate)
                         : (field == "due date") ? QDateTime::fromString(string, Qt::ISODate)
                         : QVariant();

    const QByteArray property = (field == "text") ? field.toUtf8()
                              : (field == "title") ? field.toUtf8()
                              : (field == "start date") ? "startDate"
                              : (field == "due date") ? "dueDate"
                              : QByteArray();

    VERIFY(value.isValid());
    VERIFY(!property.isEmpty());

    ScenarioScope<ZanshinContext> context;
    VERIFY(context->editor->setProperty(property, value));
}

WHEN("^I open the item in the editor again$") {
    ScenarioScope<ZanshinContext> context;
    auto artifact = context->index.data(Presentation::QueryTreeModelBase::ObjectRole)
                                  .value<Domain::Artifact::Ptr>();
    VERIFY(artifact);
    VERIFY(context->editor->setProperty("artifact", QVariant::fromValue(Domain::Artifact::Ptr())));
    VERIFY(context->editor->setProperty("artifact", QVariant::fromValue(artifact)));
    QTest::qWait(500);
}

WHEN("^I drop the item on \"(.*)\" in the central list") {
    REGEX_PARAM(QString, itemName);

    ScenarioScope<ZanshinContext> context;
    VERIFY(context->index.isValid());
    const QMimeData *data = context->model()->mimeData(QModelIndexList() << context->index);

    QAbstractItemModel *destModel = context->model();
    QModelIndex dropIndex = Zanshin::findIndex(destModel, itemName);
    VERIFY(dropIndex.isValid());
    VERIFY(destModel->dropMimeData(data, Qt::MoveAction, -1, -1, dropIndex));
    QTest::qWait(500);
}

WHEN("^I drop items on \"(.*)\" in the central list") {
    REGEX_PARAM(QString, itemName);

    ScenarioScope<ZanshinContext> context;
    VERIFY(!context->dragIndices.isEmpty());
    QModelIndexList indexes;
    std::transform(context->dragIndices.constBegin(), context->dragIndices.constEnd(),
                   std::back_inserter(indexes),
                   [] (const QPersistentModelIndex &index) {
                        VERIFY(index.isValid());
                        return index;
                   });

    const QMimeData *data = context->model()->mimeData(indexes);

    QAbstractItemModel *destModel = context->model();
    QModelIndex dropIndex = Zanshin::findIndex(destModel, itemName);
    VERIFY(dropIndex.isValid());
    VERIFY(destModel->dropMimeData(data, Qt::MoveAction, -1, -1, dropIndex));
    QTest::qWait(500);
}

WHEN("^I drop the item on \"(.*)\" in the page list") {
    REGEX_PARAM(QString, itemName);

    ScenarioScope<ZanshinContext> context;
    VERIFY(context->index.isValid());
    const QMimeData *data = context->model()->mimeData(QModelIndexList() << context->index);

    auto availablePages = context->app->property("availablePages").value<QObject*>();
    VERIFY(availablePages);

    auto destModel = availablePages->property("pageListModel").value<QAbstractItemModel*>();
    VERIFY(destModel);
    QTest::qWait(500);

    QModelIndex dropIndex = Zanshin::findIndex(destModel, itemName);
    VERIFY(dropIndex.isValid());
    VERIFY(destModel->dropMimeData(data, Qt::MoveAction, -1, -1, dropIndex));
    QTest::qWait(500);
}

WHEN("^I drop items on \"(.*)\" in the page list") {
    REGEX_PARAM(QString, itemName);

    ScenarioScope<ZanshinContext> context;
    VERIFY(!context->dragIndices.isEmpty());
    QModelIndexList indexes;
    std::transform(context->dragIndices.constBegin(), context->dragIndices.constEnd(),
                   std::back_inserter(indexes),
                   [] (const QPersistentModelIndex &index) {
                        VERIFY(index.isValid());
                        return index;
                   });

    const QMimeData *data = context->model()->mimeData(indexes);

    auto availablePages = context->app->property("availablePages").value<QObject*>();
    VERIFY(availablePages);

    auto destModel = availablePages->property("pageListModel").value<QAbstractItemModel*>();
    VERIFY(destModel);
    QTest::qWait(500);

    QModelIndex dropIndex = Zanshin::findIndex(destModel, itemName);
    VERIFY(dropIndex.isValid());
    VERIFY(destModel->dropMimeData(data, Qt::MoveAction, -1, -1, dropIndex));
    QTest::qWait(500);
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
    auto sourcesModel = sourceType == "task" ? context->app->property("taskSourcesModel").value<QAbstractItemModel*>()
                      : sourceType == "note" ? context->app->property("noteSourcesModel").value<QAbstractItemModel*>()
                      : 0;
    QTest::qWait(500);
    // I wish models had iterators...
    QList<Domain::DataSource::Ptr> sources;
    for (int i = 0; i < sourcesModel->rowCount(); i++)
        sources << sourcesModel->index(i, 0).data(Presentation::QueryTreeModelBase::ObjectRole)
                                            .value<Domain::DataSource::Ptr>();
    auto source = *std::find_if(sources.begin(), sources.end(),
                                [=] (const Domain::DataSource::Ptr &source) {
                                    return source->name() == sourceName;
                                });


    auto propertyName = sourceType == "task" ? "defaultTaskDataSource"
                      : sourceType == "note" ? "defaultNoteDataSource"
                      : 0;
    context->app->setProperty(propertyName, QVariant::fromValue(source));
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
            VERIFY_OR_DUMP(role != -1);
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
            COMPARE_OR_DUMP(Zanshin::indexString(resultIndex, role),
                            Zanshin::indexString(expectedIndex, role));
        }
    }
    COMPARE_OR_DUMP(context->indices.size(), proxy.rowCount());
}

THEN("^the list contains \"(.+)\"$") {
    REGEX_PARAM(QString, itemName);

    ScenarioScope<ZanshinContext> context;
    for (int row = 0; row < context->indices.size(); row++) {
        if (Zanshin::indexString(context->indices.at(row)) == itemName)
            return;
    }

    VERIFY_OR_DUMP(false);
}

THEN("^the list does not contain \"(.+)\"$") {
    REGEX_PARAM(QString, itemName);

    ScenarioScope<ZanshinContext> context;
    for (int row = 0; row < context->indices.size(); row++) {
        VERIFY_OR_DUMP(Zanshin::indexString(context->indices.at(row)) != itemName);
    }
}

THEN("^the task corresponding to the item is done$") {
    ScenarioScope<ZanshinContext> context;
    auto artifact = context->index.data(Presentation::QueryTreeModelBase::ObjectRole).value<Domain::Artifact::Ptr>();
    VERIFY(artifact);
    auto task = artifact.dynamicCast<Domain::Task>();
    VERIFY(task);
    VERIFY(task->isDone());
}

THEN("^the editor shows the task as done$") {
    ScenarioScope<ZanshinContext> context;
    VERIFY(context->editor->property("done").toBool());
}

THEN("^the editor shows \"(.*)\" as (.*)$") {
    REGEX_PARAM(QString, string);
    REGEX_PARAM(QString, field);

    const QVariant value = (field == "text") ? string
                         : (field == "title") ? string
                         : (field == "start date") ? QDateTime::fromString(string, Qt::ISODate)
                         : (field == "due date") ? QDateTime::fromString(string, Qt::ISODate)
                         : QVariant();

    const QByteArray property = (field == "text") ? field.toUtf8()
                              : (field == "title") ? field.toUtf8()
                              : (field == "start date") ? "startDate"
                              : (field == "due date") ? "dueDate"
                              : QByteArray();

    VERIFY(value.isValid());
    VERIFY(!property.isEmpty());

    ScenarioScope<ZanshinContext> context;
    COMPARE(context->editor->property(property), value);
}

THEN("^the default (\\S+) data source is (.*)$") {
    REGEX_PARAM(QString, sourceType);
    REGEX_PARAM(QString, expectedName);

    auto propertyName = sourceType == "task" ? "defaultTaskDataSource"
                      : sourceType == "note" ? "defaultNoteDataSource"
                      : 0;

    ScenarioScope<ZanshinContext> context;
    auto source = context->app->property(propertyName).value<Domain::DataSource::Ptr>();
    VERIFY(!source.isNull());
    COMPARE(source->name(), expectedName);
}

THEN("^the setting key (\\S+) is (\\d+)$") {
    REGEX_PARAM(QString, keyName);
    REGEX_PARAM(qint64, expectedId);

    KConfigGroup config(KGlobal::config(), "General");
    const qint64 id = config.readEntry(keyName, -1);
    COMPARE(id, expectedId);
}

#include "main.moc"
