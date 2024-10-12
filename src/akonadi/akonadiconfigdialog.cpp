/*
 * SPDX-FileCopyrightText: 2011-2015 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

#include <Akonadi/AgentInstanceFilterProxyModel>
#include <Akonadi/AgentInstance>
#include <Akonadi/AgentConfigurationDialog>
#include <Akonadi/AgentInstanceWidget>
#include <Akonadi/AgentInstanceCreateJob>
#include <Akonadi/AgentManager>
#include <Akonadi/AgentTypeDialog>
#include <Akonadi/AgentFilterProxyModel>

#include <KCalendarCore/Todo>

using namespace Akonadi;

ConfigDialog::ConfigDialog(QWidget *parent)
    : QDialog(parent),
      m_agentInstanceWidget(new Akonadi::AgentInstanceWidget(this))
{
    setWindowTitle(i18nc("@title:window", "Configure"));

    auto description = new QLabel(this);
    description->setWordWrap(true);
    description->setText(i18n("Please select or create a resource which will be used by the application to store and query its TODOs."));

    applyContentTypes(m_agentInstanceWidget->agentInstanceFilterProxyModel());

    auto toolBar = new QToolBar(this);
    toolBar->setIconSize(QSize(16, 16));
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    auto addAction = new QAction(this);
    addAction->setObjectName(QLatin1StringView("addAction"));
    addAction->setText(i18n("Add resource"));
    addAction->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    connect(addAction, &QAction::triggered, this, &ConfigDialog::onAddTriggered);
    toolBar->addAction(addAction);

    auto removeAction = new QAction(this);
    removeAction->setObjectName(QLatin1StringView("removeAction"));
    removeAction->setText(i18n("Remove resource"));
    removeAction->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    connect(removeAction, &QAction::triggered, this, &ConfigDialog::onRemoveTriggered);
    toolBar->addAction(removeAction);

    auto configureAction = new QAction(this);
    configureAction->setObjectName(QLatin1StringView("settingsAction"));
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

void ConfigDialog::applyContentTypes(AgentInstanceFilterProxyModel *model)
{
    model->addMimeTypeFilter(KCalendarCore::Todo::todoMimeType());
}

void ConfigDialog::applyContentTypes(AgentFilterProxyModel *model)
{
    model->addMimeTypeFilter(KCalendarCore::Todo::todoMimeType());
}


#include "moc_akonadiconfigdialog.cpp"
