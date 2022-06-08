/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2014 Rémi Benoit <r3m1.benoit@gmail.com>
   * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
   */


#ifndef CONTEXTPAGEMODEL_H
#define CONTEXTPAGEMODEL_H

#include "presentation/pagemodel.h"

#include "domain/contextqueries.h"
#include "domain/contextrepository.h"
#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

namespace Presentation {

class ContextPageModel : public PageModel
{
    Q_OBJECT
public:
    explicit ContextPageModel(const Domain::Context::Ptr &context,
                              const Domain::ContextQueries::Ptr &contextQueries,
                              const Domain::ContextRepository::Ptr &contextRepository,
                              const Domain::TaskQueries::Ptr &taskQueries,
                              const Domain::TaskRepository::Ptr &taskRepository,
                              QObject *parent = nullptr);

    Domain::Context::Ptr context() const;
public Q_SLOTS:
    Domain::Task::Ptr addItem(const QString &title, const QModelIndex &parentIndex = QModelIndex()) override;
    void removeItem(const QModelIndex &index) override;
    void promoteItem(const QModelIndex &index) override;

private:
    QAbstractItemModel *createCentralListModel() override;

    Domain::Context::Ptr m_context;
    Domain::ContextQueries::Ptr m_contextQueries;
    Domain::ContextRepository::Ptr m_contextRepository;
    Domain::TaskQueries::Ptr m_taskQueries;
    Domain::TaskRepository::Ptr m_taskRepository;
};

}
#endif // CONTEXTPAGEMODEL_H
