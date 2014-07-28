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


#ifndef PRESENTATION_INBOXMODEL_H
#define PRESENTATION_INBOXMODEL_H

#include <QObject>

#include "domain/datasourcequeries.h"

class QAbstractItemModel;

Q_DECLARE_METATYPE(QAbstractItemModel*)

namespace Domain {
    class ArtifactQueries;
    class NoteRepository;
    class TaskQueries;
    class TaskRepository;
}

namespace Presentation {

class InboxModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Domain::DataSource::Ptr defaultNoteDataSource READ defaultNoteDataSource WRITE setDefaultNoteDataSource)
    Q_PROPERTY(Domain::DataSource::Ptr defaultTaskDataSource READ defaultTaskDataSource WRITE setDefaultTaskDataSource)
    Q_PROPERTY(QAbstractItemModel* centralListModel READ centralListModel)
public:
    explicit InboxModel(Domain::ArtifactQueries *artifactQueries,
                        Domain::DataSourceQueries *sourceQueries,
                        Domain::TaskQueries *taskQueries,
                        Domain::TaskRepository *taskRepository,
                        Domain::NoteRepository *noteRepository,
                        QObject *parent = 0);
    ~InboxModel();

    QAbstractItemModel *centralListModel();
    Domain::DataSource::Ptr defaultNoteDataSource() const;
    Domain::DataSource::Ptr defaultTaskDataSource() const;

public slots:
    void setDefaultNoteDataSource(Domain::DataSource::Ptr source);
    void setDefaultTaskDataSource(Domain::DataSource::Ptr source);

private:
    QAbstractItemModel *createCentralListModel();
    QAbstractItemModel *m_centralListModel;

    Domain::ArtifactQueries *m_artifactQueries;
    Domain::DataSourceQueries *m_sourceQueries;
    Domain::TaskQueries *m_taskQueries;

    Domain::TaskRepository *m_taskRepository;
    Domain::QueryResult<Domain::DataSource::Ptr>::Ptr m_taskSources;

    Domain::NoteRepository *m_noteRepository;
    Domain::QueryResult<Domain::DataSource::Ptr>::Ptr m_noteSources;
};

}

#endif // PRESENTATION_INBOXMODEL_H
