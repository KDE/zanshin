/* This file is part of Zanshin Todo.

   Copyright 2013 Kevin Ottens <ervin@kde.org>
   Copyright 2013 Mario Bensi <nef@ipsquad.net>

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

#include "mockdatastore.h"

#include <QStandardItemModel>

MockDataStore::MockDataStore()
    : m_todoBaseModel(new QStandardItemModel),
      m_todoCollectionModel(new QStandardItemModel),
      m_noteBaseModel(new QStandardItemModel),
      m_noteCollectionModel(new QStandardItemModel)
{
}

MockDataStore::~MockDataStore()
{
    delete m_todoBaseModel;
    delete m_todoCollectionModel;
    delete m_noteBaseModel;
    delete m_noteCollectionModel;
}

QAbstractItemModel *MockDataStore::todoBaseModel()
{
    return m_todoBaseModel;
}

QAbstractItemModel *MockDataStore::todoCollectionModel()
{
    return m_todoCollectionModel;
}

QAbstractItemModel *MockDataStore::noteBaseModel()
{
    return m_noteBaseModel;
}

QAbstractItemModel *MockDataStore::noteCollectionModel()
{
    return m_noteCollectionModel;
}

PimItem::Ptr MockDataStore::indexFromUrl(const KUrl &url) const
{
    Q_UNUSED(url)
    return PimItem::Ptr();
}

bool MockDataStore::moveTodoToProject(const PimItem::Ptr &/*item*/, const PimItem::Ptr &/*parent*/)
{
    return true;
}

