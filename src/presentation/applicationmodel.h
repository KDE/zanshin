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


#ifndef PRESENTATION_APPLICATIONMODEL_H
#define PRESENTATION_APPLICATIONMODEL_H

#include <QObject>

#include "domain/datasourcequeries.h"

#include "presentation/metatypes.h"

namespace Domain {
    class ArtifactQueries;
    class DataSourceRepository;
    class NoteRepository;
    class ProjectQueries;
    class ProjectRepository;
    class ContextQueries;
    class ContextRepository;
    class TagQueries;
    class TagRepository;
    class TaskQueries;
    class TaskRepository;
}

namespace Presentation {

class ApplicationModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* noteSourcesModel READ noteSourcesModel)
    Q_PROPERTY(Domain::DataSource::Ptr defaultNoteDataSource READ defaultNoteDataSource WRITE setDefaultNoteDataSource)
    Q_PROPERTY(QAbstractItemModel* taskSourcesModel READ taskSourcesModel)
    Q_PROPERTY(Domain::DataSource::Ptr defaultTaskDataSource READ defaultTaskDataSource WRITE setDefaultTaskDataSource)
    Q_PROPERTY(QObject* availableSources READ availableSources)
    Q_PROPERTY(QObject* availablePages READ availablePages)
    Q_PROPERTY(QObject* currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged)
    Q_PROPERTY(QObject* editor READ editor)
public:
    explicit ApplicationModel(QObject *parent = 0);

    explicit ApplicationModel(Domain::ArtifactQueries *artifactQueries,
                              Domain::ProjectQueries *projectQueries,
                              Domain::ProjectRepository *projectRepository,
                              Domain::ContextQueries *contextQueries,
                              Domain::ContextRepository *contextRepository,
                              Domain::DataSourceQueries *sourceQueries,
                              Domain::DataSourceRepository *sourceRepository,
                              Domain::TaskQueries *taskQueries,
                              Domain::TaskRepository *taskRepository,
                              Domain::NoteRepository *noteRepository,
                              Domain::TagQueries *tagQueries,
                              Domain::TagRepository *tagRepository,
                              QObject *parent = 0);
    ~ApplicationModel();

    QAbstractItemModel *noteSourcesModel();
    Domain::DataSource::Ptr defaultNoteDataSource();

    QAbstractItemModel *taskSourcesModel();
    Domain::DataSource::Ptr defaultTaskDataSource();

    QObject *availableSources();
    QObject *availablePages();
    QObject *currentPage();
    QObject *editor();

public slots:
    void setCurrentPage(QObject *page);
    void setDefaultNoteDataSource(Domain::DataSource::Ptr source);
    void setDefaultTaskDataSource(Domain::DataSource::Ptr source);

signals:
    void currentPageChanged(QObject *page);

private:
    Domain::QueryResult<Domain::DataSource::Ptr>::Ptr noteSources();
    Domain::QueryResult<Domain::DataSource::Ptr>::Ptr taskSources();

    QObject *m_availableSources;
    QObject *m_availablePages;
    QObject *m_currentPage;
    QObject *m_editor;

    Domain::ArtifactQueries *m_artifactQueries;

    Domain::ProjectQueries *m_projectQueries;
    Domain::ProjectRepository *m_projectRepository;

    Domain::ContextQueries *m_contextQueries;
    Domain::ContextRepository *m_contextRepository;

    Domain::DataSourceQueries *m_sourceQueries;
    Domain::DataSourceRepository *m_sourceRepository;

    Domain::TaskQueries *m_taskQueries;
    Domain::TaskRepository *m_taskRepository;

    Domain::QueryResult<Domain::DataSource::Ptr>::Ptr m_taskSources;
    QAbstractItemModel *m_taskSourcesModel;

    Domain::NoteRepository *m_noteRepository;
    Domain::QueryResult<Domain::DataSource::Ptr>::Ptr m_noteSources;
    QAbstractItemModel *m_noteSourcesModel;

    Domain::TagQueries *m_tagQueries;
    Domain::TagRepository *m_tagRepository;

    bool m_ownInterface;
};

}

#endif // PRESENTATION_APPLICATIONMODEL_H
