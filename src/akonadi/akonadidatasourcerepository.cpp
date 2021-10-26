/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>

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


#include "akonadidatasourcerepository.h"

#include <QApplication>

#include <AkonadiWidgets/Akonadi/ControlGui>

#include "akonadiconfigdialog.h"

using namespace Akonadi;

DataSourceRepository::DataSourceRepository(const StorageInterface::Ptr &storage,
                                           const SerializerInterface::Ptr &serializer)
    : m_storage(storage),
      m_serializer(serializer)
{
}

KJob *DataSourceRepository::update(Domain::DataSource::Ptr source)
{
    auto collection = m_serializer->createCollectionFromDataSource(source);
    Q_ASSERT(collection.isValid());
    return m_storage->updateCollection(collection, this);
}

void DataSourceRepository::showConfigDialog()
{
    ConfigDialog dialog(qApp->activeWindow());
    dialog.exec();
}

void DataSourceRepository::windowNeedsDataBackend(QWidget *window)
{
    Akonadi::ControlGui::widgetNeedsAkonadi(window);
}
