/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_gui_zanshin.h>

#include <QMimeData>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QTreeView>
#include <QWidgetAction>

#include <algorithm>

#include "utils/mem_fn.h"

#include "domain/task.h"

#include "presentation/taskfilterproxymodel.h"
#include "presentation/querytreemodelbase.h"
#include "presentation/runningtaskmodelinterface.h"

#include "widgets/applicationcomponents.h"
#include "widgets/availablepagesview.h"
#include "widgets/availablesourcesview.h"
#include "widgets/editorview.h"
#include "widgets/filterwidget.h"
#include "widgets/pageview.h"
#include "widgets/pageviewerrorhandler.h"
#include "widgets/quickselectdialog.h"
#include "widgets/runningtaskwidget.h"


class CustomModelStub : public QStandardItemModel
{
    Q_OBJECT

    QMimeData *mimeData(const QModelIndexList &indexes) const override
    {
        QStringList dataString;
        std::transform(indexes.begin(), indexes.end(),
                       std::back_inserter(dataString),
                       [] (const QModelIndex &index) {
                           return index.data().toString();
                       });

        auto data = new QMimeData;
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(dataString));

        return data;
    }

    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &destination) override
    {
        Q_UNUSED(action);

        Q_ASSERT(row == -1);
        Q_ASSERT(column == -1);
        Q_ASSERT(destination.isValid());
        Q_ASSERT(data->hasFormat(QStringLiteral("application/x-zanshin-object")));

        auto dataString = data->property("objects").toStringList();
        Q_ASSERT(!dataString.isEmpty());

        droppedItemDataString = dataString;
        dropDestination = destination.data().toString();

        return true;
    }

public:
    QStringList droppedItemDataString;
    QString dropDestination;
};

class ApplicationModelStub : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* currentPage READ currentPage WRITE setCurrentPage)
public:
    typedef QSharedPointer<ApplicationModelStub> Ptr;

    explicit ApplicationModelStub(QObject *parent = nullptr)
        : QObject(parent), m_currentPage(nullptr) {}

    QObject *currentPage()
    {
        return m_currentPage;
    }

    void setCurrentPage(QObject *page)
    {
        if (page == m_currentPage)
            return;

        m_currentPage = page;
        emit currentPageChanged(m_currentPage);
    }

signals:
    void currentPageChanged(QObject *page);

private:
    QObject *m_currentPage;
};

class AvailablePagesModelStub : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* pageListModel READ pageListModel)
public:
    explicit AvailablePagesModelStub(QObject *parent = nullptr)
        : QObject(parent)
    {
        QStandardItem *inbox = new QStandardItem;
        inbox->setData("Inbox", Qt::DisplayRole);
        itemModel.appendRow(inbox);

        QStandardItem *project = new QStandardItem;
        project->setData("Project", Qt::DisplayRole);
        itemModel.appendRow(project);
    }

    QAbstractItemModel *pageListModel()
    {
        return &itemModel;
    }

    Q_SCRIPTABLE QObject *createPageForIndex(const QModelIndex &index)
    {
        auto page = new QObject(this);
        auto model = new QStringListModel(page);
        model->setStringList(QStringList() << QStringLiteral("Items") << QStringLiteral("from") << index.data().toString());
        page->setProperty("centralListModel",
                          QVariant::fromValue<QAbstractItemModel*>(model));
        createdPages << page;
        return page;
    }

public:
    QList<QObject*> createdPages;
    CustomModelStub itemModel;
};

class PageModelStub : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* centralListModel READ centralListModel)
public:
    QAbstractItemModel *centralListModel()
    {
        return &itemModel;
    }

    void addTask(const QString &title)
    {
        auto task = Domain::Task::Ptr::create();
        task->setTitle(title);
        addTask(task);
    }

    void addTask(const Domain::Task::Ptr &task)
    {
        QStandardItem *item = new QStandardItem;
        item->setData(QVariant::fromValue(task), Presentation::QueryTreeModelBase::ObjectRole);
        item->setData(task->title(), Qt::DisplayRole);
        itemModel.appendRow(item);
    }

    Domain::Task::Ptr itemAtRow(int row) const
    {
        return itemModel.index(row, 0).data(Presentation::QueryTreeModelBase::ObjectRole)
                                      .value<Domain::Task::Ptr>();
    }

    QModelIndexList selectedIndexes() const
    {
        return selectedItems;
    }

public:
    QModelIndexList selectedItems;
    CustomModelStub itemModel;
};

class EditorModelStub : public QObject
{
    Q_OBJECT
public:
    explicit EditorModelStub(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    void setPropertyAndSignal(const QByteArray &name, const QVariant &value)
    {
        if (property(name) == value)
            return;

        setProperty(name, value);
        if (name == "text")
            emit textChanged(value.toString());
        else if (name == "title")
            emit titleChanged(value.toString());
        else if (name == "done")
            emit doneChanged(value.toBool());
        else if (name == "startDate")
            emit startDateChanged(value.toDate());
        else if (name == "dueDate")
            emit dueDateChanged(value.toDate());
        else
            qFatal("Unsupported property %s", name.constData());
    }

public slots:
    void setTitle(const QString &title) { setPropertyAndSignal("title", title); }
    void setText(const QString &text) { setPropertyAndSignal("text", text); }
    void setDone(bool done) { setPropertyAndSignal("done", done); }
    void setStartDate(const QDate &start) { setPropertyAndSignal("startDate", start); }
    void setDueDate(const QDate &due) { setPropertyAndSignal("dueDate", due); }

signals:
    void textChanged(const QString &text);
    void titleChanged(const QString &title);
    void doneChanged(bool done);
    void startDateChanged(const QDate &date);
    void dueDateChanged(const QDate &due);
};

class RunningTaskModelStub : public Presentation::RunningTaskModelInterface
{
    Q_OBJECT
public:
    explicit RunningTaskModelStub(QObject *parent = nullptr)
        : Presentation::RunningTaskModelInterface(parent)
    {
    }

    Domain::Task::Ptr runningTask() const override { return {}; }
    void setRunningTask(const Domain::Task::Ptr &) override {}
    void taskDeleted(const Domain::Task::Ptr &) override {}
    void stopTask() override {}
    void doneTask() override {}
};

class QuickSelectDialogStub : public Widgets::QuickSelectDialogInterface
{
public:
    typedef QSharedPointer<QuickSelectDialogStub> Ptr;

    explicit QuickSelectDialogStub()
        : parent(nullptr),
          execCount(0),
          itemModel(nullptr)
    {
    }

    int exec() override
    {
        execCount++;
        return QDialog::Accepted;
    }

    void setModel(QAbstractItemModel *model) override
    {
        itemModel = model;
    }

    QPersistentModelIndex selectedIndex() const override
    {
        return index;
    }

    QWidget *parent;
    int execCount;
    QAbstractItemModel *itemModel;
    QPersistentModelIndex index;
};

class ApplicationComponentsTest : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationComponentsTest(QObject *parent = nullptr)
        : QObject(parent)
    {
        qputenv("ZANSHIN_UNIT_TEST_RUN", "1");
    }

private slots:
    void shouldHaveApplicationModelAndSetErrorHandler()
    {
        // GIVEN
        Widgets::ApplicationComponents components;
        auto model = QObjectPtr::create();

        // WHEN
        components.setModel(model);

        // THEN
        QCOMPARE(components.model(), model);
        auto errorHandlerBase = model->property("errorHandler").value<Presentation::ErrorHandler*>();
        QVERIFY(errorHandlerBase);
        auto errorHandler = static_cast<Widgets::PageViewErrorHandler*>(errorHandlerBase);
        QVERIFY(errorHandler);
        QVERIFY(!errorHandler->pageView());

        // WHEN
        auto pageView = components.pageView();

        // THEN
        QCOMPARE(errorHandler->pageView(), pageView);
    }

    void shouldApplyAvailableSourcesModelToAvailableSourcesView()
    {
        // GIVEN
        Widgets::ApplicationComponents components;
        auto model = QObjectPtr::create();
        QObject availableSources;
        model->setProperty("availableSources", QVariant::fromValue(&availableSources));

        // WHEN
        components.setModel(model);

        // THEN
        QCOMPARE(components.availableSourcesView()->model(), &availableSources);
    }

    void shouldApplyAvailableSourcesModelAlsoToCreatedAvailableSourcesView()
    {
        // GIVEN
        Widgets::ApplicationComponents components;
        // Force creation
        components.availableSourcesView();

        auto model = QObjectPtr::create();
        QObject availableSources;
        model->setProperty("availableSources", QVariant::fromValue(&availableSources));

        // WHEN
        components.setModel(model);

        // THEN
        QCOMPARE(components.availableSourcesView()->model(), &availableSources);
    }

    void shouldApplyAvailablePagesModelToAvailablePagesView()
    {
        // GIVEN
        Widgets::ApplicationComponents components;
        auto model = QObjectPtr::create();

        QObject availablePages;
        model->setProperty("availablePages", QVariant::fromValue(&availablePages));

        QObject availableSources;
        QAbstractItemModel *sourcesModel = new QStandardItemModel(model.data());
        availableSources.setProperty("sourceListModel", QVariant::fromValue(sourcesModel));
        model->setProperty("availableSources", QVariant::fromValue(&availableSources));

        // WHEN
        components.setModel(model);

        // THEN
        QCOMPARE(components.availablePagesView()->model(), &availablePages);
        QCOMPARE(components.availablePagesView()->projectSourcesModel(), sourcesModel);
    }

    void shouldApplyAvailablePagesModelAlsoToCreatedAvailablePagesView()
    {
        // GIVEN
        Widgets::ApplicationComponents components;
        // Force creation
        components.availablePagesView();

        auto model = QObjectPtr::create();
        QObject availablePages;
        QAbstractItemModel *sourcesModel = new QStandardItemModel(model.data());
        model->setProperty("dataSourcesModel", QVariant::fromValue(sourcesModel));
        model->setProperty("availablePages", QVariant::fromValue(&availablePages));

        // WHEN
        components.setModel(model);

        // THEN
        QCOMPARE(components.availablePagesView()->model(), &availablePages);
        QCOMPARE(components.availablePagesView()->projectSourcesModel(), sourcesModel);
    }

    void shouldApplyCurrentPageModelToPageView()
    {
        // GIVEN
        Widgets::ApplicationComponents components;
        auto model = QObjectPtr::create();
        QObject currentPage;
        model->setProperty("currentPage", QVariant::fromValue(&currentPage));

        // WHEN
        components.setModel(model);

        // THEN
        QCOMPARE(components.pageView()->model(), &currentPage);
    }

    void shouldApplyCurrentPageModelAlsoToCreatedPageView()
    {
        // GIVEN
        Widgets::ApplicationComponents components;
        // Force creation
        components.pageView();

        auto model = QObjectPtr::create();
        QObject currentPage;
        model->setProperty("currentPage", QVariant::fromValue(&currentPage));

        // WHEN
        components.setModel(model);

        // THEN
        QCOMPARE(components.pageView()->model(), &currentPage);
    }

    void shouldApplyRunningTaskModelToPageView()
    {
        // GIVEN
        Widgets::ApplicationComponents components;
        auto model = QObjectPtr::create();
        auto runningTaskModel = new RunningTaskModelStub(model.data());
        model->setProperty("runningTaskModel", QVariant::fromValue<Presentation::RunningTaskModelInterface*>(runningTaskModel));

        // WHEN
        components.setModel(model);

        // THEN
        QCOMPARE(components.pageView()->runningTaskModel(), runningTaskModel);
    }

    void shouldApplyRunningTaskModelAlsoToCreatedPageView()
    {
        // GIVEN
        Widgets::ApplicationComponents components;
        // Force creation
        components.pageView();

        auto model = QObjectPtr::create();
        auto runningTaskModel = new RunningTaskModelStub(model.data());
        model->setProperty("runningTaskModel", QVariant::fromValue<Presentation::RunningTaskModelInterface*>(runningTaskModel));

        // WHEN
        components.setModel(model);

        // THEN
        QCOMPARE(components.pageView()->runningTaskModel(), runningTaskModel);
    }

    void shouldApplyEditorModelToEditorView()
    {
        // GIVEN
        Widgets::ApplicationComponents components;
        auto model = QObjectPtr::create();
        QObject *editorModel = new EditorModelStub(model.data());
        model->setProperty("editor", QVariant::fromValue(editorModel));

        // WHEN
        components.setModel(model);

        // THEN
        QCOMPARE(components.editorView()->model(), editorModel);
    }

    void shouldApplyEditorModelAlsoToCreatedPageView()
    {
        // GIVEN
        Widgets::ApplicationComponents components;
        // Force creation
        components.editorView();

        auto model = QObjectPtr::create();
        QObject *editorModel = new EditorModelStub(model.data());
        model->setProperty("editor", QVariant::fromValue(editorModel));

        // WHEN
        components.setModel(model);

        // THEN
        QCOMPARE(components.editorView()->model(), editorModel);
    }

    void shouldApplyRunningTaskModelToRunningTaskView()
    {
        // GIVEN
        Widgets::ApplicationComponents components;
        auto model = QObjectPtr::create();
        auto runningTaskModel = new RunningTaskModelStub(model.data());
        model->setProperty("runningTaskModel", QVariant::fromValue<Presentation::RunningTaskModelInterface*>(runningTaskModel));

        // WHEN
        components.setModel(model);

        // THEN
        QCOMPARE(components.runningTaskView()->model(), runningTaskModel);
    }

    void shouldApplyRunningTaskModelAlsoToCreatedRunningTaskView()
    {
        // GIVEN
        Widgets::ApplicationComponents components;
        // Force creation
        components.runningTaskView();

        auto model = QObjectPtr::create();
        auto runningTaskModel = new RunningTaskModelStub(model.data());
        model->setProperty("runningTaskModel", QVariant::fromValue<Presentation::RunningTaskModelInterface*>(runningTaskModel));

        // WHEN
        components.setModel(model);

        // THEN
        QCOMPARE(components.runningTaskView()->model(), runningTaskModel);
    }


    void shouldPropageNullModelsToViews()
    {
        // GIVEN
        Widgets::ApplicationComponents components;

        auto model = QObjectPtr::create();
        auto availableSources = new QObject(model.data());
        model->setProperty("availableSources", QVariant::fromValue(availableSources));
        auto availablePages = new QObject(model.data());
        model->setProperty("availablePages", QVariant::fromValue(availablePages));
        auto currentPage = new QObject(model.data());
        model->setProperty("currentPage", QVariant::fromValue(currentPage));
        auto editorModel = new EditorModelStub(model.data());
        model->setProperty("editor", QVariant::fromValue<QObject*>(editorModel));
        auto runningTaskModel = new RunningTaskModelStub(model.data());
        model->setProperty("runningTaskModel", QVariant::fromValue<Presentation::RunningTaskModelInterface*>(runningTaskModel));

        components.setModel(model);

        // WHEN
        components.setModel(QObjectPtr());
        components.availableSourcesView();
        components.availablePagesView();
        components.pageView();
        components.editorView();
        components.runningTaskView();

        // THEN
        QVERIFY(!components.availableSourcesView()->model());
        QVERIFY(!components.availablePagesView()->model());
        QVERIFY(!components.pageView()->model());
        QVERIFY(!components.editorView()->model());
        QVERIFY(!components.runningTaskView()->model());
    }

    void shouldPropageNullModelsToCreatedViews()
    {
        // GIVEN
        Widgets::ApplicationComponents components;
        components.availableSourcesView();
        components.availablePagesView();
        components.pageView();
        components.editorView();
        components.runningTaskView();

        auto model = QObjectPtr::create();
        auto availableSources = new QObject(model.data());
        model->setProperty("availableSources", QVariant::fromValue(availableSources));
        auto availablePages = new QObject(model.data());
        model->setProperty("availablePages", QVariant::fromValue(availablePages));
        auto currentPage = new QObject(model.data());
        model->setProperty("currentPage", QVariant::fromValue(currentPage));
        auto editorModel = new EditorModelStub(model.data());
        model->setProperty("editor", QVariant::fromValue<QObject*>(editorModel));
        auto runningTaskModel = new RunningTaskModelStub(model.data());
        model->setProperty("runningTaskModel", QVariant::fromValue<Presentation::RunningTaskModelInterface*>(runningTaskModel));

        components.setModel(model);

        // WHEN
        components.setModel(QObjectPtr());

        // THEN
        QVERIFY(!components.availableSourcesView()->model());
        QVERIFY(!components.availablePagesView()->model());
        QVERIFY(!components.pageView()->model());
        QVERIFY(!components.editorView()->model());
        QVERIFY(!components.runningTaskView()->model());
    }

    void shouldApplyAvailablePagesSelectionToApplicationModel()
    {
        // GIVEN
        auto model = ApplicationModelStub::Ptr::create();

        AvailablePagesModelStub availablePagesModel;
        model->setProperty("availablePages", QVariant::fromValue<QObject*>(&availablePagesModel));
        model->setProperty("currentPage", QVariant::fromValue<QObject*>(nullptr));

        QObject editorModel;
        editorModel.setProperty("task",
                                QVariant::fromValue(Domain::Task::Ptr::create()));
        model->setProperty("editor", QVariant::fromValue<QObject*>(&editorModel));

        Widgets::ApplicationComponents components;
        components.setModel(model);

        Widgets::AvailablePagesView *availablePagesView = components.availablePagesView();
        auto pagesView = availablePagesView->findChild<QTreeView*>(QStringLiteral("pagesView"));
        QVERIFY(pagesView);

        Widgets::PageView *pageView = components.pageView();
        QVERIFY(pageView);
        QVERIFY(!pageView->model());

        QModelIndex index = pagesView->model()->index(0, 0);

        // WHEN
        pagesView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);

        // THEN
        QCOMPARE(availablePagesModel.createdPages.size(), 1);
        QCOMPARE(model->property("currentPage").value<QObject*>(),
                 availablePagesModel.createdPages.first());
        QCOMPARE(pageView->model(),
                 availablePagesModel.createdPages.first());
        QVERIFY(editorModel.property("task").value<Domain::Task::Ptr>().isNull());
    }

    void shouldApplyPageViewSelectionToEditorModel()
    {
        // GIVEN
        auto model = QObjectPtr::create();

        PageModelStub pageModel;
        pageModel.addTask(QStringLiteral("0. First task"));
        pageModel.addTask(QStringLiteral("1. Second task"));
        pageModel.addTask(QStringLiteral("2. Third task"));
        pageModel.addTask(QStringLiteral("3. Yet another task"));
        model->setProperty("currentPage", QVariant::fromValue<QObject*>(&pageModel));

        EditorModelStub editorModel;
        model->setProperty("editor", QVariant::fromValue<QObject*>(&editorModel));

        Widgets::ApplicationComponents components;
        components.setModel(model);

        Widgets::PageView *pageView = components.pageView();
        auto centralView = pageView->findChild<QTreeView*>(QStringLiteral("centralView"));
        QModelIndex index = centralView->model()->index(2, 0);

        // WHEN
        centralView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);

        // THEN
        QCOMPARE(editorModel.property("task").value<Domain::Task::Ptr>(),
                 pageModel.itemAtRow(index.row()));
    }

    void shouldHaveDefaultActionsList()
    {
        // GIVEN
        Widgets::ApplicationComponents components;

        // WHEN
        auto actions = components.globalActions();

        // THEN

        // availablePages view
        auto available = components.availablePagesView();
        auto availableGlobalActions = available->globalActions();
        for (auto it = availableGlobalActions.cbegin(); it != availableGlobalActions.cend(); ++it)
            QCOMPARE(actions.value(it.key()), it.value());

        // availableSources view
        auto availableSources = components.availableSourcesView();
        auto availableSourcesGlobalActions = availableSources->globalActions();
        for (auto it = availableSourcesGlobalActions.cbegin(); it != availableSourcesGlobalActions.cend(); ++it)
            QCOMPARE(actions.value(it.key()), it.value());

        // page view
        auto page = components.pageView();
        auto pageGlobalActions = page->globalActions();
        for (auto it = pageGlobalActions.cbegin(); it != pageGlobalActions.cend(); ++it)
            QCOMPARE(actions.value(it.key()), it.value());

        // application component own action
        auto moveAction = components.findChild<QAction*>(QStringLiteral("moveItemAction"));
        QCOMPARE(actions.value(QStringLiteral("page_view_move")), moveAction);
    }

    void shouldMoveItem()
    {
        // GIVEN
        auto model = QObjectPtr::create();

        PageModelStub pageModel;
        pageModel.addTask(QStringLiteral("0. First task"));
        pageModel.addTask(QStringLiteral("1. Second task"));
        pageModel.addTask(QStringLiteral("2. Third task"));
        pageModel.addTask(QStringLiteral("3. Yet another task"));
        model->setProperty("currentPage", QVariant::fromValue<QObject*>(&pageModel));

        AvailablePagesModelStub availablePagesModelStub;
        model->setProperty("availablePages", QVariant::fromValue<QObject*>(&availablePagesModelStub));

        QWidget *mainWidget = new QWidget;
        Widgets::ApplicationComponents components(mainWidget);
        components.setModel(model);

        auto availablePageView = components.availablePagesView();
        auto availablePagesTreeView = availablePageView->findChild<QTreeView*>(QStringLiteral("pagesView"));
        Q_ASSERT(availablePagesTreeView);

        auto dialogStub = QuickSelectDialogStub::Ptr::create();
        dialogStub->index = availablePagesModelStub.pageListModel()->index(0, 0); // inbox selected
        components.setQuickSelectDialogFactory([dialogStub] (QWidget *parent) {
            dialogStub->parent = parent;
            return dialogStub;
        });

        auto pageView = components.pageView();
        auto centralView = pageView->findChild<QTreeView*>(QStringLiteral("centralView"));
        QModelIndex index1 = pageModel.itemModel.index(0,0);
        QModelIndex index2 = pageModel.itemModel.index(2,0);

        auto filterWidget = pageView->findChild<Widgets::FilterWidget*>(QStringLiteral("filterWidget"));
        auto displayedModel = filterWidget->proxyModel();
        auto displayedIndex = displayedModel->index(0, 0);
        auto displayedIndex2 = displayedModel->index(2, 0);

        auto moveAction = components.findChild<QAction*>(QStringLiteral("moveItemAction"));

        // WHEN
        pageModel.selectedItems << index1 << index2;
        centralView->selectionModel()->setCurrentIndex(displayedIndex, QItemSelectionModel::ClearAndSelect);
        centralView->selectionModel()->setCurrentIndex(displayedIndex2, QItemSelectionModel::Select);

        moveAction->trigger();

        // THEN
        QCOMPARE(dialogStub->execCount, 1);
        QCOMPARE(dialogStub->parent, pageView);
        QCOMPARE(dialogStub->itemModel, availablePagesModelStub.pageListModel());

        QCOMPARE(availablePagesModelStub.itemModel.dropDestination, QStringLiteral("Inbox"));
        QCOMPARE(availablePagesModelStub.itemModel.droppedItemDataString.size(), 2);
        QCOMPARE(availablePagesModelStub.itemModel.droppedItemDataString.at(0), index1.data().toString());
        QCOMPARE(availablePagesModelStub.itemModel.droppedItemDataString.at(1), index2.data().toString());
    }
};

ZANSHIN_TEST_MAIN(ApplicationComponentsTest)

#include "applicationcomponentstest.moc"
