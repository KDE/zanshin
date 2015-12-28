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

#include <KAboutData>

#include <QListView>
#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>

#include "zanshin/app/dependencies.h"

#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

#include "presentation/tasklistmodel.h"

#include "utils/dependencymanager.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    App::initializeDependencies();
    KAboutData aboutData("tasklister",
                     i18n("Lists all the tasks"), "1.0");
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    auto repository = Utils::DependencyManager::globalInstance().create<Domain::TaskRepository>();
    auto queries = Utils::DependencyManager::globalInstance().create<Domain::TaskQueries>();
    auto taskList = queries->findAll();

    QListView view;
    view.setModel(new Presentation::TaskListModel(taskList, repository, &view));
    view.resize(640, 480);
    view.show();

    return app.exec();
}
