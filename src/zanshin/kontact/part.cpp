/*
 * SPDX-FileCopyrightText: 2011 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "part.h"

#include <KActionCollection>
#include <KPluginFactory>

#include <QAction>
#include <QBoxLayout>
#include <QSplitter>
#include <QStandardPaths>

#include "../app/aboutdata.h"

#include "presentation/applicationmodel.h"

#include "widgets/applicationcomponents.h"
#include "widgets/availablepagesview.h"
#include "widgets/availablesourcesview.h"
#include "widgets/editorview.h"
#include "widgets/pageview.h"

#include "utils/dependencymanager.h"
#include "integration/dependencies.h"

K_PLUGIN_FACTORY(PartFactory, registerPlugin<Part>();)

Part::Part(QWidget *parentWidget, QObject *parent, const QVariantList &)
    : KParts::ReadOnlyPart(parent)
{
    Integration::initializeGlobalAppDependencies();

    setComponentName(QStringLiteral("zanshin"), QStringLiteral("zanshin"));

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

    auto actions = components->globalActions();
    auto ac = actionCollection();
    for (auto it = actions.constBegin(); it != actions.constEnd(); ++it) {
        auto shortcut = it.value()->shortcut();
        if (!shortcut.isEmpty()) {
            ac->setDefaultShortcut(it.value(), shortcut);
        }
        ac->addAction(it.key(), it.value());
    }

    setXMLFile(QStringLiteral("zanshin_part.rc"), true);
}

Part::~Part()
{
}

bool Part::openFile()
{
    return false;
}

#include "part.moc"
