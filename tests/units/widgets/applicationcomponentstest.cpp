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

#include <QtTestGui>

#include <QMimeData>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QTreeView>
#include <QWidgetAction>

#include <algorithm>

#include "utils/mem_fn.h"

#include "domain/note.h"
#include "domain/task.h"

#include "presentation/artifactfilterproxymodel.h"
#include "presentation/querytreemodelbase.h"

#include "widgets/applicationcomponents.h"
#include "widgets/availablepagesview.h"
#include "widgets/availablesourcesview.h"
#include "widgets/editorview.h"
#include "widgets/filterwidget.h"
#include "widgets/pageview.h"
#include "widgets/quickselectdialog.h"


class CustomModelStub : public QStandardItemModel
{
    QMimeData *mimeData(const QModelIndexList &indexes) const
    {
        QStringList dataString;
        std::transform(indexes.begin(), indexes.end(),
                       std::back_inserter(dataString),
                       [] (const QModelIndex &index) {
                           return index.data().toString();
                       });

        auto data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(dataString));

        return data;
    }

    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &destination)
    {
        Q_UNUSED(action);

        Q_ASSERT(row == -1);
        Q_ASSERT(column == -1);
        Q_ASSERT(destination.isValid());
        Q_ASSERT(data->hasFormat("application/x-zanshin-object"));

        auto dataString = data->property("objects").value<QStringList>();
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

    explicit ApplicationModelStub(QObject *parent = Q_NULLPTR)
        : QObject(parent), m_currentPage(Q_NULLPTR) {}

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
    explicit AvailablePagesModelStub(QObject *parent = Q_NULLPTR)
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
        model->setStringList(QStringList() << "Items" << "from" << index.data().toString());
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

    template<typename T>
    void addItem(const QString &title)
    {
        auto artifact = T::Ptr::create();
        artifact->setTitle(title);
        addItem(artifact);
    }

    void addItem(const Domain::Artifact::Ptr &artifact)
    {
        QStandardItem *item = new QStandardItem;
        item->setData(QVariant::fromValue(artifact), Presentation::QueryTreeModelBase::ObjectRole);
        item->setData(artifact->title(), Qt::DisplayRole);
        itemModel.appendRow(item);
    }

    Domain::Artifact::Ptr itemAtRow(int row) const
    {
        return itemModel.index(row, 0).data(Presentation::QueryTreeModelBase::ObjectRole)
                                      .value<Domain::Artifact::Ptr>();
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
    explicit EditorModelStub(QObject *parent = Q_NULLPTR)
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
            emit startDateChanged(value.toDateTime());
        else if (name == "dueDate")
            emit dueDateChanged(value.toDateTime());
        else if (name == "hasTaskProperties")
            emit hasTaskPropertiesChanged(value.toBool());
        else
            qFatal("Unsupported property %s", name.constData());
    }

public slots:
    void setTitle(const QString &title) { setPropertyAndSignal("title", title); }
    void setText(const QString &text) { setPropertyAndSignal("text", text); }
    void setDone(bool done) { setPropertyAndSignal("done", done); }
    void setStartDate(const QDateTime &start) { setPropertyAndSignal("startDate", start); }
    void setDueDate(const QDateTime &due) { setPropertyAndSignal("dueDate", due); }

signals:
    void hasTaskPropertiesChanged(bool hasTaskProperties);
    void textChanged(const QString &text);
    void titleChanged(const QString &title);
    void doneChanged(bool done);
    void startDateChanged(const QDateTime &date);
    void dueDateChanged(const QDateTime &due);
};

class QuickSelectDialogStub : public Widgets::QuickSelectDialogInterface
{
public:
    typedef QSharedPointer<QuickSelectDialogStub> Ptr;

    explicit QuickSelectDialogStub()
        : parent(Q_NULLPTR),
          execCount(0),
          itemModel(Q_NULLPTR)
    {
    }

    int exec()
    {
        execCount++;
        return QDialog::Accepted;
    }

    void setModel(QAbstractItemModel *model) Q_DECL_OVERRIDE
    {
        itemModel = model;
    }

    QPersistentModelIndex selectedIndex() const Q_DECL_OVERRIDE
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
    explicit ApplicationComponentsTest(QObject *parent = Q_NULLPTR)
        : QObject(parent)
    {
        qputenv("ZANSHIN_UNIT_TEST_RUN", "1");
    }

private slots:
    void shouldHaveApplicationModel()
    {
        // GIVEN
        Widgets::ApplicationComponents components;
        auto model = QObjectPtr::create();

        // WHEN
        components.setModel(model);

        // THEN
        QCOMPARE(components.model(), model);
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

    void shouldApplyEditorModelAltoToCreatedPageView()
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

        components.setModel(model);

        // WHEN
        components.setModel(QObjectPtr());
        components.availableSourcesView();
        components.availablePagesView();
        components.pageView();
        components.editorView();

        // THEN
        QVERIFY(!components.availableSourcesView()->model());
        QVERIFY(!components.availablePagesView()->model());
        QVERIFY(!components.pageView()->model());
        QVERIFY(!components.editorView()->model());
    }

    void shouldPropageNullModelsToCreatedViews()
    {
        // GIVEN
        Widgets::ApplicationComponents components;
        components.availableSourcesView();
        components.availablePagesView();
        components.pageView();
        components.editorView();

        auto model = QObjectPtr::create();
        auto availableSources = new QObject(model.data());
        model->setProperty("availableSources", QVariant::fromValue(availableSources));
        auto availablePages = new QObject(model.data());
        model->setProperty("availablePages", QVariant::fromValue(availablePages));
        auto currentPage = new QObject(model.data());
        model->setProperty("currentPage", QVariant::fromValue(currentPage));
        auto editorModel = new EditorModelStub(model.data());
        model->setProperty("editor", QVariant::fromValue<QObject*>(editorModel));

        components.setModel(model);

        // WHEN
        components.setModel(QObjectPtr());

        // THEN
        QVERIFY(!components.availableSourcesView()->model());
        QVERIFY(!components.availablePagesView()->model());
        QVERIFY(!components.pageView()->model());
        QVERIFY(!components.editorView()->model());
    }

    void shouldApplyAvailablePagesSelectionToApplicationModel()
    {
        // GIVEN
        auto model = ApplicationModelStub::Ptr::create();

        AvailablePagesModelStub availablePagesModel;
        model->setProperty("availablePages", QVariant::fromValue<QObject*>(&availablePagesModel));
        model->setProperty("currentPage", QVariant::fromValue<QObject*>(Q_NULLPTR));

        QObject editorModel;
        editorModel.setProperty("artifact",
                                QVariant::fromValue<Domain::Artifact::Ptr>(Domain::Task::Ptr::create()));
        model->setProperty("editor", QVariant::fromValue<QObject*>(&editorModel));

        Widgets::ApplicationComponents components;
        components.setModel(model);

        Widgets::AvailablePagesView *availablePagesView = components.availablePagesView();
        auto pagesView = availablePagesView->findChild<QTreeView*>("pagesView");
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
        QVERIFY(editorModel.property("artifact").value<Domain::Artifact::Ptr>().isNull());
    }

    void shouldApplyPageViewSelectionToEditorModel()
    {
        // GIVEN
        auto model = QObjectPtr::create();

        PageModelStub pageModel;
        pageModel.addItem<Domain::Task>("0. First task");
        pageModel.addItem<Domain::Note>("1. A note");
        pageModel.addItem<Domain::Task>("2. Second task");
        pageModel.addItem<Domain::Note>("3. Another note");
        model->setProperty("currentPage", QVariant::fromValue<QObject*>(&pageModel));

        EditorModelStub editorModel;
        model->setProperty("editor", QVariant::fromValue<QObject*>(&editorModel));

        Widgets::ApplicationComponents components;
        components.setModel(model);

        Widgets::PageView *pageView = components.pageView();
        auto centralView = pageView->findChild<QTreeView*>("centralView");
        QModelIndex index = centralView->model()->index(2, 0);

        // WHEN
        centralView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);

        // THEN
        QCOMPARE(editorModel.property("artifact").value<Domain::Artifact::Ptr>(),
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
        foreach (const auto &key,  available->globalActions().keys())
            QCOMPARE(actions.value(key), available->globalActions().value(key));

        // availableSources view
        auto availableSources = components.availableSourcesView();
        foreach (const auto &key, availableSources->globalActions().keys())
            QCOMPARE(actions.value(key), availableSources->globalActions().value(key));

        // page view
        auto page = components.pageView();
        foreach (const auto &key, page->globalActions().keys())
            QCOMPARE(actions.value(key), page->globalActions().value(key));

        // application component own action
        auto moveAction = components.findChild<QAction*>("moveItemAction");
        QCOMPARE(actions.value("page_view_move"), moveAction);
    }

    void shouldMoveItem()
    {
        // GIVEN
        auto model = QObjectPtr::create();

        PageModelStub pageModel;
        pageModel.addItem<Domain::Task>("0. First task");
        pageModel.addItem<Domain::Note>("1. A note");
        pageModel.addItem<Domain::Task>("2. Second task");
        pageModel.addItem<Domain::Note>("3. Another note");
        model->setProperty("currentPage", QVariant::fromValue<QObject*>(&pageModel));

        AvailablePagesModelStub availablePagesModelStub;
        model->setProperty("availablePages", QVariant::fromValue<QObject*>(&availablePagesModelStub));

        QWidget *mainWidget = new QWidget;
        Widgets::ApplicationComponents components(mainWidget);
        components.setModel(model);

        auto availablePageView = components.availablePagesView();
        auto availablePagesTreeView = availablePageView->findChild<QTreeView*>("pagesView");
        Q_ASSERT(availablePagesTreeView);

        auto dialogStub = QuickSelectDialogStub::Ptr::create();
        dialogStub->index = availablePagesModelStub.pageListModel()->index(0, 0); // inbox selected
        components.setQuickSelectDialogFactory([dialogStub] (QWidget *parent) {
            dialogStub->parent = parent;
            return dialogStub;
        });

        auto pageView = components.pageView();
        auto centralView = pageView->findChild<QTreeView*>("centralView");
        QModelIndex index1 = pageModel.itemModel.index(0,0);
        QModelIndex index2 = pageModel.itemModel.index(2,0);

        auto filterWidget = pageView->findChild<Widgets::FilterWidget*>("filterWidget");
        auto displayedModel = filterWidget->proxyModel();
        auto displayedIndex = displayedModel->index(0, 0);
        auto displayedIndex2 = displayedModel->index(2, 0);

        auto moveAction = components.findChild<QAction*>("moveItemAction");

        // WHEN
        pageModel.selectedItems << index1 << index2;
        centralView->selectionModel()->setCurrentIndex(displayedIndex, QItemSelectionModel::ClearAndSelect);
        centralView->selectionModel()->setCurrentIndex(displayedIndex2, QItemSelectionModel::Select);

        moveAction->trigger();

        // THEN
        QCOMPARE(dialogStub->execCount, 1);
        QCOMPARE(dialogStub->parent, pageView);
        QCOMPARE(dialogStub->itemModel, availablePagesModelStub.pageListModel());

        QCOMPARE(availablePagesModelStub.itemModel.dropDestination, QString("Inbox"));
        QCOMPARE(availablePagesModelStub.itemModel.droppedItemDataString.size(), 2);
        QCOMPARE(availablePagesModelStub.itemModel.droppedItemDataString.at(0), index1.data().toString());
        QCOMPARE(availablePagesModelStub.itemModel.droppedItemDataString.at(1), index2.data().toString());
    }
};

QTEST_MAIN(ApplicationComponentsTest)

#include "applicationcomponentstest.moc"
