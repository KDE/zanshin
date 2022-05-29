/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "akonadidatasourcerepository.h"

#include <QApplication>

#include <Akonadi/ControlGui>

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
