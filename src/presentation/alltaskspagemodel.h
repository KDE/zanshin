/* This file is part of Zanshin

   Copyright 2019 David Faure <faure@kde.org>

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
