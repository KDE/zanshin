/* This file is part of Zanshin Todo.

   Copyright 2008-2010 Kevin Ottens <ervin@kde.org>
   Copyright 2008,2009 Mario Bensi <nef@ipsquad.net>

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

#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KConfigGroup>
#include <KDE/KIcon>
#include <KDE/KLocale>
#include <KDE/KMenuBar>

#include <QtGui/QDockWidget>
#include <QtGui/QHeaderView>

#include "actionlisteditor.h"
#include "configdialog.h"
#include "globaldefs.h"
#include "sidebar.h"

MainWindow::MainWindow(ModelStack *models, QWidget *parent)
    : KXmlGuiWindow(parent)
{
    setupSideBar(models);
    setupCentralWidget(models);
    setupActions();

    setupGUI(ToolBar | Keys | Save | Create);

    restoreColumnsState();

    actionCollection()->action("project_mode")->trigger();
}

void MainWindow::setupCentralWidget(ModelStack *models)
{
    m_editor = new ActionListEditor(models,
                                    m_sidebar->projectSelection(),
                                    m_sidebar->categoriesSelection(),
                                    actionCollection(),
                                    this);
    setCentralWidget(m_editor);
}

void MainWindow::setupSideBar(ModelStack *models)
{
    m_sidebar = new SideBar(models, actionCollection(), this);

    QDockWidget *dock = new QDockWidget(this);
    dock->setObjectName("SideBar");
    dock->setFeatures(dock->features() & ~QDockWidget::DockWidgetClosable);
    dock->setWidget(m_sidebar);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
}

void MainWindow::setupActions()
{
    KActionCollection *ac = actionCollection();

    KAction *action = ac->addAction(KStandardAction::ShowMenubar);
    connect(action, SIGNAL(toggled(bool)),
            menuBar(), SLOT(setVisible(bool)));

    QActionGroup *modeGroup = new QActionGroup(this);
    modeGroup->setExclusive(true);

    action = ac->addAction("project_mode", this, SLOT(onModeSwitch()));
    action->setText(i18n("Project View"));
    action->setIcon(KIcon("view-pim-tasks"));
    action->setShortcut(Qt::CTRL | Qt::Key_P);
    action->setCheckable(true);
    action->setData(Zanshin::ProjectMode);
    modeGroup->addAction(action);

    action = ac->addAction("categories_mode", this, SLOT(onModeSwitch()));
    action->setText(i18n("Categories View"));
    action->setIcon(KIcon("view-pim-notes"));
    action->setShortcut(Qt::CTRL | Qt::Key_O);
    action->setCheckable(true);
    action->setData(Zanshin::CategoriesMode);
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
    m_editor->saveColumnsState(cg);
}

void MainWindow::restoreColumnsState()
{
    KConfigGroup cg = autoSaveConfigGroup();
    m_editor->restoreColumnsState(cg);
}

void MainWindow::onModeSwitch()
{
    KAction *action = static_cast<KAction*>(sender());
    m_editor->setMode((Zanshin::ApplicationMode)action->data().toInt());
    m_sidebar->setMode((Zanshin::ApplicationMode)action->data().toInt());
}

void MainWindow::showConfigDialog()
{
    ConfigDialog dialog(this);
    dialog.exec();
}
