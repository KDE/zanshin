/* This file is part of Zanshin Todo.

   Copyright 2009 Kevin Ottens <ervin@kde.org>

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

#include "configdialog.h"

#include <akonadi/agentfilterproxymodel.h>
#include <akonadi/agentinstance.h>
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

#include "globalmodel.h"
#include "globalsettings.h"

ConfigDialog::ConfigDialog(QWidget *parent, const QString &name, GlobalSettings *settings)
    : KConfigDialog(parent, name, settings), m_settings(settings)
{
    setFaceType(Plain);

    QWidget *page = new QWidget(this);
    page->setLayout(new QVBoxLayout(page));

    QLabel *description = new QLabel(page);
    page->layout()->addWidget(description);
    description->setWordWrap(true);
    description->setText(i18n("Please select or create a resource which will be used by the application to store and query its TODOs."));

    m_collectionList = new Akonadi::CollectionView(page);
    page->layout()->addWidget(m_collectionList);
    m_collectionList->setObjectName("kcfg_collectionId");
    connect(m_collectionList, SIGNAL(clicked(const Akonadi::Collection &)),
            this, SLOT(_k_updateButtons()));

    m_collectionList->setModel(GlobalModel::todoCollections());
    m_collectionList->expandAll();

    QHBoxLayout *toolbarLayout = new QHBoxLayout;
    toolbarLayout->setAlignment(Qt::AlignRight);

    QToolBar *toolbar = new QToolBar(page);
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

    page->layout()->addItem(toolbarLayout);

    addPage(page, i18n("Resources"), QString(), QString(), false);

    updateWidgets();
}

void ConfigDialog::updateSettings()
{
    m_settings->setCollectionId(selectedCollection());
    m_settings->writeConfig();
    emit settingsChanged(objectName());
}

void ConfigDialog::updateWidgets()
{
    Akonadi::Collection::Id id = m_settings->collectionId();
    selectCollection(id);
}

void ConfigDialog::updateWidgetsDefault()
{
    QModelIndex index = m_collectionList->model()->index(0, 0);
    m_collectionList->setCurrentIndex(index);
}

bool ConfigDialog::selectCollection(Akonadi::Collection::Id id, const QModelIndex &parentIndex)
{
    const QAbstractItemModel *model = m_collectionList->model();

    for (int row=0; row<model->rowCount(parentIndex); row++) {
        QModelIndex index = model->index(row, 0, parentIndex);
        if (model->data(index, Akonadi::CollectionModel::CollectionIdRole).toInt()==id) {
            m_collectionList->setCurrentIndex(index);
            return true;
        } else if (model->rowCount(index)>0
                && selectCollection(id, index)) {
            return true;
        }
    }

    return false;
}

Akonadi::Collection::Id ConfigDialog::selectedCollection()
{
    QModelIndex current = m_collectionList->currentIndex();
    return m_collectionList->model()->data(current, Akonadi::CollectionModel::CollectionIdRole).toInt();
}

bool ConfigDialog::hasChanged()
{
    return m_settings->collectionId()!=selectedCollection();
}

bool ConfigDialog::isDefault()
{
    QModelIndex current = m_collectionList->currentIndex();
    return current.isValid() && (current.row()==0);
}

void ConfigDialog::addResource()
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

void ConfigDialog::removeResource()
{
    const QModelIndex current = m_collectionList->currentIndex();
    const Akonadi::Collection collection = m_collectionList->model()->data(current, Akonadi::CollectionModel::CollectionRole).value<Akonadi::Collection>();
    Akonadi::AgentInstance agent = Akonadi::AgentManager::self()->instance(collection.resource());

    if ( agent.isValid() ) {
        if ( KMessageBox::questionYesNo( this,
                                         i18n( "Do you really want to delete agent instance %1?", agent.name() ),
                                         i18n( "Agent Deletion" ),
                                         KStandardGuiItem::del(),
                                         KStandardGuiItem::cancel(),
                                         QString(),
                                         KMessageBox::Dangerous )
             == KMessageBox::Yes )
        {
            Akonadi::AgentManager::self()->removeInstance( agent );
        }
    }
}

void ConfigDialog::configureResource()
{
    const QModelIndex current = m_collectionList->currentIndex();
    const Akonadi::Collection collection = m_collectionList->model()->data(current, Akonadi::CollectionModel::CollectionRole).value<Akonadi::Collection>();
    Akonadi::AgentInstance agent = Akonadi::AgentManager::self()->instance(collection.resource());

    if ( agent.isValid() )
        agent.configure(this);
}
