/* This file is part of Zanshin Todo.

   Copyright 2008-2010 Kevin Ottens <ervin@kde.org>

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

#include <KDE/KAboutData>
#include <KDE/KApplication>
#include <KDE/KCmdLineArgs>
#include <KDE/KLocale>

#include "aboutdata.h"
#include "debugwindow.h"
#include "mainwindow.h"
#include "modelstack.h"

/*
template<class ProxyModel>
void createViews(TodoModel *baseModel)
{
    ProxyModel *proxy = new ProxyModel;
    proxy->setSourceModel(baseModel);

    SideBarModel *sidebarModel = new SideBarModel;
    sidebarModel->setSourceModel(proxy);

    QString className = proxy->metaObject()->className();

    Akonadi::EntityTreeView *sidebar = new Akonadi::EntityTreeView;
    sidebar->setWindowTitle(className+"/SideBar");
    sidebar->setModel(sidebarModel);
    sidebar->setSelectionMode(QAbstractItemView::ExtendedSelection);
    sidebar->show();

    SelectionProxyModel *selectionProxy = new SelectionProxyModel(sidebar->selectionModel());
    selectionProxy->setSourceModel(proxy);

    Akonadi::EntityTreeView *mainView = new Akonadi::EntityTreeView;
    mainView->setItemsExpandable(false);
    mainView->setWindowTitle(className);
    mainView->setModel(selectionProxy);

    QObject::connect(selectionProxy, SIGNAL(modelReset()),
                     mainView, SLOT(expandAll()));
    QObject::connect(selectionProxy, SIGNAL(layoutChanged()),
                     mainView, SLOT(expandAll()));
    QObject::connect(selectionProxy, SIGNAL(rowsInserted(QModelIndex, int, int)),
                     mainView, SLOT(expandAll()));

    mainView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mainView->setItemDelegate(new ActionListDelegate(mainView));

    mainView->show();
}
*/

int main(int argc, char **argv)
{
    KAboutData about = Zanshin::getAboutData();
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    options.add("debug", ki18n("Show the debug window"));

    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KApplication app;

    ModelStack models;

    if (args->isSet("debug")) {
        DebugWindow *debugWindow = new DebugWindow(&models);
        debugWindow->show();
    }

    MainWindow *mainWindow = new MainWindow(&models);
    mainWindow->show();

    /*
    TodoModel *todoModel = new TodoModel( changeRecorder );
    Akonadi::EntityTreeView *view = new Akonadi::EntityTreeView;
    view->setWindowTitle("TodoModel");
    view->setModel(todoModel);
    view->show();

    createViews<TodoTreeModel>(todoModel);
    createViews<TodoCategoriesModel>(todoModel);
    */

    return app.exec();
}

