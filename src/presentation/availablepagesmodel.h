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

#include "domain/artifactqueries.h"
#include "domain/contextqueries.h"
#include "domain/contextrepository.h"
#include "domain/datasource.h"
#include "domain/noterepository.h"
#include "domain/projectqueries.h"
#include "domain/projectrepository.h"
#include "domain/tagqueries.h"
#include "domain/tagrepository.h"
#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

#include "presentation/metatypes.h"
#include "presentation/errorhandlingmodelbase.h"

class QModelIndex;

namespace Presentation {
class AvailablePagesSortFilterProxyModel;

class AvailablePagesModel : public QObject, public ErrorHandlingModelBase
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* pageListModel READ pageListModel)
public:
    explicit AvailablePagesModel(const Domain::ArtifactQueries::Ptr &artifactQueries,
                                 const Domain::ProjectQueries::Ptr &projectQueries,
                                 const Domain::ProjectRepository::Ptr &projectRepository,
                                 const Domain::ContextQueries::Ptr &contextQueries,
                                 const Domain::ContextRepository::Ptr &contextRepository,
                                 const Domain::TaskQueries::Ptr &taskQueries,
                                 const Domain::TaskRepository::Ptr &taskRepository,
                                 const Domain::NoteRepository::Ptr &noteRepository,
                                 const Domain::TagQueries::Ptr &tagQueries,
                                 const Domain::TagRepository::Ptr &tagRepository,
                                 QObject *parent = Q_NULLPTR);

    QAbstractItemModel *pageListModel();

    Q_SCRIPTABLE QObject *createPageForIndex(const QModelIndex &index);

public slots:
    void addProject(const QString &name, const Domain::DataSource::Ptr &source);
    void addContext(const QString &name);
    void addTag(const QString &name);
    void removeItem(const QModelIndex &index);

private:
    QAbstractItemModel *createPageListModel();

    QAbstractItemModel *m_pageListModel;
    Presentation::AvailablePagesSortFilterProxyModel *m_sortProxyModel;

    Domain::ArtifactQueries::Ptr m_artifactQueries;

    Domain::ProjectQueries::Ptr m_projectQueries;
    Domain::ProjectRepository::Ptr m_projectRepository;

    Domain::ContextQueries::Ptr m_contextQueries;
    Domain::ContextRepository::Ptr m_contextRepository;

    Domain::TaskQueries::Ptr m_taskQueries;
    Domain::TaskRepository::Ptr m_taskRepository;

    Domain::NoteRepository::Ptr m_noteRepository;

    Domain::TagQueries::Ptr m_tagQueries;
    Domain::TagRepository::Ptr m_tagRepository;

    Domain::QueryResultProvider<QObjectPtr>::Ptr m_rootsProvider;
    QObjectPtr m_inboxObject;
    QObjectPtr m_workdayObject;
    QObjectPtr m_projectsObject;
    QObjectPtr m_contextsObject;
    QObjectPtr m_tagsObject;
};

}

#endif // PRESENTATION_AVAILABLEPAGESMODEL_H
