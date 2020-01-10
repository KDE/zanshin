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

#include <QRegularExpression>

#include "taskfilterproxymodel.h"

#include <limits>

#include "domain/task.h"
#include "utils/datetime.h"

#include "presentation/querytreemodelbase.h"

using namespace Presentation;

TaskFilterProxyModel::TaskFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent),
      m_sortType(TitleSort),
      m_showFuture(false)
{
    setDynamicSortFilter(true);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setSortOrder(Qt::AscendingOrder);
}

TaskFilterProxyModel::SortType TaskFilterProxyModel::sortType() const
{
    return m_sortType;
}

void TaskFilterProxyModel::setSortType(TaskFilterProxyModel::SortType type)
{
    m_sortType = type;
    invalidate();
}

void TaskFilterProxyModel::setSortOrder(Qt::SortOrder order)
{
    sort(0, order);
}

bool TaskFilterProxyModel::showFutureTasks() const
{
    return m_showFuture;
}

void TaskFilterProxyModel::setShowFutureTasks(bool show)
{
    if (m_showFuture == show)
        return;

    m_showFuture = show;
    invalidate();
}

static bool isFutureTask(const Domain::Task::Ptr &task)
{
    if (!task)
        return false;

    if (!task->startDate().isValid())
        return false;

    return task->startDate() > Utils::DateTime::currentDate();
}

bool TaskFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    const auto task = index.data(QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();
    if (task) {
        QRegularExpression regexp = filterRegularExpression();
        regexp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);

        if (task->title().contains(regexp)
         || task->text().contains(regexp)) {
            return m_showFuture || !isFutureTask(task);
        }
    }

    for (int childRow = 0; childRow < sourceModel()->rowCount(index); childRow++) {
        if (filterAcceptsRow(childRow, index))
            return true;
    }

    return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

static QDate validDate(const QDate &date = QDate())
{
    if (date.isValid())
        return date;

    return QDate(80000, 12, 31);
}

bool TaskFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (m_sortType != DateSort)
        return QSortFilterProxyModel::lessThan(left, right);

    const auto leftTask = left.data(QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();
    const auto rightTask = right.data(QueryTreeModelBase::ObjectRole).value<Domain::Task::Ptr>();

    // The addDays(1) is so that we sort non-tasks (e.g. notes) at the end

    const QDate leftDue = leftTask ? validDate(leftTask->dueDate()) : validDate().addDays(1);
    const QDate rightDue = rightTask ? validDate(rightTask->dueDate()) : validDate().addDays(1);

    const QDate leftStart = leftTask ? validDate(leftTask->startDate()) : validDate().addDays(1);
    const QDate rightStart = rightTask ? validDate(rightTask->startDate()) : validDate().addDays(1);

    return leftDue < rightDue
        || leftStart < rightStart;
}
