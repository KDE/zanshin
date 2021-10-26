/* This file is part of Zanshin

   Copyright 2014-2019 Kevin Ottens <ervin@kde.org>

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

#include "zanshincontext.h"

#include "akonadi/akonadiapplicationselectedattribute.h"
#include "akonadi/akonadicachingstorage.h"
#include "akonadi/akonadistorageinterface.h"
#include "akonadi/akonaditimestampattribute.h"

#include "presentation/applicationmodel.h"
#include "presentation/errorhandler.h"
#include "presentation/querytreemodelbase.h"

#include "testlib/akonadifakedataxmlloader.h"
#include "testlib/monitorspy.h"

#include "utils/dependencymanager.h"
#include "utils/jobhandler.h"
#include "integration/dependencies.h"

#include <AkonadiCore/Akonadi/AttributeFactory>

#include <KConfigGroup>
#include <KSharedConfig>

#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTest>

void FakeErrorHandler::doDisplayMessage(const QString &)
{
}

ZanshinContext::ZanshinContext(QObject *parent)
    : QObject(parent),
      m_presentation(nullptr),
      m_editor(nullptr),
      m_proxyModel(new QSortFilterProxyModel(this)),
      m_model(nullptr),
      m_sourceModel(nullptr),
      m_monitorSpy(nullptr)
{
    qputenv("ZANSHIN_OVERRIDE_DATE", "2015-03-10");

    static bool initializedDependencies = false;

    if (!initializedDependencies) {
        Integration::initializeGlobalAppDependencies();
        MonitorSpy::setExpirationDelay(200);
        initializedDependencies = true;
    }

    Akonadi::AttributeFactory::registerAttribute<Akonadi::ApplicationSelectedAttribute>();
    Akonadi::AttributeFactory::registerAttribute<Akonadi::TimestampAttribute>();

    const auto xmlFile = QString::fromLocal8Bit(ZANSHIN_USER_XMLDATA);
    if (xmlFile.isEmpty()) {
        qDebug() << "FATAL ERROR! ZANSHIN_USER_XMLDATA WAS NOT PROVIDED\n\n";
        exit(1);
    }

    auto searchCollection = Akonadi::Collection(1);
    searchCollection.setParentCollection(Akonadi::Collection::root());
    searchCollection.setName(QStringLiteral("Search"));
    m_data.createCollection(searchCollection);

    auto loader = Testlib::AkonadiFakeDataXmlLoader(&m_data);
    loader.load(xmlFile);

    // Sanity checks
    QVERIFY(m_data.collections().size() > 1);
    QVERIFY(m_data.items().size() > 1);
    QVERIFY(m_data.contexts().size() > 1);

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
                [this] (Utils::DependencyManager *deps) {
        return new Akonadi::CachingStorage(deps->create<Akonadi::Cache>(),
                                           Akonadi::StorageInterface::Ptr(m_data.createStorage()));
    }
    );

    using namespace Presentation;
    m_proxyModel->setDynamicSortFilter(true);

    auto appModel = ApplicationModel::Ptr::create();

    appModel->setErrorHandler(&m_errorHandler);

    m_appModel = appModel;

    auto monitor = Utils::DependencyManager::globalInstance().create<Akonadi::MonitorInterface>();
    m_monitorSpy = new MonitorSpy(monitor.data(), this);
}

// Note that setModel might invalidate the 'index' member variable, due to proxyModel->setSourceModel.
void ZanshinContext::setModel(QAbstractItemModel *model)
{
    if (m_sourceModel == model)
        return;
    m_sourceModel = model;
    if (!qobject_cast<QSortFilterProxyModel *>(model)) {
        m_proxyModel->setObjectName(QStringLiteral("m_proxyModel_in_ZanshinContext"));
        m_proxyModel->setSourceModel(model);
        m_proxyModel->setSortRole(Qt::DisplayRole);
        m_proxyModel->sort(0);
        m_model = m_proxyModel;
    } else {
        m_model = model;
    }
}

QAbstractItemModel *ZanshinContext::sourceModel() const
{
    return m_sourceModel;
}

QAbstractItemModel *ZanshinContext::model() const
{
    return m_model;
}

Domain::Task::Ptr ZanshinContext::currentTask() const
{
    return m_index.data(Presentation::QueryTreeModelBase::ObjectRole)
            .value<Domain::Task::Ptr>();
}

void ZanshinContext::waitForEmptyJobQueue()
{
    while (Utils::JobHandler::jobCount() != 0) {
        QTest::qWait(20);
    }
}

void ZanshinContext::waitForStableState()
{
    waitForEmptyJobQueue();
    m_monitorSpy->waitForStableState();
}

void ZanshinContext::collectIndicesImpl(const QModelIndex &root)
{
    QAbstractItemModel *model = m_model;
    for (int row = 0; row < model->rowCount(root); row++) {
        const QModelIndex index = model->index(row, 0, root);
        m_indices << index;
        if (model->rowCount(index) > 0)
            collectIndicesImpl(index);
    }
}

void ZanshinContext::collectIndices()
{
    m_indices.clear();
    collectIndicesImpl();
}

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
        return false;\
} while (0)

// Note: you should make sure that m_indices is filled in before calling this,
// e.g. calling Zanshin::collectIndices(context.get()) if not already done.
#define COMPARE_OR_DUMP(actual, expected) \
do {\
    if (!Zanshin::compare(actual, expected, #actual, #expected, __FILE__, __LINE__)) {\
        Zanshin::dumpIndices(m_indices); \
        return false;\
    }\
} while (0)

#define VERIFY(statement) \
do {\
    if (!Zanshin::verify((statement), #statement, __FILE__, __LINE__))\
        return false;\
} while (0)

// Note: you should make sure that m_indices is filled in before calling this,
// e.g. calling Zanshin::collectIndices(context.get()) if not already done.
#define VERIFY_OR_DUMP(statement) \
do {\
    if (!Zanshin::verify((statement), #statement, __FILE__, __LINE__)) {\
        Zanshin::dumpIndices(m_indices); \
        return false;\
    }\
} while (0)

#define VERIFY_OR_DO(statement, whatToDo) \
do {\
    if (!Zanshin::verify((statement), #statement, __FILE__, __LINE__)) {\
        whatToDo; \
        return false;\
    }\
} while (0)


bool ZanshinContext::I_display_the_available_data_sources()
{
    auto availableSources = m_appModel->property("availableSources").value<QObject*>();
    VERIFY(availableSources);

    auto sourceListModel = availableSources->property("sourceListModel").value<QAbstractItemModel*>();
    VERIFY(sourceListModel);

    m_presentation = availableSources;
    setModel(sourceListModel);

    return true;
}

bool ZanshinContext::I_display_the_available_pages()
{
    m_presentation = m_appModel->property("availablePages").value<QObject*>();
    setModel(m_presentation->property("pageListModel").value<QAbstractItemModel*>());
    return true;
}

bool ZanshinContext::I_display_the_page(const QString &pageName)
{
    if (m_editor) {
        // save pending changes
        VERIFY(m_editor->setProperty("task", QVariant::fromValue(Domain::Task::Ptr())));
    }
    auto availablePages = m_appModel->property("availablePages").value<QObject*>();
    VERIFY(availablePages);

    auto pageListModel = availablePages->property("pageListModel").value<QAbstractItemModel*>();
    VERIFY(pageListModel);
    waitForEmptyJobQueue();

    QModelIndex pageIndex = Zanshin::findIndex(pageListModel, pageName);
    VERIFY_OR_DUMP(pageIndex.isValid());

    QObject *page = nullptr;
    QMetaObject::invokeMethod(availablePages, "createPageForIndex",
                              Q_RETURN_ARG(QObject*, page),
                              Q_ARG(QModelIndex, pageIndex));
    VERIFY(page);

    VERIFY(m_appModel->setProperty("currentPage", QVariant::fromValue(page)));
    m_presentation = m_appModel->property("currentPage").value<QObject*>();

    return true;
}

bool ZanshinContext::there_is_an_item_in_the_central_list(const QString &taskName)
{
    auto m = m_presentation->property("centralListModel").value<QAbstractItemModel*>();
    setModel(m);
    waitForEmptyJobQueue();

    collectIndices();
    m_index = Zanshin::findIndex(model(), taskName);
    VERIFY_OR_DUMP(m_index.isValid());

    return true;
}

bool ZanshinContext::there_is_an_item_in_the_available_data_sources(const QString &sourceName)
{
    auto availableSources = m_appModel->property("availableSources").value<QObject*>();
    VERIFY(availableSources);
    auto m = availableSources->property("sourceListModel").value<QAbstractItemModel*>();
    VERIFY(m);
    waitForEmptyJobQueue();
    setModel(m);

    collectIndices();
    m_index = Zanshin::findIndex(model(), sourceName);
    VERIFY_OR_DUMP(m_index.isValid());

    return true;
}

bool ZanshinContext::the_central_list_contains_items_named(const QStringList &taskNames)
{
    m_dragIndices.clear();

    auto m = m_presentation->property("centralListModel").value<QAbstractItemModel*>();
    waitForEmptyJobQueue();
    setModel(m);

    for (const auto &taskName : taskNames) {
        QModelIndex index = Zanshin::findIndex(model(), taskName);
        VERIFY_OR_DO(index.isValid(), Zanshin::dumpIndices(m_dragIndices));
        m_dragIndices << index;
    }

    return true;
}

bool ZanshinContext::I_look_at_the_central_list()
{
    auto m = m_presentation->property("centralListModel").value<QAbstractItemModel*>();
    setModel(m);
    waitForStableState();
    return true;
}

bool ZanshinContext::I_check_the_item()
{
    VERIFY(model()->setData(m_index, Qt::Checked, Qt::CheckStateRole));
    waitForStableState();
    return true;
}

bool ZanshinContext::I_uncheck_the_item()
{
    VERIFY(model()->setData(m_index, Qt::Unchecked, Qt::CheckStateRole));
    waitForStableState();
    return true;
}

bool ZanshinContext::I_remove_the_item()
{
    VERIFY(QMetaObject::invokeMethod(m_presentation, "removeItem", Q_ARG(QModelIndex, m_index)));
    waitForStableState();
    return true;
}

bool ZanshinContext::I_promote_the_item()
{
    VERIFY(QMetaObject::invokeMethod(m_presentation, "promoteItem", Q_ARG(QModelIndex, m_index)));
    waitForStableState();

    return true;
}

bool ZanshinContext::I_add_a_project(const QString &projectName, const QString &parentSourceName)
{
    auto source = dataSourceFromName(parentSourceName);
    VERIFY(source);

    VERIFY(QMetaObject::invokeMethod(m_presentation, "addProject",
                                     Q_ARG(QString, projectName),
                                     Q_ARG(Domain::DataSource::Ptr, source)));
    waitForStableState();

    return true;
}

bool ZanshinContext::I_add_a_context(const QString &contextName, const QString &parentSourceName)
{
    auto source = dataSourceFromName(parentSourceName);
    VERIFY(source);

    VERIFY(QMetaObject::invokeMethod(m_presentation,
                                     "addContext",
                                     Q_ARG(QString, contextName),
                                     Q_ARG(Domain::DataSource::Ptr, source)));
    waitForStableState();

    return true;

}

bool ZanshinContext::I_add_a_task(const QString &taskName)
{
    waitForStableState();

    VERIFY(QMetaObject::invokeMethod(m_presentation,
                                     "addItem",
                                     Q_ARG(QString, taskName)));
    waitForStableState();

    return true;
}

bool ZanshinContext::I_rename_a_page(const QString &path, const QString &oldName, const QString &newName)
{
    const QString pageNodeName = path + " / ";

    VERIFY(!pageNodeName.isEmpty());

    auto availablePages = m_appModel->property("availablePages").value<QObject*>();
    VERIFY(availablePages);

    auto pageListModel = availablePages->property("pageListModel").value<QAbstractItemModel*>();
    VERIFY(pageListModel);
    waitForStableState();

    QModelIndex pageIndex = Zanshin::findIndex(pageListModel, pageNodeName + oldName);
    VERIFY(pageIndex.isValid());

    pageListModel->setData(pageIndex, newName);
    waitForStableState();

    return true;
}

bool ZanshinContext::I_remove_a_page(const QString &path, const QString &pageName)
{
    const QString pageNodeName = path + " / ";

    VERIFY(!pageNodeName.isEmpty());

    auto availablePages = m_appModel->property("availablePages").value<QObject*>();
    VERIFY(availablePages);

    auto pageListModel = availablePages->property("pageListModel").value<QAbstractItemModel*>();
    VERIFY(pageListModel);
    waitForStableState();

    QModelIndex pageIndex = Zanshin::findIndex(pageListModel, pageNodeName + pageName);
    VERIFY(pageIndex.isValid());

    VERIFY(QMetaObject::invokeMethod(availablePages, "removeItem",
                                     Q_ARG(QModelIndex, pageIndex)));
    waitForStableState();

    return true;
}

bool ZanshinContext::I_add_a_task_child(const QString &childName, const QString &parentName)
{
    waitForStableState();

    auto parentIndex = QModelIndex();
    for (int row = 0; row < m_indices.size(); row++) {
        auto index = m_indices.at(row);
        if (Zanshin::indexString(index) == parentName) {
            parentIndex = index;
            break;
        }
    }

    VERIFY_OR_DUMP(parentIndex.isValid());

    VERIFY(QMetaObject::invokeMethod(m_presentation,
                                     "addItem",
                                     Q_ARG(QString, childName),
                                     Q_ARG(QModelIndex, parentIndex)));
    waitForStableState();

    return true;
}

bool ZanshinContext::I_list_the_items()
{
    waitForStableState();
    collectIndices();
    waitForStableState();

    return true;
}

bool ZanshinContext::I_open_the_item_in_the_editor()
{
    auto task = currentTask();
    VERIFY(task);
    m_editor = m_appModel->property("editor").value<QObject*>();
    VERIFY(m_editor);
    VERIFY(m_editor->setProperty("task", QVariant::fromValue(task)));

    return true;
}

bool ZanshinContext::I_mark_the_item_done_in_the_editor()
{
    VERIFY(m_editor->setProperty("done", true));
    return true;
}

bool ZanshinContext::I_change_the_editor_field(const QString &field, const QVariant &value)
{
    const QByteArray property = (field == QStringLiteral("text")) ? field.toUtf8()
                              : (field == QStringLiteral("title")) ? field.toUtf8()
                              : (field == QStringLiteral("start date")) ? "startDate"
                              : (field == QStringLiteral("due date")) ? "dueDate"
                              : QByteArray();

    VERIFY(value.isValid());
    VERIFY(!property.isEmpty());

    VERIFY(m_editor->setProperty("editingInProgress", true));
    VERIFY(m_editor->setProperty(property, value));

    return true;
}

bool ZanshinContext::I_rename_the_item(const QString &taskName)
{
    VERIFY(m_editor->setProperty("editingInProgress", false));
    VERIFY(model()->setData(m_index, taskName, Qt::EditRole));
    waitForStableState();

    return true;
}

bool ZanshinContext::I_open_the_item_in_the_editor_again()
{
    auto task = currentTask();
    VERIFY(task);
    VERIFY(m_editor->setProperty("task", QVariant::fromValue(Domain::Task::Ptr())));
    VERIFY(m_editor->setProperty("task", QVariant::fromValue(task)));
    waitForStableState();

    return true;
}

bool ZanshinContext::I_drop_the_item_on_the_central_list(const QString &dropSiteName)
{
    VERIFY(m_index.isValid());
    const QMimeData *data = model()->mimeData(QModelIndexList() << m_index);

    QAbstractItemModel *destModel = model();
    QModelIndex dropIndex = Zanshin::findIndex(destModel, dropSiteName);
    VERIFY(dropIndex.isValid());
    VERIFY(destModel->dropMimeData(data, Qt::MoveAction, -1, -1, dropIndex));
    waitForStableState();

    return true;
}

bool ZanshinContext::I_drop_the_item_on_the_blank_area_of_the_central_list()
{
    VERIFY(m_index.isValid());
    const QMimeData *data = model()->mimeData(QModelIndexList() << m_index);

    QAbstractItemModel *destModel = model();
    VERIFY(destModel->dropMimeData(data, Qt::MoveAction, -1, -1, QModelIndex()));
    waitForStableState();

    return true;
}

bool ZanshinContext::I_drop_items_on_the_central_list(const QString &dropSiteName)
{
    VERIFY(!m_dragIndices.isEmpty());
    QModelIndexList indexes;
    bool allValid = true;
    std::transform(m_dragIndices.constBegin(), m_dragIndices.constEnd(),
                   std::back_inserter(indexes),
                   [&allValid] (const QPersistentModelIndex &index) {
                        allValid &= index.isValid();
                        return index;
                   });
    VERIFY(allValid);

    const QMimeData *data = model()->mimeData(indexes);

    QAbstractItemModel *destModel = model();
    QModelIndex dropIndex = Zanshin::findIndex(destModel, dropSiteName);
    VERIFY(dropIndex.isValid());
    VERIFY(destModel->dropMimeData(data, Qt::MoveAction, -1, -1, dropIndex));
    waitForStableState();

    return true;
}

bool ZanshinContext::I_drop_the_item_on_the_page_list(const QString &pageName)
{
    VERIFY(m_index.isValid());
    const QMimeData *data = model()->mimeData(QModelIndexList() << m_index);

    auto availablePages = m_appModel->property("availablePages").value<QObject*>();
    VERIFY(availablePages);

    auto destModel = availablePages->property("pageListModel").value<QAbstractItemModel*>();
    VERIFY(destModel);
    waitForStableState();

    QModelIndex dropIndex = Zanshin::findIndex(destModel, pageName);
    VERIFY(dropIndex.isValid());
    VERIFY(destModel->dropMimeData(data, Qt::MoveAction, -1, -1, dropIndex));
    waitForStableState();

    return true;
}

bool ZanshinContext::I_drop_items_on_the_page_list(const QString &pageName)
{
    VERIFY(!m_dragIndices.isEmpty());
    QModelIndexList indexes;
    bool allValid = true;
    std::transform(m_dragIndices.constBegin(), m_dragIndices.constEnd(),
                   std::back_inserter(indexes),
                   [&allValid] (const QPersistentModelIndex &index) {
                        allValid &= index.isValid();
                        return index;
                   });
    VERIFY(allValid);

    const QMimeData *data = model()->mimeData(indexes);

    auto availablePages = m_appModel->property("availablePages").value<QObject*>();
    VERIFY(availablePages);

    auto destModel = availablePages->property("pageListModel").value<QAbstractItemModel*>();
    VERIFY(destModel);
    waitForStableState();

    QModelIndex dropIndex = Zanshin::findIndex(destModel, pageName);
    VERIFY(dropIndex.isValid());
    VERIFY(destModel->dropMimeData(data, Qt::MoveAction, -1, -1, dropIndex));
    waitForStableState();

    return true;
}

bool ZanshinContext::I_change_the_setting(const QString &key, qint64 id)
{
    KConfigGroup config(KSharedConfig::openConfig(), "General");
    config.writeEntry(key, id);
    return true;
}

bool ZanshinContext::I_change_the_default_data_source(const QString &sourceName)
{
    waitForStableState();
    auto sourceIndex = Zanshin::findIndex(model(), sourceName);
    auto availableSources = m_appModel->property("availableSources").value<QObject*>();
    VERIFY(availableSources);
    VERIFY(QMetaObject::invokeMethod(availableSources, "setDefaultItem", Q_ARG(QModelIndex, sourceIndex)));
    waitForStableState();

    return true;
}

bool ZanshinContext::the_list_is(const TableData &data)
{
    auto roleNames = model()->roleNames();
    QVector<int> usedRoles;

    for (const auto &roleName : data.roles) {
        const int role = roleNames.key(roleName, -1);
        VERIFY_OR_DUMP(role != -1 && !usedRoles.contains(role));
        usedRoles << role;
    }

    QStandardItemModel inputModel;
    for (const auto &row : data.rows) {
        VERIFY_OR_DUMP(usedRoles.size() == row.size());

        QStandardItem *item = new QStandardItem;
        for (int i = 0; i < row.size(); ++i) {
            const auto role = usedRoles.at(i);
            const auto value = row.at(i);
            item->setData(value, role);
        }
        inputModel.appendRow(item);
    }

    QSortFilterProxyModel proxy;

    QAbstractItemModel *referenceModel;
    if (!qobject_cast<QSortFilterProxyModel *>(sourceModel())) {
        referenceModel = &proxy;
        proxy.setSourceModel(&inputModel);
        proxy.setSortRole(Qt::DisplayRole);
        proxy.sort(0);
        proxy.setObjectName(QStringLiteral("the_list_is_proxy"));
    } else {
        referenceModel = &inputModel;
    }

    for (int row = 0; row < m_indices.size(); row++) {
        QModelIndex expectedIndex = referenceModel->index(row, 0);
        QModelIndex resultIndex = m_indices.at(row);

        foreach (const auto &role, usedRoles) {
            COMPARE_OR_DUMP(Zanshin::indexString(resultIndex, role),
                            Zanshin::indexString(expectedIndex, role));
        }
    }
    COMPARE_OR_DUMP(m_indices.size(), referenceModel->rowCount());

    return true;
}

bool ZanshinContext::the_list_contains(const QString &itemName)
{
    for (int row = 0; row < m_indices.size(); row++) {
        if (Zanshin::indexString(m_indices.at(row)) == itemName)
            return true;
    }

    VERIFY_OR_DUMP(false);
    return false;
}

bool ZanshinContext::the_list_does_not_contain(const QString &itemName)
{
    for (int row = 0; row < m_indices.size(); row++) {
        VERIFY_OR_DUMP(Zanshin::indexString(m_indices.at(row)) != itemName);
    }

    return true;
}

bool ZanshinContext::the_task_corresponding_to_the_item_is_done()
{
    auto task = currentTask();
    VERIFY(task);
    VERIFY(task->isDone());

    return true;
}

bool ZanshinContext::the_editor_shows_the_task_as_done()
{
    VERIFY(m_editor->property("done").toBool());
    return true;
}

bool ZanshinContext::the_editor_shows_the_field(const QString &field, const QVariant &expectedValue)
{
    const QByteArray property = (field == QStringLiteral("text")) ? field.toUtf8()
                              : (field == QStringLiteral("title")) ? field.toUtf8()
                              : (field == QStringLiteral("start date")) ? "startDate"
                              : (field == QStringLiteral("due date")) ? "dueDate"
                              : QByteArray();

    VERIFY(expectedValue.isValid());
    VERIFY(!property.isEmpty());

    COMPARE(m_editor->property(property), expectedValue);

    return true;
}

bool ZanshinContext::the_default_data_source_is(const QString &expectedName)
{
    waitForStableState();
    auto expectedIndex = Zanshin::findIndex(model(), expectedName);
    VERIFY(expectedIndex.isValid());
    auto defaultRole = model()->roleNames().key("default", -1);
    VERIFY(expectedIndex.data(defaultRole).toBool());

    return true;
}

bool ZanshinContext::the_setting_is(const QString &key, qint64 expectedId)
{
    KConfigGroup config(KSharedConfig::openConfig(), "General");
    const qint64 id = config.readEntry(key, -1);
    COMPARE(id, expectedId);

    return true;
}

Domain::DataSource::Ptr ZanshinContext::dataSourceFromName(const QString &sourceName)
{
    auto availableSources = m_appModel->property("availableSources").value<QObject*>();
    if (!availableSources)
        return nullptr;
    auto sourceList = availableSources->property("sourceListModel").value<QAbstractItemModel*>();
    if (!sourceList)
        return nullptr;
    waitForStableState();
    QModelIndex index = Zanshin::findIndex(sourceList, sourceName);
    if (!index.isValid()) {
        qWarning() << "source" << sourceName << "not found.";
        for (int row = 0; row < sourceList->rowCount(); row++) {
            qDebug() << sourceList->index(row, 0).data().toString();
        }
        return nullptr;
    }
    return index.data(Presentation::QueryTreeModelBase::ObjectRole)
                .value<Domain::DataSource::Ptr>();
}
