/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef PRESENTATION_PROJECTPAGEMODEL_H
#define PRESENTATION_PROJECTPAGEMODEL_H

#include "presentation/pagemodel.h"

#include "domain/projectqueries.h"
#include "domain/projectrepository.h"
#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

namespace Presentation {

class ProjectPageModel : public PageModel
{
    Q_OBJECT
public:
    explicit ProjectPageModel(const Domain::Project::Ptr &project,
                              const Domain::ProjectQueries::Ptr &projectQueries,
                              const Domain::ProjectRepository::Ptr &projectRepository,
                              const Domain::TaskQueries::Ptr &taskQueries,
                              const Domain::TaskRepository::Ptr &taskRepository,
                              QObject *parent = nullptr);

    Domain::Project::Ptr project() const;

    Domain::Task::Ptr addItem(const QString &title, const QModelIndex &parentIndex = QModelIndex()) override;
    void removeItem(const QModelIndex &index) override;
    void promoteItem(const QModelIndex &index) override;

private:
    QAbstractItemModel *createCentralListModel() override;

    Domain::ProjectQueries::Ptr m_projectQueries;
    Domain::ProjectRepository::Ptr m_projectRepository;
    Domain::Project::Ptr m_project;

    Domain::TaskQueries::Ptr m_taskQueries;
    Domain::TaskRepository::Ptr m_taskRepository;
};

}

#endif // PRESENTATION_PROJECTPAGEMODEL_H
