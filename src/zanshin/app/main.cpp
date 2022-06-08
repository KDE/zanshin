/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QApplication>
#include <QBoxLayout>
#include <QDockWidget>
#include <QProcess>
#include <QAction>

#include <KActionCollection>


#include <KXmlGuiWindow>

#include <KConfigGroup>
#include <KSharedConfig>

#include "widgets/applicationcomponents.h"
#include "widgets/availablepagesview.h"
#include "widgets/availablesourcesview.h"
#include "widgets/editorview.h"
#include "widgets/pageview.h"

#include "presentation/applicationmodel.h"

#include "utils/dependencymanager.h"
#include "integration/dependencies.h"

#include "aboutdata.h"

#include <iostream>
#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>

int main(int argc, char **argv)
{
    KLocalizedString::setApplicationDomain("zanshin");
    QApplication app(argc, argv);
    Integration::initializeGlobalAppDependencies();

    auto aboutData = App::getAboutData();
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("zanshin-migratorrc"));
    KConfigGroup group = config->group("Migrations");
    if (!group.readEntry("MigratedTags", false)) {
        std::cerr << "Migrating tags, please wait..." << std::endl;
        QProcess proc;
        proc.start(QStringLiteral("zanshin-migrator"), QStringList());
        proc.waitForFinished(-1);
        if (proc.exitStatus() == QProcess::CrashExit) {
            std::cerr << "Migrator crashed!" << std::endl;
        } else if (proc.exitCode() == 0) {
            std::cerr << "Migration done" << std::endl;
        } else {
            std::cerr << "Migration error, code" << proc.exitCode() << std::endl;
        }
    }

    auto widget = new QWidget;
    auto components = new Widgets::ApplicationComponents(widget);
    components->setModel(Presentation::ApplicationModel::Ptr::create());

    auto layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(components->pageView());
    widget->setLayout(layout);

    auto sourcesDock = new QDockWidget(i18n("Sources"));
    sourcesDock->setObjectName(QStringLiteral("sourcesDock"));
    sourcesDock->setWidget(components->availableSourcesView());

    auto pagesDock = new QDockWidget(i18n("Pages"));
    pagesDock->setObjectName(QStringLiteral("pagesDock"));
    pagesDock->setWidget(components->availablePagesView());

    auto editorDock = new QDockWidget(i18n("Editor"));
    editorDock->setObjectName(QStringLiteral("editorDock"));
    editorDock->setWidget(components->editorView());

    auto window = new KXmlGuiWindow;
    window->setCentralWidget(widget);

    window->addDockWidget(Qt::RightDockWidgetArea, editorDock);
    window->addDockWidget(Qt::LeftDockWidgetArea, pagesDock);
    window->addDockWidget(Qt::LeftDockWidgetArea, sourcesDock);

    auto actions = components->globalActions();
    actions.insert(QStringLiteral("dock_sources"), sourcesDock->toggleViewAction());
    actions.insert(QStringLiteral("dock_pages"), pagesDock->toggleViewAction());
    actions.insert(QStringLiteral("dock_editor"), editorDock->toggleViewAction());

    auto ac = window->actionCollection();
    ac->addAction(KStandardAction::Quit, window, SLOT(close()));
    for (auto it = actions.constBegin(); it != actions.constEnd(); ++it) {
        auto shortcut = it.value()->shortcut();
        if (!shortcut.isEmpty()) {
            ac->setDefaultShortcut(it.value(), shortcut);
        }
        ac->addAction(it.key(), it.value());
    }

    window->setupGUI(QSize(1024, 600),
                     KXmlGuiWindow::ToolBar
                   | KXmlGuiWindow::Keys
                   | KXmlGuiWindow::Save
                   | KXmlGuiWindow::Create);

    delete window->findChild<QAction*>("help_contents");
    delete window->findChild<QAction*>("help_whats_this");

    window->show();

    {
        auto &deps = Utils::DependencyManager::globalInstance();
        auto repo = deps.create<Domain::DataSourceRepository>();
        repo->windowNeedsDataBackend(window);
    }

    return app.exec();
}
