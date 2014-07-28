/* This file is part of Zanshin

   Copyright 2014 Mario Bensi <mbensi@ipsquad.net>
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
#include <KApplication>
#include <KCmdLineArgs>

#include <QTreeView>

#include "akonadi/akonaditaskqueries.h"
#include "akonadi/akonaditaskrepository.h"
#include "presentation/querytreemodel.h"

int main(int argc, char **argv)
{
    KAboutData about("tasktreeviewer", "tasktreeviewer",
                     ki18n("Show all the tasks in tree"), "1.0");
    KCmdLineArgs::init(argc, argv, &about);
    KApplication app;

    Akonadi::TaskRepository repository;
    Akonadi::TaskQueries queries;

    auto treeQuery = [&](const Domain::Task::Ptr &task) {
        if (!task)
            return queries.findTopLevel();
        else
            return queries.findChildren(task);
    };

    auto treeFlags = [](const Domain::Task::Ptr &) {
        return Qt::ItemIsSelectable
             | Qt::ItemIsEnabled
             | Qt::ItemIsEditable
             | Qt::ItemIsUserCheckable;
    };

    auto treeData = [](const Domain::Task::Ptr &task, int role) -> QVariant {
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

        repository.save(task);
        return true;
    };

    QTreeView view;
    view.setModel(new Presentation::QueryTreeModel(treeQuery, treeFlags, treeData, treeSetData, &view));
    view.resize(640, 480);
    view.show();

    return app.exec();
}
