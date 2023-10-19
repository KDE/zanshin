/*
 * SPDX-FileCopyrightText: 2015 Franck Arrecot <franck.arrecot@gmail.com>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/



#include <testlib/qtest_gui_zanshin.h>

#include <QHeaderView>
#include <QLabel>
#include <QSignalSpy>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTestEventList>
#include <QTimer>
#include <QTreeView>
#include <QList>

#include "widgets/quickselectdialog.h"

Q_DECLARE_METATYPE(QStandardItemModel*)

class QuickSelectDialogTest : public QObject
{
    Q_OBJECT
private:
    template <typename T>
    T *widgetFromQuickSelectDialog(Widgets::QuickSelectDialog *dlg)
    {
        auto widgets = dlg->findChildren<T*>();
        Q_ASSERT(widgets.size() == 1);
        return widgets.first();
    }

    QModelIndex modelIndexFromNameHelper(QAbstractItemModel *model, const QString &name, const QModelIndex &root = QModelIndex())
    {
        for (int i = 0; i < model->rowCount(root); i++)
        {
            auto idx = model->index(i, 0, root);
            if (idx.data().toString() == name) {
                return idx;
            } else if (model->rowCount(idx) > 0) {
                auto idxChild = modelIndexFromNameHelper(model, name, idx);
                if (idxChild.isValid()) {
                    return idxChild;
                }
            }
        }
        return {};
    }

    QAbstractItemModel *prepareQuickSelectDialogData()
    {
        // GIVEN

        // a model with items
        auto model = new QStandardItemModel(this);
        auto inbox = new QStandardItem(QStringLiteral("Inbox"));
        auto workday = new QStandardItem(QStringLiteral("Workday"));
        auto projects= new QStandardItem(QStringLiteral("Projects"));
        auto contexts = new QStandardItem(QStringLiteral("Contexts"));

        auto structureNodeFlags = Qt::NoItemFlags;
        projects->setFlags(structureNodeFlags);
        contexts->setFlags(structureNodeFlags);

        // with items children
        auto projectChildOne = new QStandardItem(QStringLiteral("ProjectOne"));
        auto projectChildTwo = new QStandardItem(QStringLiteral("ProjectTwo"));
        projects->setChild(0, 0, projectChildOne);
        projects->setChild(1, 0,projectChildTwo);

        auto contextChildOne = new QStandardItem(QStringLiteral("ContextOne"));
        auto contextChildTwo = new QStandardItem(QStringLiteral("ContextTwo"));
        contexts->setChild(0, 0, contextChildOne);
        contexts->setChild(1, 0, contextChildTwo);

        auto items = QList<QStandardItem*>();
        items << inbox << workday << projects << contexts;

        for (int i = 0; i < items.size(); ++i)
            model->setItem(i, items.at(i));

        return model;
    }

private slots:
    void shouldHaveDefaultState()
    {
        Widgets::QuickSelectDialog dlg;

        auto pagesView = dlg.findChild<QTreeView*>(QStringLiteral("pagesView"));
        QVERIFY(pagesView);
        QVERIFY(pagesView->isVisibleTo(&dlg));
        QVERIFY(!pagesView->header()->isVisibleTo(&dlg));
    }

    void shouldCloseDialogOnOk()
    {
        // GIVEN
        auto model = prepareQuickSelectDialogData();
        Widgets::QuickSelectDialog dlg;
        dlg.setModel(model);
        auto treeView = widgetFromQuickSelectDialog<QTreeView>(&dlg);
        dlg.show();

        // WHEN
        QTest::keyClick(treeView, Qt::Key_Enter);

        //THEN
        QCOMPARE(dlg.result(), static_cast<int>(QDialog::Accepted));
    }

    void shouldDisplayTheProperTreeModel()
    {
        // GIVEN
        auto model = prepareQuickSelectDialogData();
        Widgets::QuickSelectDialog dlg;
        dlg.setModel(model);

        auto treeView = widgetFromQuickSelectDialog<QTreeView>(&dlg);
        auto displayModel = treeView->model();

        // WHEN
        dlg.show();

        // THEN
        auto inbox = displayModel->index(0,0);
        auto workday = displayModel->index(1,0);
        auto projects = displayModel->index(2,0);
        auto contexts = displayModel->index(3,0);

        QCOMPARE(inbox.data().toString(), QStringLiteral("Inbox"));
        QCOMPARE(workday.data().toString(), QStringLiteral("Workday"));
        QCOMPARE(projects.data().toString(), QStringLiteral("Projects"));
        QCOMPARE(contexts.data().toString(), QStringLiteral("Contexts"));

        QCOMPARE(displayModel->columnCount(), 1);
        QCOMPARE(displayModel->rowCount(), 4); // inbox, workday, projects, contexts
        QCOMPARE(displayModel->rowCount(inbox), 0); // inbox do not expose any children
        QCOMPARE(displayModel->rowCount(workday), 0); // worday do not expose any children
        QCOMPARE(displayModel->rowCount(projects), 2); // two children projects created
        QCOMPARE(displayModel->rowCount(contexts), 2); // two children contexts created
    }

    void shouldFilterTreeOnTyping()
    {
        // GIVEN
        auto model = prepareQuickSelectDialogData();
        Widgets::QuickSelectDialog dlg;
        dlg.setModel(model);

        auto treeView = widgetFromQuickSelectDialog<QTreeView>(&dlg);
        auto displayModel = treeView->model();
        auto labelFilter = widgetFromQuickSelectDialog<QLabel>(&dlg);

        // WHEN
        QTest::keyClick(treeView, Qt::Key_O, Qt::NoModifier, 20);
        QTest::keyClick(treeView, Qt::Key_N, Qt::NoModifier, 20);
        QTest::keyClick(treeView, Qt::Key_E, Qt::NoModifier, 20);

        dlg.show();

        // THEN
        auto projects = displayModel->index(0,0);
        auto contexts = displayModel->index(1,0);

        QCOMPARE(labelFilter->text(), QStringLiteral("Path: one"));
        QCOMPARE(displayModel->columnCount(), 1);
        QCOMPARE(displayModel->rowCount(), 2);
        QCOMPARE(projects.data().toString(), QStringLiteral("Projects"));
        QCOMPARE(contexts.data().toString(), QStringLiteral("Contexts"));

        QCOMPARE(displayModel->rowCount(projects), 1);
        auto projectsChild = displayModel->index(0, 0, projects);
        QCOMPARE(projectsChild.data().toString(), QStringLiteral("ProjectOne"));

        QCOMPARE(displayModel->rowCount(contexts), 1);
        auto contextsChild = displayModel->index(0, 0, contexts);
        QCOMPARE(contextsChild.data().toString(), QStringLiteral("ContextOne"));
    }

    void shouldReturnTheSelectedIndex()
    {
        // GIVEN
        auto model = prepareQuickSelectDialogData();
        Widgets::QuickSelectDialog dlg;
        dlg.setModel(model);
        dlg.show();

        auto selectedItemNames = QList<QString>();
        selectedItemNames  << QStringLiteral("Inbox") << QStringLiteral("Workday") << QStringLiteral("ProjectOne") << QStringLiteral("ProjectTwo") <<  QStringLiteral("ContextOne") << QStringLiteral("ContextTwo") << QStringLiteral("TagOne") << QStringLiteral("TagTwo");

        foreach (const auto &itemName, selectedItemNames) {
            auto treeview = widgetFromQuickSelectDialog<QTreeView>(&dlg);
            auto selectedItem = modelIndexFromNameHelper(treeview->model(), itemName);

            // WHEN
            treeview->setCurrentIndex(selectedItem);

            // THEN
            QCOMPARE(dlg.selectedIndex().data(), selectedItem.data() );
        }
    }
};

ZANSHIN_TEST_MAIN(QuickSelectDialogTest)

#include "quickselectdialogtest.moc"
