/* This file is part of Zanshin Todo.

   Copyright 2011 Kevin Ottens <ervin@kde.org>

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

#include <KDE/KConfigGroup>
#include <KDE/KPluginFactory>
#include <KDE/KStandardDirs>

#include <QtGui/QSplitter>

#include "aboutdata.h"
#include "actionlisteditor.h"
#include "maincomponent.h"
#include "core/modelstack.h"
#include "sidebar.h"

K_PLUGIN_FACTORY(PartFactory, registerPlugin<Part>();)
K_EXPORT_PLUGIN(PartFactory(Zanshin::getAboutData()))

Part::Part(QWidget *parentWidget, QObject *parent, const QVariantList &)
    : KParts::ReadOnlyPart(parent),
      m_models(new ModelStack(this)),
      m_splitter(new QSplitter(parentWidget)),
      m_component(new MainComponent(m_models, m_splitter, this))
{
    m_splitter->addWidget(m_component->sideBar());
    m_splitter->addWidget(m_component->editor());

    setComponentData(PartFactory::componentData());
    setWidget(m_splitter);
    setXMLFile(KStandardDirs::locate("data", "zanshin/zanshin_part.rc"));

    KConfigGroup cg(componentData().config(), "KontactPart");
    m_component->restoreColumnsState(cg);
}

Part::~Part()
{
    KConfigGroup cg(componentData().config(), "KontactPart");
    m_component->saveColumnsState(cg);
}

bool Part::openFile()
{
    return false;
}

