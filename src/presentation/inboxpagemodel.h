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


#ifndef PRESENTATION_INBOXPAGEMODEL_H
#define PRESENTATION_INBOXPAGEMODEL_H

#include <QObject>

#include "presentation/metatypes.h"

class QModelIndex;

namespace Domain {
    class ArtifactQueries;
    class NoteRepository;
    class TaskQueries;
    class TaskRepository;
}

namespace Presentation {

class InboxPageModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* centralListModel READ centralListModel)
public:
    explicit InboxPageModel(Domain::ArtifactQueries *artifactQueries,
                            Domain::TaskQueries *taskQueries,
                            Domain::TaskRepository *taskRepository,
                            Domain::NoteRepository *noteRepository,
                            QObject *parent = 0);
    ~InboxPageModel();

    QAbstractItemModel *centralListModel();

public slots:
    void addTask(const QString &title);
    void removeItem(const QModelIndex &index);

private:
    QAbstractItemModel *createCentralListModel();
    QAbstractItemModel *m_centralListModel;

    Domain::ArtifactQueries *m_artifactQueries;
    Domain::TaskQueries *m_taskQueries;

    Domain::TaskRepository *m_taskRepository;
    Domain::NoteRepository *m_noteRepository;
};

}

#endif // PRESENTATION_INBOXPAGEMODEL_H
