/* This file is part of Zanshin

   Copyright 2014-2015 Kevin Ottens <ervin@kde.org>

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

#include <boost/test/unit_test.hpp>
#include <cucumber-cpp/defs.hpp>

#include <QApplication>
#include <QDebug>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTest>

#include <KConfig>
#include <KConfigGroup>

#include "presentation/applicationmodel.h"
#include "presentation/errorhandler.h"
#include "presentation/querytreemodelbase.h"

#include "akonadi/akonadimonitorimpl.h"
#include "akonadi/akonadimessaginginterface.h"

#include "utils/dependencymanager.h"
#include "utils/jobhandler.h"

#include "testlib/akonadifakedata.h"
#include "testlib/akonadifakedataxmlloader.h"
#include "testlib/monitorspy.h"
#include "testlib/testsafety.h"

static int argc = 1;
static char *argv0 = "cuke-steps";
static QApplication app(argc, &argv0);

namespace CukeSteps
{
    void initializeAppDependencies();
}

namespace cucumber {
    namespace internal {
        template<>
        inline QString fromString(const std::string& s) {
            return QString::fromUtf8(s.data());
        }
    }
}

using namespace cucumber;

class FakeErrorHandler : public Presentation::ErrorHandler
{
public:
    void doDisplayMessage(const QString &)
    {
    }
};

class ZanshinContext : public QObject
{
    Q_OBJECT
public:
    explicit ZanshinContext(QObject *parent = Q_NULLPTR)
        : QObject(parent),
          app(Q_NULLPTR),
          presentation(Q_NULLPTR),
          editor(Q_NULLPTR),
          proxyModel(new QSortFilterProxyModel(this)),
          m_model(Q_NULLPTR),
          m_sourceModel(Q_NULLPTR),
          monitorSpy(Q_NULLPTR)
    {
        qputenv("ZANSHIN_OVERRIDE_DATETIME", "2015-03-10");

        static bool initializedDependencies = false;

        if (!initializedDependencies) {
            CukeSteps::initializeAppDependencies();

            QString xmlFile = QString::fromLocal8Bit(qgetenv("ZANSHIN_USER_XMLDATA"));
            if (!xmlFile.isEmpty()) {
                auto searchCollection = Akonadi::Collection(1);
                searchCollection.setParentCollection(Akonadi::Collection::root());
                searchCollection.setName(QStringLiteral("Search"));
                m_data.createCollection(searchCollection);

                MonitorSpy::setExpirationDelay(200);
                auto loader = Testlib::AkonadiFakeDataXmlLoader(&m_data);
                loader.load(xmlFile);

                // Swap regular dependencies for the fake data ones
                auto &deps = Utils::DependencyManager::globalInstance();
                deps.add<Akonadi::MonitorInterface,
                         Utils::DependencyManager::UniqueInstance>(
                         [this] (Utils::DependencyManager *) {
                             return m_data.createMonitor();
                         }
                );
                deps.add<Akonadi::StorageInterface,
                         Utils::DependencyManager::UniqueInstance>(
                         [this] (Utils::DependencyManager *) {
                             return m_data.createStorage();
                         }
                );
                deps.add<Akonadi::MessagingInterface,
                         Utils::DependencyManager::UniqueInstance>(
                         [this] (Utils::DependencyManager *) -> Akonadi::MessagingInterface* {
                            return Q_NULLPTR;
                         }
                );
            } else if (!TestLib::TestSafety::checkTestIsIsolated()) {
                qDebug() << "FATAL ERROR! SEE ABOVE\n\n";
                exit(1);
            }

            initializedDependencies = true;
        }

        using namespace Presentation;
        proxyModel->setDynamicSortFilter(true);

        auto appModel = ApplicationModel::Ptr::create();

        appModel->setErrorHandler(&m_errorHandler);

        app = appModel;

        auto monitor = Utils::DependencyManager::globalInstance().create<Akonadi::MonitorInterface>();
        monitorSpy = new MonitorSpy(monitor.data(), this);
    }

    ~ZanshinContext()
    {
    }

    // Note that setModel might invalidate the 'index' member variable, due to proxyModel->setSourceModel.
    void setModel(QAbstractItemModel *model)
    {
        if (m_sourceModel == model)
            return;
        m_sourceModel = model;
        if (!qobject_cast<QSortFilterProxyModel *>(model)) {
            proxyModel->setObjectName(QStringLiteral("m_proxyModel_in_ZanshinContext"));
            proxyModel->setSourceModel(model);
            proxyModel->setSortRole(Qt::DisplayRole);
            proxyModel->sort(0);
            m_model = proxyModel;
        } else {
            m_model = model;
        }
    }

    QAbstractItemModel *sourceModel()
    {
        return m_sourceModel;
    }

    QAbstractItemModel *model()
    {
        return m_model;
    }

    void waitForEmptyJobQueue()
    {
        while (Utils::JobHandler::jobCount() != 0) {
            QTest::qWait(20);
        }
    }

    void waitForStableState()
    {
        waitForEmptyJobQueue();
        monitorSpy->waitForStableState();
    }

    QObjectPtr app;

    QList<QPersistentModelIndex> indices;
    QPersistentModelIndex index;
    QObject *presentation;
    QObject *editor;
    QList<QPersistentModelIndex> dragIndices;

private:
    static Testlib::AkonadiFakeData m_data;

    QSortFilterProxyModel *proxyModel;
    QAbstractItemModel *m_model;
    QAbstractItemModel *m_sourceModel;
    MonitorSpy *monitorSpy;
    FakeErrorHandler m_errorHandler;
};

Testlib::AkonadiFakeData ZanshinContext::m_data = {};

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

static void collectIndicesImpl(ZanshinContext *context, const QModelIndex &root = QModelIndex())
{
    QAbstractItemModel *model = context->model();
    for (int row = 0; row < model->rowCount(root); row++) {
        const QModelIndex index = model->index(row, 0, root);
        context->indices << index;
        if (model->rowCount(index) > 0)
            collectIndicesImpl(context, index);
    }
}

static void collectIndices(ZanshinContext *context)
{
    context->indices.clear();
    collectIndicesImpl(context);
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

// Note: you should make sure that context->indices is filled in before calling this,
// e.g. calling Zanshin::collectIndices(context.get()) if not already done.
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

// Note: you should make sure that context->indices is filled in before calling this,
// e.g. calling Zanshin::collectIndices(context.get()) if not already done.
#define VERIFY_OR_DUMP(statement) \
do {\
    if (!Zanshin::verify((statement), #statement, __FILE__, __LINE__)) {\
        Zanshin::dumpIndices(context->indices); \
        BOOST_REQUIRE(false);\
    }\
} while (0)

#define VERIFY_OR_DO(statement, whatToDo) \
do {\
    if (!Zanshin::verify((statement), #statement, __FILE__, __LINE__)) {\
        whatToDo; \
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
}

GIVEN("^I display the available pages$") {
    ScenarioScope<ZanshinContext> context;
    context->presentation = context->app->property("availablePages").value<QObject*>();
    context->setModel(context->presentation->property("pageListModel").value<QAbstractItemModel*>());
}

GIVEN("^I display the \"(.*)\" page$") {
    REGEX_PARAM(QString, pageName);

    ScenarioScope<ZanshinContext> context;
    auto availablePages = context->app->property("availablePages").value<QObject*>();
    VERIFY(availablePages);

    auto pageListModel = availablePages->property("pageListModel").value<QAbstractItemModel*>();
    VERIFY(pageListModel);
    context->waitForEmptyJobQueue();

    QModelIndex pageIndex = Zanshin::findIndex(pageListModel, pageName);
    VERIFY(pageIndex.isValid());

    QObject *page = Q_NULLPTR;
    QMetaObject::invokeMethod(availablePages, "createPageForIndex",
                              Q_RETURN_ARG(QObject*, page),
                              Q_ARG(QModelIndex, pageIndex));
    VERIFY(page);

    VERIFY(context->app->setProperty("currentPage", QVariant::fromValue(page)));
    context->presentation = context->app->property("currentPage").value<QObject*>();
}

GIVEN("^there is an item named \"(.+)\" in the central list$") {
    REGEX_PARAM(QString, itemName);

    ScenarioScope<ZanshinContext> context;

    auto model = context->presentation->property("centralListModel").value<QAbstractItemModel*>();
    context->setModel(model);
    context->waitForEmptyJobQueue();

    Zanshin::collectIndices(context.get());
    context->index = Zanshin::findIndex(context->model(), itemName);
    VERIFY_OR_DUMP(context->index.isValid());
}

GIVEN("^there is an item named \"(.+)\" in the available data sources$") {
    REGEX_PARAM(QString, itemName);

    ScenarioScope<ZanshinContext> context;

    auto availableSources = context->app->property("availableSources").value<QObject*>();
    VERIFY(availableSources);
    auto model = availableSources->property("sourceListModel").value<QAbstractItemModel*>();
    VERIFY(model);
    context->waitForEmptyJobQueue();
    context->setModel(model);

    Zanshin::collectIndices(context.get());
    context->index = Zanshin::findIndex(context->model(), itemName);
    VERIFY_OR_DUMP(context->index.isValid());
}

GIVEN("^the central list contains items named:") {
    TABLE_PARAM(tableParam);

    ScenarioScope<ZanshinContext> context;
    context->dragIndices.clear();

    auto model = context->presentation->property("centralListModel").value<QAbstractItemModel*>();
    context->waitForEmptyJobQueue();
    context->setModel(model);

    for (const auto &row : tableParam.hashes()) {
        for (const auto &it : row) {
            const QString itemName = QString::fromUtf8(it.second.data());
            QModelIndex index = Zanshin::findIndex(context->model(), itemName);
            VERIFY_OR_DO(index.isValid(), Zanshin::dumpIndices(context->dragIndices));
            context->dragIndices << index;
        }
    }
}

WHEN("^I look at the central list$") {
    ScenarioScope<ZanshinContext> context;

    auto model = context->presentation->property("centralListModel").value<QAbstractItemModel*>();
    context->setModel(model);
    context->waitForStableState();
}

WHEN("^I check the item$") {
    ScenarioScope<ZanshinContext> context;
    VERIFY(context->model()->setData(context->index, Qt::Checked, Qt::CheckStateRole));
    context->waitForStableState();
}

WHEN("^I uncheck the item$") {
    ScenarioScope<ZanshinContext> context;
    VERIFY(context->model()->setData(context->index, Qt::Unchecked, Qt::CheckStateRole));
    context->waitForStableState();
}

WHEN("^I remove the item$") {
    ScenarioScope<ZanshinContext> context;
    VERIFY(QMetaObject::invokeMethod(context->presentation, "removeItem", Q_ARG(QModelIndex, context->index)));
    context->waitForStableState();
}

WHEN("^I promote the item$") {
    ScenarioScope<ZanshinContext> context;
    VERIFY(QMetaObject::invokeMethod(context->presentation, "promoteItem", Q_ARG(QModelIndex, context->index)));
    context->waitForStableState();
}

WHEN("^I add a project named \"(.*)\" in the source named \"(.*)\"$") {
    REGEX_PARAM(QString, projectName);
    REGEX_PARAM(QString, sourceName);

    ScenarioScope<ZanshinContext> context;

    auto availableSources = context->app->property("availableSources").value<QObject*>();
    VERIFY(availableSources);
    auto sourceList = availableSources->property("sourceListModel").value<QAbstractItemModel*>();
    VERIFY(sourceList);
    context->waitForStableState();
    QModelIndex index = Zanshin::findIndex(sourceList, sourceName);
    VERIFY(index.isValid());
    auto source = index.data(Presentation::QueryTreeModelBase::ObjectRole)
                       .value<Domain::DataSource::Ptr>();
    VERIFY(source);

    VERIFY(QMetaObject::invokeMethod(context->presentation, "addProject",
                                     Q_ARG(QString, projectName),
                                     Q_ARG(Domain::DataSource::Ptr, source)));
    context->waitForStableState();
}

WHEN("^I rename a \"(.*)\" named \"(.*)\" to \"(.*)\"$") {
    REGEX_PARAM(QString, objectType);
    REGEX_PARAM(QString, oldName);
    REGEX_PARAM(QString, newName);

    const QString pageNodeName = (objectType == QStringLiteral("project")) ? QStringLiteral("Projects / ")
                               : (objectType == QStringLiteral("context")) ? QStringLiteral("Contexts / ")
                               : QString();

    VERIFY(!pageNodeName.isEmpty());

    ScenarioScope<ZanshinContext> context;
    auto availablePages = context->app->property("availablePages").value<QObject*>();
    VERIFY(availablePages);

    auto pageListModel = availablePages->property("pageListModel").value<QAbstractItemModel*>();
    VERIFY(pageListModel);
    context->waitForStableState();

    QModelIndex pageIndex = Zanshin::findIndex(pageListModel, pageNodeName + oldName);
    VERIFY(pageIndex.isValid());

    pageListModel->setData(pageIndex, newName);
    context->waitForStableState();
}

WHEN("^I remove a \"(.*)\" named \"(.*)\"$") {
    REGEX_PARAM(QString, objectType);
    REGEX_PARAM(QString, objectName);

    const QString pageNodeName = (objectType == QStringLiteral("project")) ? QStringLiteral("Projects / ")
                               : (objectType == QStringLiteral("context")) ? QStringLiteral("Contexts / ")
                               : (objectType == QStringLiteral("tag"))     ? QStringLiteral("Tags / ")
                               : QString();

    VERIFY(!pageNodeName.isEmpty());

    ScenarioScope<ZanshinContext> context;
    auto availablePages = context->app->property("availablePages").value<QObject*>();
    VERIFY(availablePages);

    auto pageListModel = availablePages->property("pageListModel").value<QAbstractItemModel*>();
    VERIFY(pageListModel);
    context->waitForStableState();

    QModelIndex pageIndex = Zanshin::findIndex(pageListModel, pageNodeName + objectName);
    VERIFY(pageIndex.isValid());

    VERIFY(QMetaObject::invokeMethod(availablePages, "removeItem",
                                     Q_ARG(QModelIndex, pageIndex)));
    context->waitForStableState();
}

WHEN("^I add a \"(.*)\" named \"(.+)\"$") {
    REGEX_PARAM(QString, objectType);
    REGEX_PARAM(QString, objectName);

    QByteArray actionName = (objectType == QStringLiteral("context")) ? "addContext"
                          : (objectType == QStringLiteral("note"))    ? "addItem"
                          : (objectType == QStringLiteral("task"))    ? "addItem"
                          : (objectType == QStringLiteral("tag"))     ? "addTag"
                          : QByteArray();

    VERIFY(!actionName.isEmpty());

    ScenarioScope<ZanshinContext> context;
    context->waitForStableState();

    VERIFY(QMetaObject::invokeMethod(context->presentation,
                                     actionName.data(),
                                     Q_ARG(QString, objectName)));
    context->waitForStableState();
}

WHEN("^I add a child named \"(.+)\" under the task named \"(.+)\"$") {
    REGEX_PARAM(QString, childName);
    REGEX_PARAM(QString, parentName);

    ScenarioScope<ZanshinContext> context;
    context->waitForStableState();

    auto parentIndex = QModelIndex();
    for (int row = 0; row < context->indices.size(); row++) {
        auto index = context->indices.at(row);
        if (Zanshin::indexString(index) == parentName) {
            parentIndex = index;
            break;
        }
    }

    VERIFY_OR_DUMP(parentIndex.isValid());

    VERIFY(QMetaObject::invokeMethod(context->presentation,
                                     "addItem",
                                     Q_ARG(QString, childName),
                                     Q_ARG(QModelIndex, parentIndex)));
    context->waitForStableState();
}

WHEN("^I list the items$") {
    ScenarioScope<ZanshinContext> context;
    context->waitForStableState();
    Zanshin::collectIndices(context.get());
    context->waitForStableState();
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

    const QVariant value = (field == QStringLiteral("text")) ? string
                         : (field == QStringLiteral("title")) ? string
                         : (field == QStringLiteral("start date")) ? QDateTime::fromString(string, Qt::ISODate)
                         : (field == QStringLiteral("due date")) ? QDateTime::fromString(string, Qt::ISODate)
                         : QVariant();

    const QByteArray property = (field == QStringLiteral("text")) ? field.toUtf8()
                              : (field == QStringLiteral("title")) ? field.toUtf8()
                              : (field == QStringLiteral("start date")) ? "startDate"
                              : (field == QStringLiteral("due date")) ? "dueDate"
                              : QByteArray();

    VERIFY(value.isValid());
    VERIFY(!property.isEmpty());

    ScenarioScope<ZanshinContext> context;
    VERIFY(context->editor->setProperty(property, value));
}

WHEN("^I rename the item to \"(.+)\"$") {
    REGEX_PARAM(QString, title);
    ScenarioScope<ZanshinContext> context;
    VERIFY(context->model()->setData(context->index, title, Qt::EditRole));
    context->waitForStableState();
}

WHEN("^I open the item in the editor again$") {
    ScenarioScope<ZanshinContext> context;
    auto artifact = context->index.data(Presentation::QueryTreeModelBase::ObjectRole)
                                  .value<Domain::Artifact::Ptr>();
    VERIFY(artifact);
    VERIFY(context->editor->setProperty("artifact", QVariant::fromValue(Domain::Artifact::Ptr())));
    VERIFY(context->editor->setProperty("artifact", QVariant::fromValue(artifact)));
    context->waitForStableState();
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
    context->waitForStableState();
}

WHEN("^I drop the item on the blank area of the central list") {

    ScenarioScope<ZanshinContext> context;
    VERIFY(context->index.isValid());
    const QMimeData *data = context->model()->mimeData(QModelIndexList() << context->index);

    QAbstractItemModel *destModel = context->model();
    VERIFY(destModel->dropMimeData(data, Qt::MoveAction, -1, -1, QModelIndex()));
    context->waitForStableState();
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
    context->waitForStableState();
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
    context->waitForStableState();

    QModelIndex dropIndex = Zanshin::findIndex(destModel, itemName);
    VERIFY(dropIndex.isValid());
    VERIFY(destModel->dropMimeData(data, Qt::MoveAction, -1, -1, dropIndex));
    context->waitForStableState();
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
    context->waitForStableState();

    QModelIndex dropIndex = Zanshin::findIndex(destModel, itemName);
    VERIFY(dropIndex.isValid());
    VERIFY(destModel->dropMimeData(data, Qt::MoveAction, -1, -1, dropIndex));
    context->waitForStableState();
}

WHEN("^the setting key (\\S+) changes to (\\d+)$") {
    REGEX_PARAM(QString, keyName);
    REGEX_PARAM(qint64, id);

    ScenarioScope<ZanshinContext> context;
    KConfigGroup config(KSharedConfig::openConfig(), "General");
    config.writeEntry(keyName, id);
}

WHEN("^the user changes the default data source to \"(.*)\"$") {
    REGEX_PARAM(QString, sourceName);

    ScenarioScope<ZanshinContext> context;
    context->waitForStableState();
    auto sourceIndex = Zanshin::findIndex(context->model(), sourceName);
    auto availableSources = context->app->property("availableSources").value<QObject*>();
    VERIFY(availableSources);
    VERIFY(QMetaObject::invokeMethod(availableSources, "setDefaultItem", Q_ARG(QModelIndex, sourceIndex)));
    context->waitForStableState();
}


THEN("^the list is") {
    TABLE_PARAM(tableParam);

    ScenarioScope<ZanshinContext> context;
    auto roleNames = context->model()->roleNames();
    QSet<int> usedRoles;

    QStandardItemModel inputModel;
    for (const auto &row : tableParam.hashes()) {
        QStandardItem *item = new QStandardItem;
        for (const auto &it : row) {
            const QByteArray roleName = it.first.data();
            const QString value = QString::fromUtf8(it.second.data());
            const int role = roleNames.key(roleName, -1);
            VERIFY_OR_DUMP(role != -1);
            item->setData(value, role);
            usedRoles.insert(role);
        }
        inputModel.appendRow(item);
    }

    QSortFilterProxyModel proxy;

    QAbstractItemModel *referenceModel;
    if (!qobject_cast<QSortFilterProxyModel *>(context->sourceModel())) {
        referenceModel = &proxy;
        proxy.setSourceModel(&inputModel);
        proxy.setSortRole(Qt::DisplayRole);
        proxy.sort(0);
        proxy.setObjectName(QStringLiteral("the_list_is_proxy"));
    } else {
        referenceModel = &inputModel;
    }

    for (int row = 0; row < context->indices.size(); row++) {
        QModelIndex expectedIndex = referenceModel->index(row, 0);
        QModelIndex resultIndex = context->indices.at(row);

        foreach (const auto &role, usedRoles) {
            COMPARE_OR_DUMP(Zanshin::indexString(resultIndex, role),
                            Zanshin::indexString(expectedIndex, role));
        }
    }
    COMPARE_OR_DUMP(context->indices.size(), referenceModel->rowCount());
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

    const QVariant value = (field == QStringLiteral("text")) ? string
                         : (field == QStringLiteral("title")) ? string
                         : (field == QStringLiteral("delegate")) ? string
                         : (field == QStringLiteral("start date")) ? QDateTime::fromString(string, Qt::ISODate)
                         : (field == QStringLiteral("due date")) ? QDateTime::fromString(string, Qt::ISODate)
                         : QVariant();

    const QByteArray property = (field == QStringLiteral("text")) ? field.toUtf8()
                              : (field == QStringLiteral("title")) ? field.toUtf8()
                              : (field == QStringLiteral("delegate")) ? "delegateText"
                              : (field == QStringLiteral("start date")) ? "startDate"
                              : (field == QStringLiteral("due date")) ? "dueDate"
                              : QByteArray();

    VERIFY(value.isValid());
    VERIFY(!property.isEmpty());

    ScenarioScope<ZanshinContext> context;
    COMPARE(context->editor->property(property), value);
}

THEN("^the default data source is \"(.*)\"$") {
    REGEX_PARAM(QString, expectedName);

    ScenarioScope<ZanshinContext> context;
    context->waitForStableState();
    auto expectedIndex = Zanshin::findIndex(context->model(), expectedName);
    VERIFY(expectedIndex.isValid());
    auto defaultRole = context->model()->roleNames().key("default", -1);
    VERIFY(expectedIndex.data(defaultRole).toBool());
}

THEN("^the setting key (\\S+) is (\\d+)$") {
    REGEX_PARAM(QString, keyName);
    REGEX_PARAM(qint64, expectedId);

    KConfigGroup config(KSharedConfig::openConfig(), "General");
    const qint64 id = config.readEntry(keyName, -1);
    COMPARE(id, expectedId);
}

#include "cuke-steps.moc"
