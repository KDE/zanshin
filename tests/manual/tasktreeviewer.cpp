/* This file is part of Zanshin

   Copyright 2014 Mario Bensi <mbensi@ipsquad.net>

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
#include "presentation/tasktreemodel.h"

int main(int argc, char **argv)
{
    KAboutData about("tasktreeviewer", "tasktreeviewer",
                     ki18n("Show all the tasks in tree"), "1.0");
    KCmdLineArgs::init(argc, argv, &about);
    KApplication app;

    Akonadi::TaskRepository repository;
    Akonadi::TaskQueries queries;

    QTreeView view;
    view.setModel(new Presentation::TaskTreeModel([&]{ return queries.findTopLevel(); },
                                                  &queries, &repository, &view));
    view.resize(640, 480);
    view.show();

    return app.exec();
}
