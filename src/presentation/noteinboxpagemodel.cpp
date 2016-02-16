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


#include "noteinboxpagemodel.h"

#include <QMimeData>

#include "presentation/querytreemodel.h"

using namespace Presentation;

NoteInboxPageModel::NoteInboxPageModel(const Domain::NoteQueries::Ptr &noteQueries,
                                       const Domain::NoteRepository::Ptr &noteRepository,
                                       QObject *parent)
    : PageModel(parent),
      m_noteQueries(noteQueries),
      m_noteRepository(noteRepository)
{
}

Domain::Artifact::Ptr NoteInboxPageModel::addItem(const QString &title, const QModelIndex &)
{
    auto note = Domain::Note::Ptr::create();
    note->setTitle(title);
    const auto job = m_noteRepository->create(note);
    installHandler(job, tr("Cannot add note %1 in Inbox").arg(title));

    return note;
}

void NoteInboxPageModel::removeItem(const QModelIndex &index)
{
    QVariant data = index.data(QueryTreeModel<Domain::Note::Ptr>::ObjectRole);
    auto artifact = data.value<Domain::Artifact::Ptr>();
    auto note = artifact.objectCast<Domain::Note>();
    Q_ASSERT(note);
    const auto job = m_noteRepository->remove(note);
    installHandler(job, tr("Cannot remove note %1 from Inbox").arg(note->title()));
}

void NoteInboxPageModel::promoteItem(const QModelIndex &)
{
    qFatal("Not supported");
}

QAbstractItemModel *NoteInboxPageModel::createCentralListModel()
{
    auto query = [this](const Domain::Note::Ptr &note) -> Domain::QueryResultInterface<Domain::Note::Ptr>::Ptr {
        if (!note)
            return m_noteQueries->findInbox();
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
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return note->title();
        } else {
            return QVariant();
        }
    };

    auto setData = [this](const Domain::Note::Ptr &note, const QVariant &value, int role) {
        if (role != Qt::EditRole) {
            return false;
        }

        const auto currentTitle = note->title();
        note->setTitle(value.toString());
        const auto job = m_noteRepository->update(note);
        installHandler(job, tr("Cannot modify note %1 in Inbox").arg(currentTitle));
        return true;
    };

    auto drop = [this](const QMimeData *, Qt::DropAction, const Domain::Artifact::Ptr &) {
        return false;
    };

    auto drag = [](const Domain::Note::List &notes) -> QMimeData* {
        if (notes.isEmpty())
            return Q_NULLPTR;

        auto artifacts = Domain::Artifact::List();
        artifacts.reserve(notes.size());
        std::copy(notes.constBegin(), notes.constEnd(),
                  std::back_inserter(artifacts));

        auto data = new QMimeData;
        data->setData(QStringLiteral("application/x-zanshin-object"), "object");
        data->setProperty("objects", QVariant::fromValue(artifacts));
        return data;
    };

    return new QueryTreeModel<Domain::Note::Ptr>(query, flags, data, setData, drop, drag, this);
}
