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


#include "tasklistmodel.h"

#include "domain/taskrepository.h"

using namespace Presentation;

TaskListModel::TaskListModel(const TaskList::Ptr &taskList, const Domain::TaskRepository::Ptr &repository, QObject *parent)
    : QAbstractListModel(parent),
      m_taskList(taskList),
      m_repository(repository)
{
    m_taskList->addPreInsertHandler([this](const Domain::Task::Ptr &, int index) {
                                        beginInsertRows(QModelIndex(), index, index);
                                    });
    m_taskList->addPostInsertHandler([this](const Domain::Task::Ptr &, int) {
                                         endInsertRows();
                                     });
    m_taskList->addPreRemoveHandler([this](const Domain::Task::Ptr &, int index) {
                                        beginRemoveRows(QModelIndex(), index, index);
                                    });
    m_taskList->addPostRemoveHandler([this](const Domain::Task::Ptr &, int) {
                                         endRemoveRows();
                                     });
    m_taskList->addPostReplaceHandler([this](const Domain::Task::Ptr &, int idx) {
                                         emit dataChanged(index(idx), index(idx));
                                     });
}

TaskListModel::~TaskListModel()
{
}

Qt::ItemFlags TaskListModel::flags(const QModelIndex &index) const
{
    if (!isModelIndexValid(index)) {
        return Qt::NoItemFlags;
    }

    return QAbstractListModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
}

int TaskListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return m_taskList->data().size();
}

QVariant TaskListModel::data(const QModelIndex &index, int role) const
{
    if (!isModelIndexValid(index)) {
        return QVariant();
    }

    if (role != Qt::DisplayRole && role != Qt::CheckStateRole) {
        return QVariant();
    }

    const auto task = taskForIndex(index);
    if (role == Qt::DisplayRole)
        return task->title();
    else
        return task->isDone() ? Qt::Checked : Qt::Unchecked;
}

bool TaskListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!isModelIndexValid(index)) {
        return false;
    }

    if (role != Qt::EditRole && role != Qt::CheckStateRole) {
        return false;
    }

    auto task = taskForIndex(index);
    if (role == Qt::EditRole) {
        task->setTitle(value.toString());
    } else {
        task->setDone(value.toInt() == Qt::Checked);
    }

    m_repository->update(task);
    return true;
}

Domain::Task::Ptr TaskListModel::taskForIndex(const QModelIndex &index) const
{
    return m_taskList->data().at(index.row());
}

bool TaskListModel::isModelIndexValid(const QModelIndex &index) const
{
    return index.isValid()
        && index.column() == 0
        && index.row() >= 0
        && index.row() < m_taskList->data().size();
}
