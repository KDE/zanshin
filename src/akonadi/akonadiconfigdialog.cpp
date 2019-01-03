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
#include <QApplication>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QPointer>
#include <QToolBar>

#include <KLocalizedString>

#include <AkonadiCore/AgentFilterProxyModel>
#include <AkonadiCore/AgentInstance>
#include <AkonadiWidgets/AgentConfigurationDialog>
#include <AkonadiWidgets/AgentInstanceWidget>
#include <AkonadiCore/AgentInstanceCreateJob>
#include <AkonadiCore/AgentManager>
#include <AkonadiWidgets/AgentTypeDialog>

#include <KCalCore/Todo>
#include <Akonadi/Notes/NoteUtils>

using namespace Akonadi;

ConfigDialog::ConfigDialog(StorageInterface::FetchContentTypes types, QWidget *parent)
    : QDialog(parent),
      m_agentInstanceWidget(new Akonadi::AgentInstanceWidget(this)),
      m_types(types)
{
    setWindowTitle(i18n("Configure"));

    auto description = new QLabel(this);
    description->setWordWrap(true);
    description->setText(i18n("Please select or create a resource which will be used by the application to store and query its TODOs."));

    applyContentTypes(m_agentInstanceWidget->agentFilterProxyModel());

    auto toolBar = new QToolBar(this);
    toolBar->setIconSize(QSize(16, 16));
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    auto addAction = new QAction(this);
    addAction->setObjectName(QStringLiteral("addAction"));
    addAction->setText(i18n("Add resource"));
    addAction->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    connect(addAction, &QAction::triggered, this, &ConfigDialog::onAddTriggered);
    toolBar->addAction(addAction);

    auto removeAction = new QAction(this);
    removeAction->setObjectName(QStringLiteral("removeAction"));
    removeAction->setText(i18n("Remove resource"));
    removeAction->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    connect(removeAction, &QAction::triggered, this, &ConfigDialog::onRemoveTriggered);
    toolBar->addAction(removeAction);

    auto configureAction = new QAction(this);
    configureAction->setObjectName(QStringLiteral("settingsAction"));
    configureAction->setText(i18n("Configure resource..."));
    configureAction->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
    connect(configureAction, &QAction::triggered, this, &ConfigDialog::onConfigureTriggered);
    toolBar->addAction(configureAction);

    auto buttons = new QDialogButtonBox(this);
    buttons->setStandardButtons(QDialogButtonBox::Close);
    connect(buttons, &QDialogButtonBox::accepted, this, &ConfigDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &ConfigDialog::reject);

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
    auto dlg = QPointer<AgentTypeDialog>(new AgentTypeDialog(this));
    applyContentTypes(dlg->agentFilterProxyModel());
    if (dlg->exec()) {
        if (!dlg)
            return;

        const auto agentType = dlg->agentType();

        if (agentType.isValid()) {
            auto job = new Akonadi::AgentInstanceCreateJob(agentType, this);
            job->configure(this);
            job->start();
        }
    }
    delete dlg;
}

void ConfigDialog::onRemoveTriggered()
{
    auto list = m_agentInstanceWidget->selectedAgentInstances();
    if (!list.isEmpty()) {
        auto answer = QMessageBox::question(this,
                                            i18n("Multiple Agent Deletion"),
                                            i18n("Do you really want to delete the selected agent instances?"),
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
    if (agent.isValid()) {
        AgentConfigurationDialog dialog(agent, this);
        dialog.exec();
    }
}

void ConfigDialog::applyContentTypes(AgentFilterProxyModel *model)
{
    if (m_types & StorageInterface::Notes)
        model->addMimeTypeFilter(NoteUtils::noteMimeType());
    if (m_types & StorageInterface::Tasks)
        model->addMimeTypeFilter(KCalCore::Todo::todoMimeType());
}

