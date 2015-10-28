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

Domain::Artifact::Ptr TagPageModel::addItem(const QString &title)
{
    auto note = Domain::Note::Ptr::create();
    note->setTitle(title);
    const auto job = m_noteRepository->createInTag(note, m_tag);
    installHandler(job, tr("Cannot add note %1 in tag %2").arg(title).arg(m_tag->name()));
    return note;
}

void TagPageModel::removeItem(const QModelIndex &index)
{
    QVariant data = index.data(QueryTreeModel<Domain::Artifact::Ptr>::ObjectRole);
    auto artifact = data.value<Domain::Artifact::Ptr>();
    const auto job = m_tagRepository->dissociate(m_tag, artifact);
    if (artifact.objectCast<Domain::Task>()) {
        installHandler(job, tr("Cannot remove task %1 from tag %2").arg(artifact->title()).arg(m_tag->name()));
    } else if (artifact.objectCast<Domain::Note>()) {
        installHandler(job, tr("Cannot remove note %1 from tag %2").arg(artifact->title()).arg(m_tag->name()));
    }
}

QAbstractItemModel *TagPageModel::createCentralListModel()
{
    auto query = [this] (const Domain::Artifact::Ptr &artifact) -> Domain::QueryResultInterface<Domain::Artifact::Ptr>::Ptr {
        if (!artifact)
            return m_tagQueries->findTopLevelArtifacts(m_tag);
        else
            return Domain::QueryResult<Domain::Artifact::Ptr>::Ptr();
    };

    auto flags = [](const Domain::Artifact::Ptr &artifact) {
        const auto defaultFlags = Qt::ItemIsSelectable
                                | Qt::ItemIsEnabled
                                | Qt::ItemIsEditable
                                | Qt::ItemIsDragEnabled;

        return artifact.dynamicCast<Domain::Note>() ? defaultFlags : Qt::NoItemFlags;
    };

    auto data = [](const Domain::Artifact::Ptr &artifact, int role) -> QVariant {
        if (role != Qt::DisplayRole
         && role != Qt::EditRole) {
            return QVariant();
        }

        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return artifact->title();
        } else {
            return QVariant();
        }
    };

    auto setData = [this] (const Domain::Artifact::Ptr &artifact, const QVariant &value, int role) {
        if (role != Qt::EditRole) {
            return false;
        }

        if (auto note = artifact.dynamicCast<Domain::Note>()) {
            const auto currentTitle = note->title();
            note->setTitle(value.toString());
            const auto job = m_noteRepository->update(note);
            installHandler(job, tr("Cannot modify note %1 in tag %2").arg(currentTitle).arg(m_tag->name()));
            return true;
        }

        return false;
    };

    auto drop = [this] (const QMimeData *, Qt::DropAction, const Domain::Artifact::Ptr &) {
        return false;
    };

    auto drag = [] (const Domain::Artifact::List &artifacts) -> QMimeData* {
        if (artifacts.isEmpty())
            return Q_NULLPTR;

        auto data = new QMimeData;
        data->setData("application/x-zanshin-object", "object");
        data->setProperty("objects", QVariant::fromValue(artifacts));
        return data;
    };

    return new QueryTreeModel<Domain::Artifact::Ptr>(query, flags, data, setData, drop, drag, this);
}
