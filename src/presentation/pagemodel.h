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


#ifndef PRESENTATION_PAGEMODEL_H
#define PRESENTATION_PAGEMODEL_H

#include <QObject>

#include "domain/noterepository.h"
#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

#include "presentation/metatypes.h"
#include "presentation/errorhandlingmodelbase.h"

class QModelIndex;

namespace Presentation {

class PageModel : public QObject, public ErrorHandlingModelBase
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* centralListModel READ centralListModel)
public:
    explicit PageModel(const Domain::TaskQueries::Ptr &taskQueries,
                       const Domain::TaskRepository::Ptr &taskRepository,
                       const Domain::NoteRepository::Ptr &noteRepository,
                       QObject *parent = Q_NULLPTR);

    QAbstractItemModel *centralListModel();

public slots:
    virtual Domain::Task::Ptr addTask(const QString &title) = 0;
    virtual void removeItem(const QModelIndex &index) = 0;

protected:
    Domain::TaskQueries::Ptr taskQueries() const;

    Domain::TaskRepository::Ptr taskRepository() const;
    Domain::NoteRepository::Ptr noteRepository() const;

private:
    virtual QAbstractItemModel *createCentralListModel() = 0;

    QAbstractItemModel *m_centralListModel;

    Domain::TaskQueries::Ptr m_taskQueries;

    Domain::TaskRepository::Ptr m_taskRepository;
    Domain::NoteRepository::Ptr m_noteRepository;
};

}

#endif // PRESENTATION_PAGEMODEL_H
