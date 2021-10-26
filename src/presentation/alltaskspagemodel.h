/*
 * SPDX-FileCopyrightText: 2019 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef PRESENTATION_ALLTASKSPAGEMODEL_H
#define PRESENTATION_ALLTASKSPAGEMODEL_H

#include "presentation/pagemodel.h"

#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

namespace Presentation {

class AllTasksPageModel : public PageModel
{
    Q_OBJECT
public:
    explicit AllTasksPageModel(const Domain::TaskQueries::Ptr &taskQueries,
                              const Domain::TaskRepository::Ptr &taskRepository,
                              QObject *parent = nullptr);

    Domain::Task::Ptr addItem(const QString &title, const QModelIndex &parentIndex = QModelIndex()) override;
    void removeItem(const QModelIndex &index) override;
    void promoteItem(const QModelIndex &index) override;

private:
    QAbstractItemModel *createCentralListModel() override;

    Domain::TaskQueries::Ptr m_taskQueries;
    Domain::TaskRepository::Ptr m_taskRepository;
};

}

#endif // PRESENTATION_ALLTASKSPAGEMODEL_H
