/* This file is part of Zanshin

   Copyright 2011-2015 Kevin Ottens <ervin@kde.org>

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

#include "akonadiconfigdialog.h"

#include <QAction>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QToolBar>

#include <AkonadiCore/AgentFilterProxyModel>
#include <AkonadiCore/AgentInstance>
#include <AkonadiWidgets/AgentInstanceWidget>
#include <AkonadiCore/AgentInstanceCreateJob>
#include <AkonadiCore/AgentManager>
#include <AkonadiWidgets/AgentTypeDialog>

using namespace Akonadi;

ConfigDialog::ConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Configure"));

    auto description = new QLabel(this);
    description->setWordWrap(true);
    description->setText(tr("Please select or create a resource which will be used by the application to store and query its TODOs."));

    m_agentInstanceWidget = new Akonadi::AgentInstanceWidget(this);
    m_agentInstanceWidget->agentFilterProxyModel()->addMimeTypeFilter("application/x-vnd.akonadi.calendar.todo");

    auto toolBar = new QToolBar(this);
    toolBar->setIconSize(QSize(16, 16));
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    auto addAction = new QAction(this);
    addAction->setObjectName("addAction");
    addAction->setText(tr("Add resource"));
    addAction->setIcon(QIcon::fromTheme("list-add"));
    connect(addAction, SIGNAL(triggered()), this, SLOT(onAddTriggered()));
    toolBar->addAction(addAction);

    auto removeAction = new QAction(this);
    removeAction->setObjectName("removeAction");
    removeAction->setText(tr("Remove resource"));
    removeAction->setIcon(QIcon::fromTheme("list-remove"));
    connect(removeAction, SIGNAL(triggered()), this, SLOT(onRemoveTriggered()));
    toolBar->addAction(removeAction);

    auto configureAction = new QAction(this);
    configureAction->setObjectName("settingsAction");
    configureAction->setText(tr("Configure resource..."));
    configureAction->setIcon(QIcon::fromTheme("configure"));
    connect(configureAction, SIGNAL(triggered()), this, SLOT(onConfigureTriggered()));
    toolBar->addAction(configureAction);

    auto buttons = new QDialogButtonBox(this);
    buttons->setStandardButtons(QDialogButtonBox::Close);
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    auto layout = new QVBoxLayout;
    layout->addWidget(description);
    layout->addWidget(m_agentInstanceWidget);

    auto toolBarLayout = new QHBoxLayout;
    toolBarLayout->setAlignment(Qt::AlignRight);
    toolBarLayout->addWidget(toolBar);
    layout->addLayout(toolBarLayout);

    layout->addWidget(buttons);

    setLayout(layout);
}

void ConfigDialog::onAddTriggered()
{
    Akonadi::AgentTypeDialog dlg(this);
    dlg.agentFilterProxyModel()->addMimeTypeFilter("application/x-vnd.akonadi.calendar.todo");
    if (dlg.exec()) {
        const auto agentType = dlg.agentType();

        if (agentType.isValid()) {
            auto job = new Akonadi::AgentInstanceCreateJob(agentType, this);
            job->configure(this);
            job->start();
        }
    }
}

void ConfigDialog::onRemoveTriggered()
{
    auto list = m_agentInstanceWidget->selectedAgentInstances();
    if (!list.isEmpty()) {
        auto answer = QMessageBox::question(this,
                                            tr("Multiple Agent Deletion"),
                                            tr("Do you really want to delect the selected agent instances?"),
                                            QMessageBox::Yes | QMessageBox::No,
                                            QMessageBox::No);
        if (answer == QMessageBox::Yes) {
            foreach (const auto &agent, list) {
                Akonadi::AgentManager::self()->removeInstance(agent);
            }
        }
    }
}

void ConfigDialog::onConfigureTriggered()
{
    auto agent = m_agentInstanceWidget->currentAgentInstance();
    if (agent.isValid())
        agent.configure(this);
}

