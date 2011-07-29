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

#include "todomodel.h"

#include <KDE/KCalCore/Todo>

#include <KDebug>
#include <KIcon>
#include <KLocale>

#include <KDE/Akonadi/ItemModifyJob>

TodoModel::TodoModel(Akonadi::ChangeRecorder *monitor, QObject *parent)
    : Akonadi::EntityTreeModel(monitor, parent)
{
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(onSourceInsertRows(QModelIndex,int,int)));
    connect(this, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(onSourceRemoveRows(QModelIndex,int,int)));
    connect(this, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(onSourceDataChanged(QModelIndex,QModelIndex)));

    onSourceInsertRows(QModelIndex(), 0, rowCount()-1);
}

TodoModel::~TodoModel()
{
}

Qt::ItemFlags TodoModel::flags(const QModelIndex &index) const
{
    return Akonadi::EntityTreeModel::flags(index) | Qt::ItemIsEditable;
}

int TodoModel::entityColumnCount(HeaderGroup headerGroup) const
{
    if (headerGroup == CollectionTreeHeaders) {
        return 1;
    } else {
        return 5;
    }
}

QVariant TodoModel::entityHeaderData(int section, Qt::Orientation orientation, int role, HeaderGroup headerGroup) const
{
    if (orientation == Qt::Vertical) {
        return EntityTreeModel::entityHeaderData(section, orientation, role, headerGroup);
    }

    if (headerGroup == CollectionTreeHeaders) {
        return i18n("Summary");
    } else if (role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return i18n("Summary");
        case 1:
            return i18n("Project");
        case 2:
            return i18n("Categories");
        case 3:
            return i18n("Due Date");
        case 4:
            return i18n("Collection");
        }
    }

    return EntityTreeModel::entityHeaderData(section, orientation, role, headerGroup);
}

QVariant TodoModel::entityData(const Akonadi::Item &item, int column, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        switch (column) {
        case 0:
            return todoFromItem(item)->summary();
        case 1:
            return m_summaryMap[todoFromItem(item)->relatedTo()];
        case 2:
            return todoFromItem(item)->categories().join(", ");
        case 3:
            return todoFromItem(item)->dtDue().date().toString();
        case 4:
            return modelIndexForCollection(this, item.parentCollection()).data();
        }
    case Qt::EditRole:
        switch (column) {
        case 0:
            return todoFromItem(item)->summary();
        case 1:
            return m_summaryMap[todoFromItem(item)->relatedTo()];
        case 2:
            return todoFromItem(item)->categories();
        case 3:
            return todoFromItem(item)->dtDue().date();
        case 4:
            return modelIndexForCollection(this, item.parentCollection()).data();
        }
    case Qt::DecorationRole:
        if (column==4) {
            return modelIndexForCollection(this, item.parentCollection()).data(Qt::DecorationRole);
        } else {
            return EntityTreeModel::entityData(item, column, role);
        }
    default:
        return EntityTreeModel::entityData(item, column, role);
    }
}

bool TodoModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if ((role!=Qt::EditRole && role!=Qt::CheckStateRole) || !index.isValid()) {
        return EntityTreeModel::setData(index, value, role);
    }

    Akonadi::Item item = data(index, ItemRole).value<Akonadi::Item>();

    if (!item.isValid() || !item.hasPayload<KCalCore::Todo::Ptr>()) {
        return EntityTreeModel::setData(index, value, role);
    }

    bool shouldModifyItem = false;

    KCalCore::Todo::Ptr todo = todoFromItem(item);

    switch (index.column()) {
    case 0:
        if (role==Qt::EditRole) {
            todo->setSummary(value.toString());
            shouldModifyItem = true;
        } else if (role==Qt::CheckStateRole) {
            todo->setCompleted(value.toInt()==Qt::Checked);
            shouldModifyItem = true;
        }
        break;
    case 1:
        todo->setRelatedTo(value.toString());
        shouldModifyItem = true;
        break;
    case 2:
        todoFromItem(item)->setCategories(value.toStringList());
        shouldModifyItem = true;
        break;
    case 3:
        todo->setDtDue(KDateTime(value.toDate()));
        todo->setHasDueDate(true);
        todo->setAllDay(true);
        shouldModifyItem = true;
        break;
    case 4:
        break;
    }

    if (shouldModifyItem) {
        Akonadi::ItemModifyJob *itemModifyJob = new Akonadi::ItemModifyJob( item, this );
        connect(itemModifyJob, SIGNAL(result(KJob*)),
                 this, SLOT(updateJobDone(KJob*)));
    }

    return false;
}

void TodoModel::onSourceInsertRows(const QModelIndex &parent, int begin, int end)
{
    for (int i = begin; i <= end; i++) {
        QModelIndex child = index(i, 0, parent);
        onSourceInsertRows(child, 0, rowCount(child)-1);

        KCalCore::Todo::Ptr todo = todoFromIndex(child);

        if (!todo) {
            continue;
        }

        QString uid = todo->uid();

        m_summaryMap[uid] = todo->summary();
    }
}

void TodoModel::onSourceRemoveRows(const QModelIndex &parent, int begin, int end)
{
    for (int i = begin; i <= end; ++i) {
        QModelIndex child = index(i, 0, parent);
        KCalCore::Todo::Ptr todo = todoFromIndex(child);

        if (!todo) {
            continue;
        }

        QString uid = todo->uid();

        m_summaryMap.remove(uid);
    }
}

void TodoModel::onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end)
{
    for (int row = begin.row(); row <= end.row(); ++row) {
        for (int column = begin.column(); column <= end.column(); ++column) {
            KCalCore::Todo::Ptr todo = todoFromIndex( index(row, column, begin.parent()) );

            if (!todo) {
                continue;
            }

            QString uid = todo->uid();

            m_summaryMap[uid] = todo->summary();
        }
    }
}

KCalCore::Todo::Ptr TodoModel::todoFromIndex(const QModelIndex &index) const
{
    Akonadi::Item item = data(index, ItemRole).value<Akonadi::Item>();
    return todoFromItem(item);
}

KCalCore::Todo::Ptr TodoModel::todoFromItem(const Akonadi::Item &item) const
{
    if (!item.isValid() || !item.hasPayload<KCalCore::Todo::Ptr>()) {
        return KCalCore::Todo::Ptr();
    } else {
        return item.payload<KCalCore::Todo::Ptr>();
    }
}

Qt::DropActions TodoModel::supportedDropActions() const
{
    return Qt::MoveAction;
}
