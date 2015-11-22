/* This file is part of Zanshin

   Copyright 2015 Kevin Ottens <ervin@kde.org>

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


#ifndef PRESENTATION_AVAILABLENOTEPAGESMODEL_H
#define PRESENTATION_AVAILABLENOTEPAGESMODEL_H

#include "presentation/availablepagesmodelinterface.h"

#include "domain/notequeries.h"
#include "domain/noterepository.h"
#include "domain/tagqueries.h"
#include "domain/tagrepository.h"

#include "presentation/metatypes.h"

namespace Presentation {

class AvailablePagesSortFilterProxyModel;

class AvailableNotePagesModel : public AvailablePagesModelInterface
{
    Q_OBJECT
public:
    explicit AvailableNotePagesModel(const Domain::NoteQueries::Ptr &noteQueries,
                                     const Domain::NoteRepository::Ptr &noteRepository,
                                     const Domain::TagQueries::Ptr &tagQueries,
                                     const Domain::TagRepository::Ptr &tagRepository,
                                     QObject *parent = Q_NULLPTR);

    QAbstractItemModel *pageListModel() Q_DECL_OVERRIDE;

    bool hasProjectPages() const Q_DECL_OVERRIDE;
    bool hasContextPages() const Q_DECL_OVERRIDE;
    bool hasTagPages() const Q_DECL_OVERRIDE;

    QObject *createPageForIndex(const QModelIndex &index) Q_DECL_OVERRIDE;

    void addProject(const QString &name, const Domain::DataSource::Ptr &source) Q_DECL_OVERRIDE;
    void addContext(const QString &name) Q_DECL_OVERRIDE;
    void addTag(const QString &name) Q_DECL_OVERRIDE;
    void removeItem(const QModelIndex &index) Q_DECL_OVERRIDE;

private:
    QAbstractItemModel *createPageListModel();

    QAbstractItemModel *m_pageListModel;
    Presentation::AvailablePagesSortFilterProxyModel *m_sortProxyModel;

    Domain::NoteQueries::Ptr m_noteQueries;
    Domain::NoteRepository::Ptr m_noteRepository;

    Domain::TagQueries::Ptr m_tagQueries;
    Domain::TagRepository::Ptr m_tagRepository;

    Domain::QueryResultProvider<QObjectPtr>::Ptr m_rootsProvider;
    QObjectPtr m_inboxObject;
    QObjectPtr m_tagsObject;
};

}

#endif // PRESENTATION_AVAILABLENOTEPAGESMODEL_H
