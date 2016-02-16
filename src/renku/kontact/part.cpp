/* This file is part of Renku Notes

   Copyright 2015 Kevin Ottens <ervin@kde.org>

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

#include "part.h"

#include <KActionCollection>
#include <KPluginFactory>
#include <KStandardDirs>

#include <QAction>
#include <QBoxLayout>
#include <QSplitter>

#include "../app/aboutdata.h"
#include "../app/dependencies.h"

#include "presentation/applicationmodel.h"

#include "widgets/applicationcomponents.h"
#include "widgets/availablepagesview.h"
#include "widgets/availablesourcesview.h"
#include "widgets/editorview.h"
#include "widgets/pageview.h"

#include "utils/dependencymanager.h"

K_PLUGIN_FACTORY(PartFactory, registerPlugin<Part>();)

Part::Part(QWidget *parentWidget, QObject *parent, const QVariantList &)
    : KParts::ReadOnlyPart(parent)
{
    App::initializeDependencies();

    setComponentName(QStringLiteral("renku"), QStringLiteral("renku"));

    auto splitter = new QSplitter(parentWidget);
    auto sidebar = new QSplitter(Qt::Vertical, parentWidget);

    auto components = new Widgets::ApplicationComponents(parentWidget);
    components->setModel(Presentation::ApplicationModel::Ptr::create());

    sidebar->addWidget(components->availablePagesView());
    sidebar->addWidget(components->availableSourcesView());

    splitter->addWidget(sidebar);
    splitter->addWidget(components->pageView());
    splitter->addWidget(components->editorView());
    setWidget(splitter);

    auto actions = QHash<QString, QAction*>();
    actions.unite(components->availableSourcesView()->globalActions());
    actions.unite(components->availablePagesView()->globalActions());
    actions.unite(components->pageView()->globalActions());

    auto ac = actionCollection();
    for (auto it = actions.constBegin(); it != actions.constEnd(); ++it) {
        ac->addAction(it.key(), it.value());
    }

    setXMLFile(KStandardDirs::locate("data", QStringLiteral("renku/renku_part.rc")));
}

Part::~Part()
{
}

bool Part::openFile()
{
    return false;
}
#include "part.moc"
