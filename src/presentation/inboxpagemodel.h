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

#include "presentation/pagemodel.h"

#include "domain/artifactqueries.h"

namespace Presentation {

class InboxPageModel : public PageModel
{
    Q_OBJECT
public:
    explicit InboxPageModel(const Domain::ArtifactQueries::Ptr &artifactQueries,
                            const Domain::TaskQueries::Ptr &taskQueries,
                            const Domain::TaskRepository::Ptr &taskRepository,
                            const Domain::NoteRepository::Ptr &noteRepository,
                            QObject *parent = Q_NULLPTR);

    void addTask(const QString &title) Q_DECL_OVERRIDE;
    void removeItem(const QModelIndex &index) Q_DECL_OVERRIDE;

private:
    QAbstractItemModel *createCentralListModel() Q_DECL_OVERRIDE;

    Domain::ArtifactQueries::Ptr m_artifactQueries;
};

}

#endif // PRESENTATION_INBOXPAGEMODEL_H
