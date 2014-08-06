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


#ifndef PRESENTATION_AVAILABLEPAGESMODEL_H
#define PRESENTATION_AVAILABLEPAGESMODEL_H

#include <QObject>

#include "domain/datasource.h"
#include "domain/queryresult.h"

#include "presentation/metatypes.h"

class QModelIndex;

namespace Domain {
    class ArtifactQueries;
    class NoteRepository;
    class ProjectQueries;
    class ProjectRepository;
    class TaskQueries;
    class TaskRepository;
}

namespace Presentation {

class AvailablePagesModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* pageListModel READ pageListModel)
public:
    explicit AvailablePagesModel(Domain::ArtifactQueries *artifactQueries,
                                 Domain::ProjectQueries *projectQueries,
                                 Domain::ProjectRepository *projectRepository,
                                 Domain::TaskQueries *taskQueries,
                                 Domain::TaskRepository *taskRepository,
                                 Domain::NoteRepository *noteRepository,
                                 QObject *parent = 0);
    ~AvailablePagesModel();

    QAbstractItemModel *pageListModel();

    Q_SCRIPTABLE QObject *createPageForIndex(const QModelIndex &index);

public slots:
    void addProject(const QString &name, const Domain::DataSource::Ptr &source);

private:
    QAbstractItemModel *createPageListModel();

    QAbstractItemModel *m_pageListModel;

    Domain::ArtifactQueries *m_artifactQueries;

    Domain::ProjectQueries *m_projectQueries;
    Domain::ProjectRepository *m_projectRepository;

    Domain::TaskQueries *m_taskQueries;
    Domain::TaskRepository *m_taskRepository;

    Domain::NoteRepository *m_noteRepository;

    Domain::QueryResultProvider<QObjectPtr>::Ptr m_rootsProvider;
    QObjectPtr m_inboxObject;
    QObjectPtr m_projectsObject;
};

}

#endif // PRESENTATION_AVAILABLEPAGESMODEL_H
