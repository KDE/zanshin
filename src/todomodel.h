/* This file is part of Zanshin Todo.

   Copyright 2008-2010 Kevin Ottens <ervin@kde.org>
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

#ifndef ZANSHIN_TODOMODEL_H
#define ZANSHIN_TODOMODEL_H

#include <QtGui/QSortFilterProxyModel>

#include <KDE/Akonadi/EntityTreeModel>
#include <KDE/KCalCore/Todo>

class TodoModel : public Akonadi::EntityTreeModel
{
    Q_OBJECT
    Q_ENUMS(ItemType Roles)

public:
    enum ItemType
    {
        StandardTodo = 0,
        ProjectTodo,
        Category,
        Collection,
        Inbox,
        CategoryRoot
    };

    enum Roles {
        UidRole = Akonadi::EntityTreeModel::UserRole + 1,
        ParentUidRole,
        AncestorsUidRole,
        CategoriesRole,
        ItemTypeRole,
        DataTypeRole,
        ChildUidsRole,
        ChildIndexesRole,
        CategoryPathRole,
        UserRole = Akonadi::EntityTreeModel::UserRole + 100
    };

    enum DataType {
        StandardType = 0,
        CategoryType,
        ProjectType
    };

    TodoModel(Akonadi::ChangeRecorder *monitor, QObject *parent = 0);
    virtual ~TodoModel();

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    virtual int entityColumnCount(HeaderGroup headerGroup) const;
    virtual QVariant entityHeaderData(int section, Qt::Orientation orientation, int role, HeaderGroup headerGroup) const;
    virtual QVariant entityData(const Akonadi::Item &item, int column, int role) const;
    virtual QVariant entityData(const Akonadi::Collection &collection, int column, int role) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    virtual Qt::DropActions supportedDropActions() const;

private slots:
    void onSourceInsertRows(const QModelIndex &sourceIndex, int begin, int end);
    void onSourceRemoveRows(const QModelIndex &sourceIndex, int begin, int end);
    void onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end);

private:
    KCalCore::Todo::Ptr todoFromIndex(const QModelIndex &index) const;
    KCalCore::Todo::Ptr todoFromItem(const Akonadi::Item &item) const;

    ItemType itemTypeFromItem(const Akonadi::Item &item) const;
    QString uidFromItem(const Akonadi::Item &item) const;
    QString relatedUidFromItem(const Akonadi::Item &item) const;
    QStringList ancestorsUidFromItem(const Akonadi::Item &item) const;
    QStringList categoriesFromItem(const Akonadi::Item &item) const;
    QStringList childUidsFromItem(const Akonadi::Item &item) const;
    QModelIndexList childIndexesFromIndex(const QModelIndex &index) const;

    QHash<QString, QString> m_summaryMap;
    QHash<QString, QString> m_parentMap;
    QHash<QString, QStringList> m_childrenMap;
};

Q_DECLARE_METATYPE(QModelIndexList)

#endif

