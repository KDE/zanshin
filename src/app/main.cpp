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
#include <QMainWindow>

#include "widgets/applicationcomponents.h"
#include "widgets/availablepagesview.h"
#include "widgets/datasourcecombobox.h"
#include "widgets/editorview.h"
#include "widgets/pageview.h"

#include "presentation/applicationmodel.h"

#include "dependencies.h"

int main(int argc, char **argv)
{
    App::initializeDependencies();

    QApplication app(argc, argv);

    auto widget = new QWidget;
    auto components = new Widgets::ApplicationComponents(widget);
    components->setModel(new Presentation::ApplicationModel(components));

    QVBoxLayout *layout = new QVBoxLayout;

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(components->defaultTaskSourceCombo());
    hbox->addWidget(components->defaultNoteSourceCombo());

    layout->addLayout(hbox);
    layout->addWidget(components->pageView());

    widget->setLayout(layout);

    auto pagesDock = new QDockWidget;
    pagesDock->setWidget(components->availablePagesView());

    auto editorDock = new QDockWidget;
    editorDock->setWidget(components->editorView());

    QMainWindow window;
    window.setCentralWidget(widget);
    window.addDockWidget(Qt::RightDockWidgetArea, editorDock);
    window.addDockWidget(Qt::LeftDockWidgetArea, pagesDock);
    window.show();

    return app.exec();
}
