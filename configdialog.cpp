/* This file is part of Zanshin Todo.

   Copyright 2009 Kevin Ottens <ervin@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

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

#include <akonadi/collectionmodel.h>
#include <akonadi/collectionview.h>

#include <KDE/KLocale>

#include <QtGui/QLayout>

#include "globalmodel.h"
#include "globalsettings.h"

ConfigDialog::ConfigDialog(QWidget *parent, const QString &name, GlobalSettings *settings)
    : KConfigDialog(parent, name, settings), m_settings(settings)
{
    setFaceType(Plain);

    QWidget *page = new QWidget(this);
    page->setLayout(new QVBoxLayout(page));

    m_collectionList = new Akonadi::CollectionView(page);
    page->layout()->addWidget(m_collectionList);
    m_collectionList->setObjectName("kcfg_collectionId");
    connect(m_collectionList, SIGNAL(clicked(const Akonadi::Collection &)),
            this, SLOT(_k_updateButtons()));

    m_collectionList->setModel(GlobalModel::todoCollections());

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

    QAbstractItemModel *model = m_collectionList->model();
    for (int row=0; row<model->rowCount(); row++) {
        QModelIndex index = model->index(row, 0);
        if (model->data(index, Akonadi::CollectionModel::CollectionIdRole).toInt()==id) {
            m_collectionList->setCurrentIndex(index);
            break;
        }
    }
}

void ConfigDialog::updateWidgetsDefault()
{
    QModelIndex index = m_collectionList->model()->index(0, 0);
    m_collectionList->setCurrentIndex(index);
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
