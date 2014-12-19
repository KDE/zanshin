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

#include <QApplication>
#include <QBoxLayout>
#include <QDockWidget>
#include <QMenu>
#include <QProcess>
#include <QToolBar>
#include <QToolButton>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KComponentData>
#include <KMainWindow>

#include "widgets/applicationcomponents.h"
#include "widgets/availablepagesview.h"
#include "widgets/availablesourcesview.h"
#include "widgets/datasourcecombobox.h"
#include "widgets/editorview.h"
#include "widgets/pageview.h"

#include "presentation/applicationmodel.h"

#include "utils/dependencymanager.h"

#include "dependencies.h"

#include <iostream>

int main(int argc, char **argv)
{
    App::initializeDependencies();

    QApplication app(argc, argv);

    KComponentData mainComponentData("zanshin");

    KSharedConfig::Ptr config = KSharedConfig::openConfig("zanshin-migratorrc");
    KConfigGroup group = config->group("Migrations");
    if (!group.readEntry("Migrated021Projects", false)) {
        std::cerr << "Migrating data from zanshin 0.2, please wait..." << std::endl;
        QProcess proc;
        proc.start("zanshin-migrator");
        proc.waitForFinished();
        std::cerr << "Migration done" << std::endl;
    }

    auto widget = new QWidget;
    auto components = new Widgets::ApplicationComponents(widget);
    components->setModel(Utils::DependencyManager::globalInstance().create<Presentation::ApplicationModel>());

    QVBoxLayout *layout = new QVBoxLayout;

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(components->defaultTaskSourceCombo());
    hbox->addWidget(components->defaultNoteSourceCombo());

    layout->addLayout(hbox);
    layout->addWidget(components->pageView());

    widget->setLayout(layout);

    auto sourcesDock = new QDockWidget(QObject::tr("Sources"));
    sourcesDock->setObjectName("sourcesDock");
    sourcesDock->setWidget(components->availableSourcesView());

    auto pagesDock = new QDockWidget(QObject::tr("Pages"));
    pagesDock->setObjectName("pagesDock");
    pagesDock->setWidget(components->availablePagesView());

    auto editorDock = new QDockWidget(QObject::tr("Editor"));
    editorDock->setObjectName("editorDock");
    editorDock->setWidget(components->editorView());

    auto configureMenu = new QMenu(widget);
    foreach (QAction *action, components->configureActions()) {
        configureMenu->addAction(action);
    }

    auto configureButton = new QToolButton(widget);
    configureButton->setIcon(QIcon::fromTheme("configure"));
    configureButton->setText(QObject::tr("Settings"));
    configureButton->setToolTip(configureButton ->text());
    configureButton->setPopupMode(QToolButton::InstantPopup);
    configureButton->setMenu(configureMenu);

    auto window = new KMainWindow;
    window->resize(1024, 600);
    window->setAutoSaveSettings("MainWindow");
    window->setCentralWidget(widget);

    QToolBar *mainToolBar = window->addToolBar(QObject::tr("Main"));
    mainToolBar->setObjectName("mainToolBar");
    mainToolBar->addWidget(configureButton);

    window->addDockWidget(Qt::RightDockWidgetArea, editorDock);
    window->addDockWidget(Qt::LeftDockWidgetArea, pagesDock);
    window->addDockWidget(Qt::LeftDockWidgetArea, sourcesDock);

    window->show();

    return app.exec();
}
