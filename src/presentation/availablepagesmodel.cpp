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


#include "availablepagesmodel.h"

#include <QIcon>

#include "domain/projectqueries.h"
#include "domain/projectrepository.h"

#include "presentation/metatypes.h"
#include "presentation/querytreemodel.h"

using namespace Presentation;

AvailablePagesModel::AvailablePagesModel(Domain::ProjectQueries *projectQueries,
                                         Domain::ProjectRepository *projectRepository,
                                         QObject *parent)
    : QObject(parent),
      m_pageListModel(0),
      m_projectQueries(projectQueries),
      m_projectRepository(projectRepository)
{
}

AvailablePagesModel::~AvailablePagesModel()
{
}

QAbstractItemModel *AvailablePagesModel::pageListModel()
{
    if (!m_pageListModel)
        m_pageListModel = createPageListModel();
    return m_pageListModel;
}

QAbstractItemModel *AvailablePagesModel::createPageListModel()
{
    m_inboxObject = QObjectPtr::create();
    m_inboxObject->setProperty("name", tr("Inbox"));
    m_projectsObject = QObjectPtr::create();
    m_projectsObject->setProperty("name", tr("Projects"));

    m_rootsProvider = Domain::QueryResultProvider<QObjectPtr>::Ptr::create();
    m_rootsProvider->append(m_inboxObject);
    m_rootsProvider->append(m_projectsObject);

    auto query = [this](const QObjectPtr &object) -> Domain::QueryResultInterface<QObjectPtr>::Ptr {
        if (!object)
            return Domain::QueryResult<QObjectPtr>::create(m_rootsProvider);
        else if (object == m_projectsObject)
            return Domain::QueryResult<Domain::Project::Ptr, QObjectPtr>::copy(m_projectQueries->findAll());
        else
            return Domain::QueryResult<QObjectPtr>::Ptr();
    };

    auto flags = [](const QObjectPtr &object) {
        const Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable
                                         | Qt::ItemIsEnabled;

        return object.objectCast<Domain::Project>() ? (defaultFlags | Qt::ItemIsEditable) : defaultFlags;
    };

    auto data = [this](const QObjectPtr &object, int role) -> QVariant {
        if (role != Qt::DisplayRole
         && role != Qt::EditRole
         && role != Qt::DecorationRole
         && role != QueryTreeModelBase::IconNameRole) {
            return QVariant();
        }

        if (role == Qt::EditRole
         && (object == m_inboxObject || object == m_projectsObject)) {
            return QVariant();
        }

        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return object->property("name").toString();
        } else if (role == Qt::DecorationRole || role == QueryTreeModelBase::IconNameRole) {
            const QString iconName = object == m_inboxObject ? "mail-folder-inbox"
                                   : object == m_projectsObject ? "folder"
                                   : "view-pim-tasks";

            if (role == Qt::DecorationRole)
                return QVariant::fromValue(QIcon::fromTheme(iconName));
            else
                return iconName;
        } else {
            return QVariant();
        }
    };

    auto setData = [this](const QObjectPtr &object, const QVariant &value, int role) {
        if (role != Qt::EditRole) {
            return false;
        }

        if (object == m_inboxObject || object == m_projectsObject) {
            return false;
        }

        auto project = object.objectCast<Domain::Project>();
        Q_ASSERT(project);
        project->setName(value.toString());
        m_projectRepository->update(project);
        return true;
    };

    return new QueryTreeModel<QObjectPtr>(query, flags, data, setData, this);
}
