/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
   Copyright 2014 Rémi Benoit <r3m1.benoit@gmail.com>

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

#include "tagpagemodel.h"

#include <QMimeData>

#include "domain/noterepository.h"
#include "domain/task.h"
#include "domain/tagqueries.h"
#include "domain/tagrepository.h"
#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

#include "presentation/querytreemodel.h"

using namespace Presentation;

TagPageModel::TagPageModel(const Domain::Tag::Ptr &tag,
                           const Domain::TagQueries::Ptr &tagQueries,
                           const Domain::TagRepository::Ptr &tagRepository,
                           const Domain::NoteRepository::Ptr &noteRepository,
                           QObject *parent)
    : PageModel(parent),
      m_tag(tag),
      m_tagQueries(tagQueries),
      m_tagRepository(tagRepository),
      m_noteRepository(noteRepository)
{

}

Domain::Tag::Ptr TagPageModel::tag() const
{
    return m_tag;
}

Domain::Artifact::Ptr TagPageModel::addItem(const QString &title, const QModelIndex &)
{
    auto note = Domain::Note::Ptr::create();
    note->setTitle(title);
    const auto job = m_noteRepository->createInTag(note, m_tag);
    installHandler(job, tr("Cannot add note %1 in tag %2").arg(title, m_tag->name()));
    return note;
}

void TagPageModel::removeItem(const QModelIndex &index)
{
    QVariant data = index.data(QueryTreeModel<Domain::Artifact::Ptr>::ObjectRole);
    auto artifact = data.value<Domain::Artifact::Ptr>();
    auto note = artifact.objectCast<Domain::Note>();
    Q_ASSERT(note);
    const auto job = m_tagRepository->dissociate(m_tag, note);
    installHandler(job, tr("Cannot remove note %1 from tag %2").arg(note->title(), m_tag->name()));
}

void TagPageModel::promoteItem(const QModelIndex &)
{
    qFatal("Not supported");
}

QAbstractItemModel *TagPageModel::createCentralListModel()
{
    auto query = [this] (const Domain::Note::Ptr &note) -> Domain::QueryResultInterface<Domain::Note::Ptr>::Ptr {
        if (!note)
            return m_tagQueries->findNotes(m_tag);
        else
            return Domain::QueryResult<Domain::Note::Ptr>::Ptr();
    };

    auto flags = [](const Domain::Note::Ptr &) {
        return Qt::ItemIsSelectable
             | Qt::ItemIsEnabled
             | Qt::ItemIsEditable
             | Qt::ItemIsDragEnabled;
    };

    auto data = [](const Domain::Note::Ptr &note, int role) -> QVariant {
        if (role != Qt::DisplayRole
         && role != Qt::EditRole) {
            return QVariant();
        }

        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return note->title();
        } else {
            return QVariant();
        }
    };

    auto setData = [this] (const Domain::Note::Ptr &note, const QVariant &value, int role) {
        if (role != Qt::EditRole) {
            return false;
        }

        const auto currentTitle = note->title();
        note->setTitle(value.toString());
        const auto job = m_noteRepository->update(note);
        installHandler(job, tr("Cannot modify note %1 in tag %2").arg(currentTitle, m_tag->name()));
        return true;
    };

    auto drop = [this] (const QMimeData *, Qt::DropAction, const Domain::Note::Ptr &) {
        return false;
    };

    auto drag = [] (const Domain::Note::List &notes) -> QMimeData* {
        if (notes.isEmpty())
            return Q_NULLPTR;

        auto draggedArtifacts = Domain::Artifact::List();
        std::copy(notes.constBegin(), notes.constEnd(),
                  std::back_inserter(draggedArtifacts));

        auto data = new QMimeData;
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(draggedArtifacts));
        return data;
    };

    return new QueryTreeModel<Domain::Note::Ptr>(query, flags, data, setData, drop, drag, this);
}
