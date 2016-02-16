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

#include <testlib/qtest_gui_zanshin.h>

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
#include "widgets/newprojectdialog.h"
#include "widgets/quickselectdialog.h"

#include "messageboxstub.h"

class NewProjectDialogStub : public Widgets::NewProjectDialogInterface
{
public:
    typedef QSharedPointer<NewProjectDialogStub> Ptr;

    explicit NewProjectDialogStub()
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

    QString name() const Q_DECL_OVERRIDE
    {
        return QStringLiteral("name");
    }

    Domain::DataSource::Ptr dataSource() const Q_DECL_OVERRIDE
    {
        return source;
    }

    QWidget *parent;
    int execCount;
    QAbstractItemModel *sourceModel;
    Domain::DataSource::Ptr source;
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

        auto pagesView = available.findChild<QTreeView*>(QStringLiteral("pagesView"));
        QVERIFY(pagesView);
        QVERIFY(pagesView->isVisibleTo(&available));
        QVERIFY(!pagesView->header()->isVisibleTo(&available));
        QCOMPARE(pagesView->dragDropMode(), QTreeView::DropOnly);

        auto actionBar = available.findChild<QToolBar*>(QStringLiteral("actionBar"));
        QVERIFY(actionBar);
        QVERIFY(actionBar->isVisibleTo(&available));

        auto addProjectAction = available.findChild<QAction*>(QStringLiteral("addProjectAction"));
        QVERIFY(addProjectAction);
        auto addContextAction = available.findChild<QAction*>(QStringLiteral("addContextAction"));
        QVERIFY(addContextAction);
        auto addTagAction = available.findChild<QAction*>(QStringLiteral("addTagAction"));
        QVERIFY(addTagAction);
        auto removeAction = available.findChild<QAction*>(QStringLiteral("removeAction"));
        QVERIFY(removeAction);
        auto goPreviousAction = available.findChild<QAction*>(QStringLiteral("goPreviousAction"));
        QVERIFY(goPreviousAction);
        auto goNextAction = available.findChild<QAction*>(QStringLiteral("goNextAction"));
        QVERIFY(goNextAction);
        auto goToAction = available.findChild<QAction*>(QStringLiteral("goToAction"));
        QVERIFY(goToAction);

        auto projectDialogFactory = available.projectDialogFactory();
        QVERIFY(projectDialogFactory(&available).dynamicCast<Widgets::NewProjectDialog>());

        auto quickSelectDialogFactory = available.quickSelectDialogFactory();
        QVERIFY(quickSelectDialogFactory(&available).dynamicCast<Widgets::QuickSelectDialog>());

        auto actions = available.globalActions();
        QCOMPARE(actions.value(QStringLiteral("pages_project_add")), addProjectAction);
        QCOMPARE(actions.value(QStringLiteral("pages_context_add")), addContextAction);
        QCOMPARE(actions.value(QStringLiteral("pages_tag_add")), addTagAction);
        QCOMPARE(actions.value(QStringLiteral("pages_remove")), removeAction);
        QCOMPARE(actions.value(QStringLiteral("pages_go_previous")), goPreviousAction);
        QCOMPARE(actions.value(QStringLiteral("pages_go_next")), goNextAction);
        QCOMPARE(actions.value(QStringLiteral("pages_go_to")), goToAction);
    }

    void shouldShowOnlyAddActionsNeededByTheModel_data()
    {
        QTest::addColumn<bool>("hasProjects");
        QTest::addColumn<bool>("hasContexts");
        QTest::addColumn<bool>("hasTags");

        QTest::newRow("!projects !contexts !tags") << false << false << false;
        QTest::newRow("!projects !contexts tags") << false << false << true;
        QTest::newRow("!projects contexts !tags") << false << true << false;
        QTest::newRow("!projects contexts tags") << false << true << true;
        QTest::newRow("projects !contexts !tags") << true << false << false;
        QTest::newRow("projects !contexts tags") << true << false << true;
        QTest::newRow("projects contexts !tags") << true << true << false;
        QTest::newRow("projects contexts tags") << true << true << true;
    }

    void shouldShowOnlyAddActionsNeededByTheModel()
    {
        // GIVEN
        QFETCH(bool, hasProjects);
        QFETCH(bool, hasContexts);
        QFETCH(bool, hasTags);

        AvailablePagesModelStub stubPagesModel;
        stubPagesModel.setProperty("hasProjectPages", hasProjects);
        stubPagesModel.setProperty("hasContextPages", hasContexts);
        stubPagesModel.setProperty("hasTagPages", hasTags);

        Widgets::AvailablePagesView available;
        auto addProjectAction = available.findChild<QAction*>(QStringLiteral("addProjectAction"));
        QVERIFY(addProjectAction);
        auto addContextAction = available.findChild<QAction*>(QStringLiteral("addContextAction"));
        QVERIFY(addContextAction);
        auto addTagAction = available.findChild<QAction*>(QStringLiteral("addTagAction"));
        QVERIFY(addTagAction);

        // WHEN
        available.setModel(&stubPagesModel);

        // THEN
        QCOMPARE(addProjectAction->isVisible(), hasProjects);
        QCOMPARE(addContextAction->isVisible(), hasContexts);
        QCOMPARE(addTagAction->isVisible(), hasTags);
    }

    void shouldDisplayListFromPageModel()
    {
        // GIVEN
        QStringListModel model(QStringList() << QStringLiteral("A") << QStringLiteral("B") << QStringLiteral("C") );

        AvailablePagesModelStub stubPagesModel;
        stubPagesModel.setProperty("pageListModel", QVariant::fromValue(static_cast<QAbstractItemModel*>(&model)));

        Widgets::AvailablePagesView available;
        auto pagesView = available.findChild<QTreeView*>(QStringLiteral("pagesView"));
        QVERIFY(pagesView);
        QVERIFY(!pagesView->model());

        // WHEN
        available.setModel(&stubPagesModel);
        QTest::qWait(10);

        // THEN
        QCOMPARE(pagesView->model(), &model);
        QCOMPARE(pagesView->selectionModel()->currentIndex(), model.index(0, 0));
    }

    void shouldNotCrashWithNullModel()
    {
        // GIVEN
        QStringListModel model(QStringList() << QStringLiteral("A") << QStringLiteral("B") << QStringLiteral("C") );

        AvailablePagesModelStub stubPagesModel;
        stubPagesModel.setProperty("pageListModel", QVariant::fromValue(static_cast<QAbstractItemModel*>(&model)));

        Widgets::AvailablePagesView available;
        available.setModel(&stubPagesModel);
        QTest::qWait(10);

        auto pagesView = available.findChild<QTreeView*>(QStringLiteral("pagesView"));
        QVERIFY(pagesView);
        QCOMPARE(pagesView->model(), &model);

        // WHEN
        available.setModel(Q_NULLPTR);
        QTest::qWait(10);

        // THEN
        QVERIFY(!available.isEnabled());
        QVERIFY(!pagesView->model());
    }

    void shouldAddNewProjects()
    {
        // GIVEN
        AvailablePagesModelStub model;
        QStringListModel sourceModel;
        auto dialogStub = NewProjectDialogStub::Ptr::create();

        auto source = Domain::DataSource::Ptr::create();

        Widgets::AvailablePagesView available;
        available.setModel(&model);
        available.setProjectSourcesModel(&sourceModel);
        available.setDefaultProjectSource(source);
        available.setProjectDialogFactory([dialogStub] (QWidget *parent) {
            dialogStub->parent = parent;
            return dialogStub;
        });

        auto addProjectAction = available.findChild<QAction*>(QStringLiteral("addProjectAction"));

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
        auto dialogStub = NewProjectDialogStub::Ptr::create();

        auto source = Domain::DataSource::Ptr::create();

        auto msgBoxStub = MessageBoxStub::Ptr::create();
        msgBoxStub->setTextInput(QStringLiteral("Foo"));

        Widgets::AvailablePagesView available;
        available.setModel(&model);
        available.setProjectSourcesModel(&sourceModel);
        available.setDefaultProjectSource(source);
        available.setMessageBoxInterface(msgBoxStub);

        auto addContextAction = available.findChild<QAction*>(QStringLiteral("addContextAction"));

        // WHEN
        addContextAction->trigger();

        // THEN
        QVERIFY(msgBoxStub->called());
        QCOMPARE(model.contextNames.size(), 1);
        QCOMPARE(model.contextNames.first(), QStringLiteral("Foo"));
    }

    void shouldAddNewTags()
    {
        // GIVEN
        AvailablePagesModelStub model;
        QStringListModel sourceModel;
        auto dialogStub = NewProjectDialogStub::Ptr::create();

        auto source = Domain::DataSource::Ptr::create();

        auto msgBoxStub = MessageBoxStub::Ptr::create();
        msgBoxStub->setTextInput(QStringLiteral("Foo"));

        Widgets::AvailablePagesView available;
        available.setModel(&model);
        available.setProjectSourcesModel(&sourceModel);
        available.setDefaultProjectSource(source);
        available.setMessageBoxInterface(msgBoxStub);

        auto addTagAction = available.findChild<QAction*>(QStringLiteral("addTagAction"));

        // WHEN
        addTagAction->trigger();

        // THEN
        QVERIFY(msgBoxStub->called());
        QCOMPARE(model.tagNames.size(), 1);
        QCOMPARE(model.tagNames.first(), QStringLiteral("Foo"));
    }

    void shouldRemoveAPage_data()
    {
        QTest::addColumn<QObjectPtr>("object");
        QTest::addColumn<bool>("actionEnabled");

        auto project1 = Domain::Project::Ptr::create();
        project1->setName(QStringLiteral("Project 1"));
        QTest::newRow("project") << QObjectPtr(project1) << true;

        auto context1 = Domain::Context::Ptr::create();
        context1->setName(QStringLiteral("Context 1"));
        QTest::newRow("context") << QObjectPtr(context1) << true;

        auto tag1 = Domain::Tag::Ptr::create();
        tag1->setName(QStringLiteral("Tag 1"));
        QTest::newRow("tag") << QObjectPtr(tag1) << true;

        QTest::newRow("non removable") << QObjectPtr::create() << false;
    }

    void shouldRemoveAPage()
    {
        QFETCH(QObjectPtr, object);
        QFETCH(bool, actionEnabled);

        // GIVEN
        QStringList list;
        list << QStringLiteral("A") << QStringLiteral("B") << QStringLiteral("C");
        QStandardItemModel model;
        for (int row = 0; row < list.count(); ++row) {
            model.setItem(row, new QStandardItem(list.at(row)));
        }
        QVERIFY(model.setData(model.index(0, 0), QVariant::fromValue(object), Presentation::QueryTreeModelBase::ObjectRole));

        AvailablePagesModelStub stubPagesModel;
        stubPagesModel.setProperty("pageListModel", QVariant::fromValue(static_cast<QAbstractItemModel*>(&model)));

        Widgets::AvailablePagesView available;
        auto pagesView = available.findChild<QTreeView*>(QStringLiteral("pagesView"));
        QVERIFY(pagesView);
        QVERIFY(!pagesView->model());

        available.setModel(&stubPagesModel);
        QTest::qWait(10);

        auto removeAction = available.findChild<QAction*>(QStringLiteral("removeAction"));

        auto msgbox = MessageBoxStub::Ptr::create();
        available.setMessageBoxInterface(msgbox);

        // WHEN
        if (actionEnabled)
            removeAction->trigger();

        // THEN
        QCOMPARE(removeAction->isEnabled(), actionEnabled);
        if (actionEnabled) {
            QCOMPARE(stubPagesModel.projectRemoved, list.first());
        }
    }

    void shouldGoToPreviousSelectablePage()
    {
        // GIVEN
        QStandardItemModel model;
        model.appendRow(new QStandardItem(QStringLiteral("Inbox")));
        auto projects = new QStandardItem(QStringLiteral("Projects"));
        projects->setFlags(Qt::NoItemFlags);
        model.appendRow(projects);
        projects->appendRow(new QStandardItem(QStringLiteral("Project 1")));
        projects->appendRow(new QStandardItem(QStringLiteral("Project 2")));

        AvailablePagesModelStub stubPagesModel;
        stubPagesModel.setProperty("pageListModel", QVariant::fromValue(static_cast<QAbstractItemModel*>(&model)));

        Widgets::AvailablePagesView available;
        auto pagesView = available.findChild<QTreeView*>(QStringLiteral("pagesView"));
        QVERIFY(pagesView);
        QVERIFY(!pagesView->model());

        available.setModel(&stubPagesModel);
        QTest::qWait(10);

        auto goPreviousAction = available.findChild<QAction*>(QStringLiteral("goPreviousAction"));
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
        model.appendRow(new QStandardItem(QStringLiteral("Inbox")));
        auto projects = new QStandardItem(QStringLiteral("Projects"));
        projects->setFlags(Qt::NoItemFlags);
        model.appendRow(projects);
        projects->appendRow(new QStandardItem(QStringLiteral("Project 1")));
        projects->appendRow(new QStandardItem(QStringLiteral("Project 2")));

        AvailablePagesModelStub stubPagesModel;
        stubPagesModel.setProperty("pageListModel", QVariant::fromValue(static_cast<QAbstractItemModel*>(&model)));

        Widgets::AvailablePagesView available;
        auto pagesView = available.findChild<QTreeView*>(QStringLiteral("pagesView"));
        QVERIFY(pagesView);
        QVERIFY(!pagesView->model());

        available.setModel(&stubPagesModel);
        QTest::qWait(10);

        auto goNextAction = available.findChild<QAction*>(QStringLiteral("goNextAction"));
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

    void shouldGoToUserSelectedIndex()
    {
        // GIVEN
        QStandardItemModel model;
        model.appendRow(new QStandardItem(QStringLiteral("Inbox")));
        auto projects = new QStandardItem(QStringLiteral("Projects"));
        projects->setFlags(Qt::NoItemFlags);
        model.appendRow(projects);
        projects->appendRow(new QStandardItem(QStringLiteral("Project 1")));
        projects->appendRow(new QStandardItem(QStringLiteral("Project 2")));

        AvailablePagesModelStub stubPagesModel;
        stubPagesModel.setProperty("pageListModel", QVariant::fromValue(static_cast<QAbstractItemModel*>(&model)));
        auto dialogStub = QuickSelectDialogStub::Ptr::create();
        // Project 2 will be selected
        dialogStub->index = model.index(1, 0, model.index(1, 0));

        Widgets::AvailablePagesView available;
        available.setModel(&stubPagesModel);
        available.setQuickSelectDialogFactory([dialogStub] (QWidget *parent) {
            dialogStub->parent = parent;
            return dialogStub;
        });

        auto pagesView = available.findChild<QTreeView*>(QStringLiteral("pagesView"));
        QVERIFY(pagesView);
        QCOMPARE(pagesView->model(), &model);

        auto goToAction = available.findChild<QAction*>(QStringLiteral("goToAction"));

        // WHEN
        goToAction->trigger();

        // THEN
        QCOMPARE(dialogStub->execCount, 1);
        QCOMPARE(dialogStub->parent, &available);
        QCOMPARE(dialogStub->itemModel, &model);
        QCOMPARE(QPersistentModelIndex(pagesView->currentIndex()), dialogStub->index);
    }
};

ZANSHIN_TEST_MAIN(AvailablePagesViewTest)

#include "availablepagesviewtest.moc"
