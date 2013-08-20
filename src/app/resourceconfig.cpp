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

#include "resourceconfig.h"

#include <akonadi/agentfilterproxymodel.h>
#include <akonadi/agentinstance.h>
#include <akonadi/agentinstancewidget.h>
#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agenttypedialog.h>
#include <akonadi/collectionmodel.h>
#include <akonadi/collectionview.h>

#include <KDE/KAction>
#include <KDE/KLocale>
#include <KDE/KMessageBox>
#include <KDE/KStandardGuiItem>

#include <QtCore/QTimer>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QToolBar>
#include <QVBoxLayout>

ResourceConfig::ResourceConfig(QWidget *parent)
    : QWidget(parent)
{
    setLayout(new QVBoxLayout(this));

    QLabel *description = new QLabel(this);
    layout()->addWidget(description);
    description->setWordWrap(true);
    description->setText(i18n("Please select or create a resource which will be used by the application to store and query its TODOs."));

    m_agentInstanceWidget = new Akonadi::AgentInstanceWidget(this);
    m_agentInstanceWidget->agentFilterProxyModel()->addMimeTypeFilter("application/x-vnd.akonadi.calendar.todo");
    layout()->addWidget(m_agentInstanceWidget);

    QHBoxLayout *toolbarLayout = new QHBoxLayout;
    toolbarLayout->setAlignment(Qt::AlignRight);

    QToolBar *toolbar = new QToolBar(this);
    toolbar->setIconSize(QSize(16, 16));
    toolbarLayout->addWidget(toolbar);

    KAction *add = new KAction( KStandardGuiItem::add().icon(),
                                KStandardGuiItem::add().text(),
                                this);
    connect(add, SIGNAL(triggered(bool)), this, SLOT(addResource()));

    KAction *remove = new KAction( KStandardGuiItem::remove().icon(),
                                   KStandardGuiItem::remove().text(),
                                   this);
    connect(remove, SIGNAL(triggered(bool)), this, SLOT(removeResource()));

    KAction *configure = new KAction( KStandardGuiItem::configure().icon(),
                                      KStandardGuiItem::configure().text(),
                                      this);
    connect(configure, SIGNAL(triggered(bool)), this, SLOT(configureResource()));

    toolbar->addAction(add);
    toolbar->addAction(remove);
    toolbar->addAction(configure);

    layout()->addItem(toolbarLayout);
}

void ResourceConfig::addResource()
{
    Akonadi::AgentTypeDialog dlg(this);
    dlg.agentFilterProxyModel()->addMimeTypeFilter("application/x-vnd.akonadi.calendar.todo");
    if (dlg.exec()) {
        const Akonadi::AgentType agentType = dlg.agentType();

        if (agentType.isValid()) {
            Akonadi::AgentInstanceCreateJob *job = new Akonadi::AgentInstanceCreateJob(agentType, this);
            job->configure(this);
            job->start(); // TODO: check result
        }
    }
}

void ResourceConfig::removeResource()
{
    QList<Akonadi::AgentInstance> list = m_agentInstanceWidget->selectedAgentInstances();
    if (!list.isEmpty()) {
        if (KMessageBox::questionYesNo( this,
                                        i18np( "Do you really want to delete the selected agent instance?",
                                               "Do you really want to delete these %1 agent instances?",
                                               list.size() ),
                                        i18n( "Multiple Agent Deletion" ),
                                        KStandardGuiItem::del(),
                                        KStandardGuiItem::cancel(),
                                        QString(),
                                        KMessageBox::Dangerous )
            == KMessageBox::Yes )
        {
            foreach( const Akonadi::AgentInstance &agent, list ) {
                Akonadi::AgentManager::self()->removeInstance( agent );
            }
        }
    }
}

void ResourceConfig::configureResource()
{
    Akonadi::AgentInstance agent = m_agentInstanceWidget->currentAgentInstance();
    if (agent.isValid()) {
        agent.configure(this);
    }
}

