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


#include "availablenotepagesmodel.h"

#include <QIcon>
#include <QMimeData>

#include <KLocalizedString>

#include "presentation/availablepagessortfilterproxymodel.h"
#include "presentation/noteinboxpagemodel.h"
#include "presentation/querytreemodel.h"
#include "presentation/tagpagemodel.h"

#include "utils/jobhandler.h"

using namespace Presentation;

AvailableNotePagesModel::AvailableNotePagesModel(const Domain::NoteQueries::Ptr &noteQueries,
                                                 const Domain::NoteRepository::Ptr &noteRepository,
                                                 const Domain::TagQueries::Ptr &tagQueries,
                                                 const Domain::TagRepository::Ptr &tagRepository,
                                                 QObject *parent)
    : AvailablePagesModelInterface(parent),
      m_pageListModel(Q_NULLPTR),
      m_sortProxyModel(Q_NULLPTR),
      m_noteQueries(noteQueries),
      m_noteRepository(noteRepository),
      m_tagQueries(tagQueries),
      m_tagRepository(tagRepository)
{
}

QAbstractItemModel *AvailableNotePagesModel::pageListModel()
{
    if (!m_pageListModel)
        m_pageListModel = createPageListModel();

    if (!m_sortProxyModel) {
        m_sortProxyModel = new AvailablePagesSortFilterProxyModel(this);
        m_sortProxyModel->setSourceModel(m_pageListModel);
    }

    return m_sortProxyModel;
}

bool AvailableNotePagesModel::hasProjectPages() const
{
    return false;
}

bool AvailableNotePagesModel::hasContextPages() const
{
    return false;
}

bool AvailableNotePagesModel::hasTagPages() const
{
    return true;
}

QObject *AvailableNotePagesModel::createPageForIndex(const QModelIndex &index)
{
    QObjectPtr object = index.data(QueryTreeModelBase::ObjectRole).value<QObjectPtr>();

    if (object == m_inboxObject) {
        auto inboxPageModel = new NoteInboxPageModel(m_noteQueries,
                                                     m_noteRepository,
                                                     this);
        inboxPageModel->setErrorHandler(errorHandler());
        return inboxPageModel;
    } else if (auto tag = object.objectCast<Domain::Tag>()) {
        auto tagPageModel = new TagPageModel(tag,
                                             m_tagQueries,
                                             m_tagRepository,
                                             m_noteRepository,
                                             this);
        tagPageModel->setErrorHandler(errorHandler());
        return tagPageModel;
    }

    return Q_NULLPTR;
}

void AvailableNotePagesModel::addProject(const QString &, const Domain::DataSource::Ptr &)
{
    qFatal("Not supported");
}

void AvailableNotePagesModel::addContext(const QString &)
{
    qFatal("Not supported");
}

void AvailableNotePagesModel::addTag(const QString &name)
{
    auto tag = Domain::Tag::Ptr::create();
    tag->setName(name);
    const auto job = m_tagRepository->create(tag);
    installHandler(job, i18n("Cannot add tag %1", name));
}

void AvailableNotePagesModel::removeItem(const QModelIndex &index)
{
    QObjectPtr object = index.data(QueryTreeModelBase::ObjectRole).value<QObjectPtr>();
    if (auto tag = object.objectCast<Domain::Tag>()) {
        const auto job = m_tagRepository->remove(tag);
        installHandler(job, i18n("Cannot remove tag %1", tag->name()));
    } else {
        Q_ASSERT(false);
    }
}

QAbstractItemModel *AvailableNotePagesModel::createPageListModel()
{
    m_inboxObject = QObjectPtr::create();
    m_inboxObject->setProperty("name", i18n("Inbox"));
    m_tagsObject = QObjectPtr::create();
    m_tagsObject->setProperty("name", i18n("Tags"));

    m_rootsProvider = Domain::QueryResultProvider<QObjectPtr>::Ptr::create();
    m_rootsProvider->append(m_inboxObject);
    m_rootsProvider->append(m_tagsObject);

    auto query = [this](const QObjectPtr &object) -> Domain::QueryResultInterface<QObjectPtr>::Ptr {
        if (!object)
            return Domain::QueryResult<QObjectPtr>::create(m_rootsProvider);
        else if (object == m_tagsObject)
            return Domain::QueryResult<Domain::Tag::Ptr, QObjectPtr>::copy(m_tagQueries->findAll());
        else
            return Domain::QueryResult<QObjectPtr>::Ptr();
    };

    auto flags = [this](const QObjectPtr &object) {
        const Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable
                                         | Qt::ItemIsEnabled
                                         | Qt::ItemIsEditable
                                         | Qt::ItemIsDropEnabled;
        const Qt::ItemFlags immutableNodeFlags = Qt::ItemIsSelectable
                                               | Qt::ItemIsEnabled
                                               | Qt::ItemIsDropEnabled;
        const Qt::ItemFlags structureNodeFlags = Qt::NoItemFlags;

        return object.objectCast<Domain::Tag>() ? defaultFlags
             : object == m_inboxObject ? immutableNodeFlags
             : structureNodeFlags;
    };

    auto data = [this](const QObjectPtr &object, int role, int) -> QVariant {
        if (role != Qt::DisplayRole
         && role != Qt::EditRole
         && role != Qt::DecorationRole
         && role != QueryTreeModelBase::IconNameRole) {
            return QVariant();
        }

        if (role == Qt::EditRole
         && (object == m_inboxObject
          || object == m_tagsObject)) {
            return QVariant();
        }

        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return object->property("name").toString();
        } else if (role == Qt::DecorationRole || role == QueryTreeModelBase::IconNameRole) {
            const QString iconName = (object == m_inboxObject) ? QStringLiteral("mail-folder-inbox")
                                   : (object == m_tagsObject)  ? QStringLiteral("folder")
                                   : QStringLiteral("view-pim-tasks");

            if (role == Qt::DecorationRole)
                return QVariant::fromValue(QIcon::fromTheme(iconName));
            else
                return iconName;
        } else {
            return QVariant();
        }
    };

    auto setData = [this](const QObjectPtr &object, const QVariant &, int role) {
        if (role != Qt::EditRole) {
            return false;
        }

        if (object == m_inboxObject
         || object == m_tagsObject) {
            return false;
        }

        if (object.objectCast<Domain::Tag>()) {
            return false; // Tag renaming is NOT allowed
        } else {
            Q_ASSERT(false);
        }

        return true;
    };

    auto drop = [this](const QMimeData *mimeData, Qt::DropAction, const QObjectPtr &object) {
        if (!mimeData->hasFormat(QStringLiteral("application/x-zanshin-object")))
            return false;

        auto droppedArtifacts = mimeData->property("objects").value<Domain::Artifact::List>();
        if (droppedArtifacts.isEmpty())
            return false;

        if (std::any_of(droppedArtifacts.begin(), droppedArtifacts.end(),
                        [](const Domain::Artifact::Ptr &droppedArtifact) {
                            return !droppedArtifact.objectCast<Domain::Note>();
                        })) {
            return false;
        }

        if (auto tag = object.objectCast<Domain::Tag>()) {
            foreach (const auto &droppedArtifact, droppedArtifacts) {
                auto note = droppedArtifact.staticCast<Domain::Note>();
                const auto job = m_tagRepository->associate(tag, note);
                installHandler(job, i18n("Cannot tag %1 with %2", note->title(), tag->name()));
            }
            return true;
        } else if (object == m_inboxObject) {
            foreach (const auto &droppedArtifact, droppedArtifacts) {
                auto note = droppedArtifact.staticCast<Domain::Note>();
                const auto job = m_tagRepository->dissociateAll(note);
                installHandler(job, i18n("Cannot move %1 to Inbox", note->title()));
            }
            return true;
        }

        return false;
    };

    auto drag = [](const QObjectPtrList &) -> QMimeData* {
        return Q_NULLPTR;
    };

    return new QueryTreeModel<QObjectPtr>(query, flags, data, setData, drop, drag, nullptr, this);
}
