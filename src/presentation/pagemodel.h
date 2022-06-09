/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef PRESENTATION_PAGEMODEL_H
#define PRESENTATION_PAGEMODEL_H

#include <QObject>

#include <QModelIndex>

#include "domain/project.h"
#include "domain/task.h"
#include "domain/taskqueries.h"

#include "presentation/metatypes.h"
#include "presentation/errorhandlingmodelbase.h"

namespace Presentation {

class PageModel : public QObject, public ErrorHandlingModelBase
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* centralListModel READ centralListModel)
public:
    enum class TaskExtraPart {
        None = 0x00,
        DataSource = 0x01,
        Project = 0x02,
        Contexts = 0x04,
    };
    Q_ENUM(TaskExtraPart)
    Q_DECLARE_FLAGS(TaskExtraParts, TaskExtraPart)
    Q_FLAG(TaskExtraParts)

    explicit PageModel(QObject *parent = nullptr);

    QAbstractItemModel *centralListModel();

public slots:
    virtual Domain::Task::Ptr addItem(const QString &title, const QModelIndex &parentIndex = QModelIndex()) = 0;
    virtual void removeItem(const QModelIndex &index) = 0;
    virtual void promoteItem(const QModelIndex &index) = 0;

protected:
    struct TaskExtraData
    {
        bool isChildTask = false;
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr dataSourceQueryResult;
        Domain::QueryResult<Domain::Project::Ptr>::Ptr projectQueryResult;
        Domain::QueryResult<Domain::Context::Ptr>::Ptr contextQueryResult;
    };
    using TaskExtraDataPtr = QSharedPointer<TaskExtraData>;

    using ProjectQueryPtr = Domain::QueryResult<Domain::Project::Ptr>::Ptr;
    static TaskExtraDataPtr fetchTaskExtraData(Domain::TaskQueries::Ptr taskQueries,
                                               const TaskExtraParts &parts,
                                               const QModelIndex &index,
                                               const Domain::Task::Ptr &task);
    static QVariant defaultTaskData(const Domain::Task::Ptr &task, int role, const TaskExtraDataPtr &info);

private:
    virtual QAbstractItemModel *createCentralListModel() = 0;

    QAbstractItemModel *m_centralListModel;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Presentation::PageModel::TaskExtraParts)

#endif // PRESENTATION_PAGEMODEL_H
