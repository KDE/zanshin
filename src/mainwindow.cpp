/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>
   Copyright 2008, 2009 Mario Bensi <nef@ipsquad.net>

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

#include "mainwindow.h"

#include <akonadi/control.h>

#include <akonadi/collectionfetchjob.h>

#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KConfigGroup>
#include <KDE/KIcon>
#include <KDE/KLocale>

#include <QtGui/QDockWidget>
#include <QtGui/QHeaderView>

#include "actionlisteditor.h"
#include "actionlistview.h"
#include "configdialog.h"
#include "globalmodel.h"
#include "globalsettings.h"
#include "todoflatmodel.h"
#include "sidebar.h"

MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent)
{
    Akonadi::Control::start();

    setupSideBar();
    setupCentralWidget();
    setupActions();

    setupGUI();

    restoreColumnState();
    applySettings();

    actionCollection()->action("project_mode")->trigger();
}

void MainWindow::setupCentralWidget()
{
    m_editor = new ActionListEditor(this, actionCollection());

    connect(m_sidebar, SIGNAL(noProjectInboxActivated()),
            m_editor, SLOT(showNoProjectInbox()));
    connect(m_sidebar, SIGNAL(projectActivated(QModelIndex)),
            m_editor, SLOT(focusOnProject(QModelIndex)));
    connect(m_sidebar, SIGNAL(noContextInboxActivated()),
            m_editor, SLOT(showNoContextInbox()));
    connect(m_sidebar, SIGNAL(contextActivated(QModelIndex)),
            m_editor, SLOT(focusOnContext(QModelIndex)));

    setCentralWidget(m_editor);
}

void MainWindow::setupSideBar()
{
    m_sidebar = new SideBar(this, actionCollection());

    QDockWidget *dock = new QDockWidget(this);
    dock->setObjectName("SideBar");
    dock->setFeatures(dock->features() & ~QDockWidget::DockWidgetClosable);
    dock->setWidget(m_sidebar);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
}

void MainWindow::setupActions()
{
    KActionCollection *ac = actionCollection();

    QActionGroup *modeGroup = new QActionGroup(this);
    modeGroup->setExclusive(true);

    KAction *action = ac->addAction("project_mode", m_sidebar, SLOT(switchToProjectMode()));
    action->setText(i18n("Project Mode"));
    action->setIcon(KIcon("view-pim-tasks"));
    action->setShortcut(Qt::CTRL | Qt::Key_P);
    action->setCheckable(true);
    modeGroup->addAction(action);

    action = ac->addAction("context_mode", m_sidebar, SLOT(switchToContextMode()));
    action->setText(i18n("Context Mode"));
    action->setIcon(KIcon("view-pim-notes"));
    action->setShortcut(Qt::CTRL | Qt::Key_O);
    action->setCheckable(true);
    modeGroup->addAction(action);

    ac->addAction(KStandardAction::Preferences, this, SLOT(showConfigDialog()));
    ac->addAction(KStandardAction::Quit, this, SLOT(close()));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveColumnsState();
    KXmlGuiWindow::closeEvent(event);
}

void MainWindow::saveAutoSaveSettings()
{
    saveColumnsState();
    KXmlGuiWindow::saveAutoSaveSettings();
}

void MainWindow::saveColumnsState()
{
    KConfigGroup cg = autoSaveConfigGroup();
    QByteArray state = m_editor->view()->header()->saveState();
    cg.writeEntry("MainHeaderState", state.toBase64());
}

void MainWindow::restoreColumnState()
{
    KConfigGroup cg = autoSaveConfigGroup();
    QByteArray state;
    if (cg.hasKey("MainHeaderState")) {
        state = cg.readEntry("MainHeaderState", state);
        m_editor->view()->header()->restoreState(QByteArray::fromBase64(state));
    }
}

void MainWindow::showConfigDialog()
{
    if (KConfigDialog::showDialog("settings")) {
        return;
    }

    ConfigDialog *dialog = new ConfigDialog(this, "settings",
                                            GlobalSettings::self());

    connect(dialog, SIGNAL(settingsChanged(const QString&)),
            this, SLOT(applySettings()));

    dialog->show();
}

void MainWindow::applySettings()
{
    Akonadi::Collection collection(GlobalSettings::collectionId());

    if (!collection.isValid()) return;

    Akonadi::CollectionFetchJob *job =  new Akonadi::CollectionFetchJob(collection, Akonadi::CollectionFetchJob::Base);
    job->exec();

    if (job->collections().isEmpty()) return;

    GlobalModel::todoFlat()->setCollection(job->collections().first());
}
