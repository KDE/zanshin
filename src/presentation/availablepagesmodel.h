/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef PRESENTATION_AVAILABLETASKPAGESMODEL_H
#define PRESENTATION_AVAILABLETASKPAGESMODEL_H

#include "presentation/errorhandlingmodelbase.h"

#include "domain/contextqueries.h"
#include "domain/contextrepository.h"
#include "domain/datasourcequeries.h"
#include "domain/projectqueries.h"
#include "domain/projectrepository.h"
#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

#include "presentation/metatypes.h"

class QModelIndex;

namespace Presentation {
class AvailablePagesSortFilterProxyModel;

class AvailablePagesModel : public QObject, public ErrorHandlingModelBase
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* pageListModel READ pageListModel)
public:
    explicit AvailablePagesModel(const Domain::DataSourceQueries::Ptr &dataSourceQueries,
                                 const Domain::ProjectQueries::Ptr &projectQueries,
                                 const Domain::ProjectRepository::Ptr &projectRepository,
                                 const Domain::ContextQueries::Ptr &contextQueries,
                                 const Domain::ContextRepository::Ptr &contextRepository,
                                 const Domain::TaskQueries::Ptr &taskQueries,
                                 const Domain::TaskRepository::Ptr &taskRepository,
                                 QObject *parent = nullptr);

    QAbstractItemModel *pageListModel();

    Q_SCRIPTABLE QObject *createPageForIndex(const QModelIndex &index);

public Q_SLOTS:
    void addProject(const QString &name, const Domain::DataSource::Ptr &source);
    void addContext(const QString &name, const Domain::DataSource::Ptr &source);
    void removeItem(const QModelIndex &index);

private:
    QAbstractItemModel *createPageListModel();

    QAbstractItemModel *m_pageListModel;
    Presentation::AvailablePagesSortFilterProxyModel *m_sortProxyModel;

    Domain::DataSourceQueries::Ptr m_dataSourceQueries;

    Domain::ProjectQueries::Ptr m_projectQueries;
    Domain::ProjectRepository::Ptr m_projectRepository;

    Domain::ContextQueries::Ptr m_contextQueries;
    Domain::ContextRepository::Ptr m_contextRepository;

    Domain::TaskQueries::Ptr m_taskQueries;
    Domain::TaskRepository::Ptr m_taskRepository;

    Domain::QueryResultProvider<QObjectPtr>::Ptr m_rootsProvider;
    QObjectPtr m_inboxObject;
    QObjectPtr m_workdayObject;
    QObjectPtr m_projectsObject;
    QObjectPtr m_contextsObject;
    QObjectPtr m_allTasksObject;
};

}

#endif // PRESENTATION_AVAILABLETASKPAGESMODEL_H
