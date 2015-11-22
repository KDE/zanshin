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

#include <QAbstractItemModel>
#include <QAction>
#include <QDebug>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QToolBar>
#include <QTreeView>

#include "domain/project.h"
#include "domain/context.h"
#include "domain/tag.h"

#include "presentation/metatypes.h"
#include "presentation/querytreemodelbase.h"

#include "widgets/availablepagesview.h"
#include "widgets/newpagedialog.h"

#include "messageboxstub.h"

class NewPageDialogStub : public Widgets::NewPageDialogInterface
{
public:
    typedef QSharedPointer<NewPageDialogStub> Ptr;

    explicit NewPageDialogStub()
        : parent(Q_NULLPTR),
          execCount(0),
          sourceModel(Q_NULLPTR),
          source(Domain::DataSource::Ptr::create())
    {
    }

    int exec()
    {
        execCount++;
        return QDialog::Accepted;
    }

    void setDataSourcesModel(QAbstractItemModel *model) Q_DECL_OVERRIDE
    {
        sourceModel = model;
    }

    void setPageType(PageType type) Q_DECL_OVERRIDE
    {
        lastPageAdded = type;
    }

    PageType pageType() const Q_DECL_OVERRIDE
    {
        return lastPageAdded;
    }

    QString name() const Q_DECL_OVERRIDE
    {
        return "name";
    }

    Domain::DataSource::Ptr dataSource() const Q_DECL_OVERRIDE
    {
        return source;
    }

    QWidget *parent;
    int execCount;
    QAbstractItemModel *sourceModel;
    Domain::DataSource::Ptr source;
    PageType lastPageAdded;
};

class AvailablePagesModelStub : public QObject
{
    Q_OBJECT
public:
    explicit AvailablePagesModelStub(QObject *parent = Q_NULLPTR)
        : QObject(parent)
    {
    }
public slots:
    void addProject(const QString &name, const Domain::DataSource::Ptr &source)
    {
        projectNames << name;
        sources << source;
    }

    void addContext(const QString &name)
    {
        contextNames << name;
    }

    void addTag(const QString &name)
    {
        tagNames << name;
    }

    void removeItem(const QModelIndex &index)
    {
        projectRemoved = index.data().toString();
    }

public Q_SLOTS:
    QObject *createPageForIndex(const QModelIndex &) { return Q_NULLPTR; }

public:
    QStringList projectNames;
    QStringList contextNames;
    QStringList tagNames;
    QList<Domain::DataSource::Ptr> sources;
    QString projectRemoved;
};

class AvailablePagesViewTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldHaveDefaultState()
    {
        Widgets::AvailablePagesView available;

        QVERIFY(!available.model());
        QVERIFY(!available.projectSourcesModel());
        QVERIFY(available.defaultProjectSource().isNull());

        auto pagesView = available.findChild<QTreeView*>("pagesView");
        QVERIFY(pagesView);
        QVERIFY(pagesView->isVisibleTo(&available));
        QVERIFY(!pagesView->header()->isVisibleTo(&available));
        QCOMPARE(pagesView->dragDropMode(), QTreeView::DropOnly);

        auto actionBar = available.findChild<QToolBar*>("actionBar");
        QVERIFY(actionBar);
        QVERIFY(actionBar->isVisibleTo(&available));

        auto addProjectAction = available.findChild<QAction*>("addProjectAction");
        QVERIFY(addProjectAction);
        auto addContextAction = available.findChild<QAction*>("addContextAction");
        QVERIFY(addContextAction);
        auto addTagAction = available.findChild<QAction*>("addTagAction");
        QVERIFY(addTagAction);
        auto removeAction = available.findChild<QAction*>("removeAction");
        QVERIFY(removeAction);
        auto goPreviousAction = available.findChild<QAction*>("goPreviousAction");
        QVERIFY(goPreviousAction);
        auto goNextAction = available.findChild<QAction*>("goNextAction");
        QVERIFY(goNextAction);

        auto factory = available.projectDialogFactory();
        QVERIFY(factory(&available).dynamicCast<Widgets::NewPageDialog>());

        auto actions = available.globalActions();
        QCOMPARE(actions.value("pages_project_add"), addProjectAction);
        QCOMPARE(actions.value("pages_context_add"), addContextAction);
        QCOMPARE(actions.value("pages_tag_add"), addTagAction);
        QCOMPARE(actions.value("pages_remove"), removeAction);
        QCOMPARE(actions.value("pages_go_previous"), goPreviousAction);
        QCOMPARE(actions.value("pages_go_next"), goNextAction);
    }

    void shouldDisplayListFromPageModel()
    {
        // GIVEN
        QStringListModel model(QStringList() << "A" << "B" << "C" );

        AvailablePagesModelStub stubPagesModel;
        stubPagesModel.setProperty("pageListModel", QVariant::fromValue(static_cast<QAbstractItemModel*>(&model)));

        Widgets::AvailablePagesView available;
        auto pagesView = available.findChild<QTreeView*>("pagesView");
        QVERIFY(pagesView);
        QVERIFY(!pagesView->model());

        // WHEN
        available.setModel(&stubPagesModel);
        QTest::qWait(10);

        // THEN
        QCOMPARE(pagesView->model(), &model);
        QCOMPARE(pagesView->selectionModel()->currentIndex(), model.index(0, 0));
    }

    void shouldAddNewProjects()
    {
        // GIVEN
        AvailablePagesModelStub model;
        QStringListModel sourceModel;
        auto dialogStub = NewPageDialogStub::Ptr::create();

        auto source = Domain::DataSource::Ptr::create();

        Widgets::AvailablePagesView available;
        available.setModel(&model);
        available.setProjectSourcesModel(&sourceModel);
        available.setDefaultProjectSource(source);
        available.setProjectDialogFactory([dialogStub] (QWidget *parent) {
            dialogStub->parent = parent;
            return dialogStub;
        });

        auto addProjectAction = available.findChild<QAction*>("addProjectAction");

        // WHEN
        addProjectAction->trigger();

        // THEN
        QCOMPARE(dialogStub->execCount, 1);
        QCOMPARE(dialogStub->parent, &available);
        QCOMPARE(dialogStub->sourceModel, &sourceModel);
        QCOMPARE(model.projectNames.size(), 1);
        QCOMPARE(model.projectNames.first(), dialogStub->name());
        QCOMPARE(model.sources.size(), 1);
        QCOMPARE(model.sources.first(), dialogStub->dataSource());
        QCOMPARE(available.defaultProjectSource(), dialogStub->dataSource());
    }

    void shouldAddNewContexts()
    {
        // GIVEN
        AvailablePagesModelStub model;
        QStringListModel sourceModel;
        auto dialogStub = NewPageDialogStub::Ptr::create();

        auto source = Domain::DataSource::Ptr::create();

        auto msgBoxStub = MessageBoxStub::Ptr::create();
        msgBoxStub->setTextInput("Foo");

        Widgets::AvailablePagesView available;
        available.setModel(&model);
        available.setProjectSourcesModel(&sourceModel);
        available.setDefaultProjectSource(source);
        available.setMessageBoxInterface(msgBoxStub);

        auto addContextAction = available.findChild<QAction*>("addContextAction");

        // WHEN
        addContextAction->trigger();

        // THEN
        QVERIFY(msgBoxStub->called());
        QCOMPARE(model.contextNames.size(), 1);
        QCOMPARE(model.contextNames.first(), QString("Foo"));
    }

    void shouldAddNewTags()
    {
        // GIVEN
        AvailablePagesModelStub model;
        QStringListModel sourceModel;
        auto dialogStub = NewPageDialogStub::Ptr::create();

        auto source = Domain::DataSource::Ptr::create();

        auto msgBoxStub = MessageBoxStub::Ptr::create();
        msgBoxStub->setTextInput("Foo");

        Widgets::AvailablePagesView available;
        available.setModel(&model);
        available.setProjectSourcesModel(&sourceModel);
        available.setDefaultProjectSource(source);
        available.setMessageBoxInterface(msgBoxStub);

        auto addTagAction = available.findChild<QAction*>("addTagAction");

        // WHEN
        addTagAction->trigger();

        // THEN
        QVERIFY(msgBoxStub->called());
        QCOMPARE(model.tagNames.size(), 1);
        QCOMPARE(model.tagNames.first(), QString("Foo"));
    }

    void shouldRemoveAPage_data()
    {
        QTest::addColumn<QObjectPtr>("object");

        auto project1 = Domain::Project::Ptr::create();
        project1->setName("Project 1");
        QTest::newRow("project") << QObjectPtr(project1);

        auto context1 = Domain::Context::Ptr::create();
        context1->setName("Context 1");
        QTest::newRow("context") << QObjectPtr(context1);

        auto tag1 = Domain::Tag::Ptr::create();
        tag1->setName("Tag 1");
        QTest::newRow("tag") << QObjectPtr(tag1);
    }

    void shouldRemoveAPage()
    {
        QFETCH(QObjectPtr, object);

        // GIVEN
        QStringList list;
        list << "A" << "B" << "C";
        QStandardItemModel model;
        for (int row = 0; row < list.count(); ++row) {
            model.setItem(row, new QStandardItem(list.at(row)));
        }

        AvailablePagesModelStub stubPagesModel;
        stubPagesModel.setProperty("pageListModel", QVariant::fromValue(static_cast<QAbstractItemModel*>(&model)));

        Widgets::AvailablePagesView available;
        auto pagesView = available.findChild<QTreeView*>("pagesView");
        QVERIFY(pagesView);
        QVERIFY(!pagesView->model());

        available.setModel(&stubPagesModel);
        QTest::qWait(10);

        auto removeAction = available.findChild<QAction*>("removeAction");

        QVERIFY(model.setData(model.index(0, 0), QVariant::fromValue(object), Presentation::QueryTreeModelBase::ObjectRole));

        auto msgbox = MessageBoxStub::Ptr::create();
        available.setMessageBoxInterface(msgbox);

        // WHEN
        removeAction->trigger();

        // THEN
        QCOMPARE(stubPagesModel.projectRemoved, list.first());
    }

    void shouldGoToPreviousSelectablePage()
    {
        // GIVEN
        QStandardItemModel model;
        model.appendRow(new QStandardItem("Inbox"));
        auto projects = new QStandardItem("Projects");
        projects->setFlags(Qt::NoItemFlags);
        model.appendRow(projects);
        projects->appendRow(new QStandardItem("Project 1"));
        projects->appendRow(new QStandardItem("Project 2"));

        AvailablePagesModelStub stubPagesModel;
        stubPagesModel.setProperty("pageListModel", QVariant::fromValue(static_cast<QAbstractItemModel*>(&model)));

        Widgets::AvailablePagesView available;
        auto pagesView = available.findChild<QTreeView*>("pagesView");
        QVERIFY(pagesView);
        QVERIFY(!pagesView->model());

        available.setModel(&stubPagesModel);
        QTest::qWait(10);

        auto goPreviousAction = available.findChild<QAction*>("goPreviousAction");
        pagesView->setCurrentIndex(model.index(1, 0, model.indexFromItem(projects)));

        // WHEN
        goPreviousAction->trigger();

        // THEN
        QCOMPARE(pagesView->currentIndex(),
                 model.index(0, 0, model.indexFromItem(projects)));

        // WHEN
        goPreviousAction->trigger();

        // THEN
        QCOMPARE(pagesView->currentIndex(),
                 model.index(0, 0));

        // WHEN
        goPreviousAction->trigger();

        // THEN
        QCOMPARE(pagesView->currentIndex(),
                 model.index(0, 0));
    }

    void shouldGoToNextSelectablePage()
    {
        // GIVEN
        QStandardItemModel model;
        model.appendRow(new QStandardItem("Inbox"));
        auto projects = new QStandardItem("Projects");
        projects->setFlags(Qt::NoItemFlags);
        model.appendRow(projects);
        projects->appendRow(new QStandardItem("Project 1"));
        projects->appendRow(new QStandardItem("Project 2"));

        AvailablePagesModelStub stubPagesModel;
        stubPagesModel.setProperty("pageListModel", QVariant::fromValue(static_cast<QAbstractItemModel*>(&model)));

        Widgets::AvailablePagesView available;
        auto pagesView = available.findChild<QTreeView*>("pagesView");
        QVERIFY(pagesView);
        QVERIFY(!pagesView->model());

        available.setModel(&stubPagesModel);
        QTest::qWait(10);

        auto goNextAction = available.findChild<QAction*>("goNextAction");
        pagesView->setCurrentIndex(model.index(0, 0));

        // WHEN
        goNextAction->trigger();

        // THEN
        QCOMPARE(pagesView->currentIndex(),
                 model.index(0, 0, model.indexFromItem(projects)));

        // WHEN
        goNextAction->trigger();

        // THEN
        QCOMPARE(pagesView->currentIndex(),
                 model.index(1, 0, model.indexFromItem(projects)));

        // WHEN
        goNextAction->trigger();

        // THEN
        QCOMPARE(pagesView->currentIndex(),
                 model.index(1, 0, model.indexFromItem(projects)));
    }
};

QTEST_MAIN(AvailablePagesViewTest)

#include "availablepagesviewtest.moc"
