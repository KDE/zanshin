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


#include "pagemodel.h"

#include "presentation/querytreemodel.h"

#include <klocalizedstring.h>

using namespace Presentation;

PageModel::PageModel(QObject *parent)
    : QObject(parent),
      m_centralListModel(nullptr)
{
}

QAbstractItemModel *PageModel::centralListModel()
{
    if (!m_centralListModel)
        m_centralListModel = createCentralListModel();
    return m_centralListModel;
}

PageModel::TaskExtraDataPtr PageModel::fetchTaskExtraData(Domain::TaskQueries::Ptr taskQueries,
                                                          const TaskExtraParts &parts,
                                                          const QModelIndex &index,
                                                          const Domain::Task::Ptr &task)
{
    TaskExtraDataPtr info = TaskExtraDataPtr::create();
    if (index.parent().isValid()) {
        info->isChildTask = true;
    }

    // children are in the same data source as their parent
    if (!info->isChildTask && parts.testFlag(TaskExtraPart::DataSource)) {
        info->dataSourceQueryResult = taskQueries->findDataSource(task);
        if (info->dataSourceQueryResult) {
            QPersistentModelIndex persistentIndex(index);
            info->dataSourceQueryResult->addPostInsertHandler([persistentIndex](const Domain::DataSource::Ptr &, int) {
                // When a data source was found (inserted into the result), update the rendering of the item
                auto model = const_cast<QAbstractItemModel *>(persistentIndex.model());
                model->dataChanged(persistentIndex, persistentIndex);
            });
        }
    }

    // children are in the same project as their parent
    if (!info->isChildTask && parts.testFlag(TaskExtraPart::Project)) {
        info->projectQueryResult = taskQueries->findProject(task);
        if (info->projectQueryResult) {
            QPersistentModelIndex persistentIndex(index);
            info->projectQueryResult->addPostInsertHandler([persistentIndex](const Domain::Project::Ptr &, int) {
                // When a project was found (inserted into the result), update the rendering of the item
                auto model = const_cast<QAbstractItemModel *>(persistentIndex.model());
                model->dataChanged(persistentIndex, persistentIndex);
            });
        }
    }

    if (parts.testFlag(TaskExtraPart::Contexts)) {
        info->contextQueryResult = taskQueries->findContexts(task);
        if (info->contextQueryResult) {
            QPersistentModelIndex persistentIndex(index);
            info->contextQueryResult->addPostInsertHandler([persistentIndex](const Domain::Context::Ptr &, int) {
                // When a project was found (inserted into the result), update the rendering of the item
                auto model = const_cast<QAbstractItemModel *>(persistentIndex.model());
                model->dataChanged(persistentIndex, persistentIndex);
            });
        }
    }

    return info;
}

QVariant PageModel::defaultTaskData(const Domain::Task::Ptr &task, int role, const TaskExtraDataPtr &info)
{
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return task->title();
    case Qt::CheckStateRole:
        return task->isDone() ? Qt::Checked : Qt::Unchecked;
    case Presentation::QueryTreeModelBase::IsChildRole:
        return !info ? QVariant() : info->isChildTask;
    case Presentation::QueryTreeModelBase::ProjectRole:
        if (!info || !info->projectQueryResult) {
            return QVariant();
        } else if (info->projectQueryResult->data().isEmpty()) {
            return QString();
        } else {
            return info->projectQueryResult->data().at(0)->name();
        }
    case Presentation::QueryTreeModelBase::DataSourceRole:
        if (!info || !info->dataSourceQueryResult) {
            return QVariant();
        } else if (info->dataSourceQueryResult->data().isEmpty()) {
            return QString();
        } else {
            return info->dataSourceQueryResult->data().at(0)->name();
        }
    case Presentation::QueryTreeModelBase::ContextListRole:
        if (!info || !info->contextQueryResult) {
            return QVariant();
        } else {
            const auto contexts = info->contextQueryResult->data();
            auto result = QStringList();
            std::transform(std::cbegin(contexts), std::cend(contexts),
                           std::back_inserter(result),
                           [](const Domain::Context::Ptr &context) { return context->name(); });
            return result;
        }
    default:
        break;
    }
    return QVariant();
}
