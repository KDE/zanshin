/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
      m_showDone(false),
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

bool TaskFilterProxyModel::showDoneTasks() const
{
    return m_showDone;
}

void TaskFilterProxyModel::setShowDoneTasks(bool show)
{
    if (m_showDone == show)
        return;

    m_showDone = show;
    invalidate();
}

static bool isDoneTask(const Domain::Task::Ptr &task)
{
    if (!task)
        return false;

    return task->isDone();
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
            return (m_showDone || !isDoneTask(task)) && (m_showFuture || !isFutureTask(task));
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

#include "moc_taskfilterproxymodel.cpp"
