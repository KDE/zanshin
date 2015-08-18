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

#ifndef AKONADIDATASTOREIMPLEMENTATION_H
#define AKONADIDATASTOREIMPLEMENTATION_H

#include "datastoreinterface.h"

#include <QSortFilterProxyModel>
#include <QSet>
#include <Akonadi/Collection>

class CollectionFilter: public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit CollectionFilter(QObject *parent = 0);
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;

public slots:
    void setActiveCollections(const Akonadi::Collection::List &set);
private:
    Akonadi::Collection::List mActiveCollections;
};


class AkonadiDataStore : public QObject, public DataStoreInterface
{
    Q_OBJECT
public:
    static AkonadiDataStore &instance();

    AkonadiDataStore();
    virtual ~AkonadiDataStore();

    virtual QAbstractItemModel *todoBaseModel();
    virtual QAbstractItemModel *todoCollectionModel();
    virtual QAbstractItemModel *noteBaseModel();
    virtual QAbstractItemModel *noteCollectionModel();

    bool isProject(const Akonadi::Item &item) const;

    virtual PimItem::Ptr indexFromUrl(const KUrl &url) const;

    virtual bool moveTodoToProject(const PimItem::Ptr &item, const PimItem::Ptr &parent);

private:
    QAbstractItemModel *m_todoBaseModel;
    QAbstractItemModel *m_todoCollectionModel;
    QAbstractItemModel *m_noteBaseModel;
    QAbstractItemModel *m_noteCollectionModel;
};

#endif // AKONADIDATASTOREIMPLEMENTATION_H
