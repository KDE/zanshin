/*
 * SPDX-FileCopyrightText: 2014 Mario Bensi <mbensi@ipsquad.net>
   SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include <KAboutData>



#include <QTreeView>
#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>

#include "integration/dependencies.h"

#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

#include "presentation/querytreemodel.h"

#include "utils/dependencymanager.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Integration::initializeGlobalAppDependencies();
    KAboutData aboutData(QStringLiteral("tasktreeviewer"),
                     QStringLiteral("Show all the tasks in tree"), QStringLiteral("1.0"));
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    auto repository = Utils::DependencyManager::globalInstance().create<Domain::TaskRepository>();
    auto queries = Utils::DependencyManager::globalInstance().create<Domain::TaskQueries>();

    auto treeQuery = [&](const Domain::Task::Ptr &task) {
        if (!task)
            return queries->findTopLevel();
        else
            return queries->findChildren(task);
    };

    auto treeFlags = [](const Domain::Task::Ptr &) {
        return Qt::ItemIsSelectable
             | Qt::ItemIsEnabled
             | Qt::ItemIsEditable
             | Qt::ItemIsUserCheckable;
    };

    auto treeData = [](const Domain::Task::Ptr &task, int role, int) -> QVariant {
        if (role != Qt::DisplayRole && role != Qt::CheckStateRole) {
            return QVariant();
        }

        if (role == Qt::DisplayRole)
            return task->title();
        else
            return task->isDone() ? Qt::Checked : Qt::Unchecked;
    };

    auto treeSetData = [&](const Domain::Task::Ptr &task, const QVariant &value, int role) {
        if (role != Qt::EditRole && role != Qt::CheckStateRole) {
            return false;
        }

        if (role == Qt::EditRole) {
            task->setTitle(value.toString());
        } else {
            task->setDone(value.toInt() == Qt::Checked);
        }

        repository->update(task);
        return true;
    };

    QTreeView view;
    view.setModel(new Presentation::QueryTreeModel<Domain::Task::Ptr>(treeQuery, treeFlags, treeData, treeSetData, &view));
    view.resize(640, 480);
    view.show();

    return app.exec();
}
