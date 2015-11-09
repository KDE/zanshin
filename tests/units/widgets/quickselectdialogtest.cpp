/* This file is part of Zanshin

   Copyright 2015 Franck Arrecot <franck.arrecot@gmail.com>

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

#include <QLabel>
#include <QSignalSpy>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTestEventList>
#include <QTimer>
#include <QTreeView>
#include <QVector>

#include "widgets/quickselectdialog.h"

Q_DECLARE_METATYPE(QStandardItemModel*)

class QuickSelectDialogTest : public QObject
{
    Q_OBJECT
private:
    template <typename T>
    T *widgetFromQuickSelectDialog(QuickSelectDialog *dlg)
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
        auto model = new QStandardItemModel();
        auto inbox = new QStandardItem("Inbox");
        auto workday = new QStandardItem("Workday");
        auto projects= new QStandardItem("Projects");
        auto contexts = new QStandardItem("Contexts");

        auto structureNodeFlags = Qt::NoItemFlags;
        projects->setFlags(structureNodeFlags);
        contexts->setFlags(structureNodeFlags);

        // with items children
        auto projectChildOne = new QStandardItem("ProjectOne");
        auto projectChildTwo = new QStandardItem("ProjectTwo");
        projects->setChild(0, 0, projectChildOne);
        projects->setChild(1, 0,projectChildTwo);

        auto contextChildOne = new QStandardItem("ContextOne");
        auto contextChildTwo = new QStandardItem("ContextTwo");
        contexts->setChild(0, 0, contextChildOne);
        contexts->setChild(1, 0, contextChildTwo);

        auto items = QVector<QStandardItem*>();
        items << inbox << workday << projects << contexts;

        for (int i = 0; i < items.size(); ++i)
            model->setItem(i, items.at(i));

        return model;
    }

private slots:
    void shouldCloseDialogOnOk()
    {
        // GIVEN
        auto model = prepareQuickSelectDialogData();
        QuickSelectDialog dlg;
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
        QuickSelectDialog dlg;
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

        QCOMPARE(inbox.data().toString(), QString("Inbox"));
        QCOMPARE(workday.data().toString(), QString("Workday"));
        QCOMPARE(projects.data().toString(), QString("Projects"));
        QCOMPARE(contexts.data().toString(), QString("Contexts"));

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
        QuickSelectDialog dlg;
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

        QCOMPARE(labelFilter->text(), QString("Path: one"));
        QCOMPARE(displayModel->columnCount(), 1);
        QCOMPARE(displayModel->rowCount(), 2);
        QCOMPARE(projects.data().toString(), QString("Projects"));
        QCOMPARE(contexts.data().toString(), QString("Contexts"));

        QCOMPARE(displayModel->rowCount(projects), 1);
        auto projectsChild = displayModel->index(0, 0, projects);
        QCOMPARE(projectsChild.data().toString(), QString("ProjectOne"));

        QCOMPARE(displayModel->rowCount(contexts), 1);
        auto contextsChild = displayModel->index(0, 0, contexts);
        QCOMPARE(contextsChild.data().toString(), QString("ContextOne"));
    }

    void shouldReturnTheSelectedIndex()
    {
        // GIVEN
        auto model = prepareQuickSelectDialogData();
        QuickSelectDialog dlg;
        dlg.setModel(model);
        dlg.show();

        auto selectedItemNames = QVector<QString>();
        selectedItemNames  << "Inbox" << "Workday" << "ProjectOne" << "ProjectTwo" <<  "ContextOne" << "ContextTwo" << "TagOne" << "TagTwo";

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

QTEST_MAIN(QuickSelectDialogTest)

#include "quickselectdialogtest.moc"
