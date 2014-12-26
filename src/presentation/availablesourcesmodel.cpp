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


#include "availablesourcesmodel.h"

#include <QIcon>

#include "domain/datasourcequeries.h"
#include "domain/datasourcerepository.h"

#include "presentation/querytreemodel.h"
#include "presentation/errorhandler.h"

using namespace Presentation;

AvailableSourcesModel::AvailableSourcesModel(const Domain::DataSourceQueries::Ptr &dataSourceQueries,
                                             const Domain::DataSourceRepository::Ptr &dataSourceRepository,
                                             QObject *parent)
    : QObject(parent),
      m_sourceListModel(0),
      m_searchListModel(0),
      m_dataSourceQueries(dataSourceQueries),
      m_dataSourceRepository(dataSourceRepository),
      m_errorHandler(0)
{
}

QAbstractItemModel *AvailableSourcesModel::sourceListModel()
{
    if (!m_sourceListModel)
        m_sourceListModel = createSourceListModel();
    return m_sourceListModel;
}

QAbstractItemModel *AvailableSourcesModel::searchListModel()
{
    if (!m_searchListModel)
        m_searchListModel = createSearchListModel();
    return m_searchListModel;
}

void AvailableSourcesModel::listSource(const Domain::DataSource::Ptr &source)
{
    Q_ASSERT(source);
    source->setSelected(true);
    source->setListStatus(Domain::DataSource::Listed);
    const auto job = m_dataSourceRepository->update(source);
    if (m_errorHandler)
        m_errorHandler->installHandler(job, tr("Cannot modify source %1").arg(source->name()));
}

void AvailableSourcesModel::unlistSource(const Domain::DataSource::Ptr &source)
{
    Q_ASSERT(source);
    source->setSelected(false);
    source->setListStatus(Domain::DataSource::Unlisted);
    const auto job = m_dataSourceRepository->update(source);
    if (m_errorHandler)
        m_errorHandler->installHandler(job, tr("Cannot modify source %1").arg(source->name()));
}

void AvailableSourcesModel::bookmarkSource(const Domain::DataSource::Ptr &source)
{
    Q_ASSERT(source);
    if (source->listStatus() == Domain::DataSource::Bookmarked)
        source->setListStatus(Domain::DataSource::Listed);
    else
        source->setListStatus(Domain::DataSource::Bookmarked);
    const auto job = m_dataSourceRepository->update(source);
    if (m_errorHandler)
        m_errorHandler->installHandler(job, tr("Cannot modify source %1").arg(source->name()));
}

QAbstractItemModel *AvailableSourcesModel::createSourceListModel()
{
    auto query = [this] (const Domain::DataSource::Ptr &source) {
        if (!source)
            return m_dataSourceQueries->findTopLevel();
        else
            return m_dataSourceQueries->findChildren(source);
    };

    auto flags = [] (const Domain::DataSource::Ptr &source) {
        const Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable
                                         | Qt::ItemIsEnabled;
        if (source->contentTypes() != Domain::DataSource::NoContent)
            return defaultFlags | Qt::ItemIsUserCheckable;
        else
            return defaultFlags;
    };

    auto data = [] (const Domain::DataSource::Ptr &source, int role) -> QVariant {
        if (role != Qt::DisplayRole
         && role != Qt::EditRole
         && role != Qt::DecorationRole
         && role != Qt::CheckStateRole
         && role != QueryTreeModelBase::IconNameRole) {
            return QVariant();
        }

        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return source->name();
        } else if (role == Qt::DecorationRole || role == QueryTreeModelBase::IconNameRole) {
            const QString iconName = source->iconName().isEmpty() ? "folder" : source->iconName();

            if (role == Qt::DecorationRole)
                return QVariant::fromValue(QIcon::fromTheme(iconName));
            else
                return iconName;
        } else if (role == Qt::CheckStateRole) {
            if (source->contentTypes() != Domain::DataSource::NoContent)
                return source->isSelected() ? Qt::Checked : Qt::Unchecked;
            else
                return QVariant();
        } else {
            return QVariant();
        }
    };

    auto setData = [this] (const Domain::DataSource::Ptr &source, const QVariant &value, int role) {
        if (role != Qt::CheckStateRole)
            return false;
        if (source->contentTypes() == Domain::DataSource::NoContent)
            return false;

        source->setSelected(value.toInt() == Qt::Checked);
        const auto job = m_dataSourceRepository->update(source);
        if (m_errorHandler)
            m_errorHandler->installHandler(job, tr("Cannot modify source %1").arg(source->name()));
        return true;
    };

    auto drop = [] (const QMimeData *mimeData, Qt::DropAction, const Domain::DataSource::Ptr &source) {
        Q_UNUSED(mimeData)
        Q_UNUSED(source)
        return false;
    };

    auto drag = [](const Domain::DataSource::List &) -> QMimeData* {
        return 0;
    };

    return new QueryTreeModel<Domain::DataSource::Ptr>(query, flags, data, setData, drop, drag, this);
}

QAbstractItemModel *AvailableSourcesModel::createSearchListModel()
{
    auto query = [this] (const Domain::DataSource::Ptr &source) {
        if (!source)
            return m_dataSourceQueries->findSearchTopLevel();
        else
            return m_dataSourceQueries->findSearchChildren(source);
    };

    auto flags = [] (const Domain::DataSource::Ptr &source) {
        Q_UNUSED(source)
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    };

    auto data = [] (const Domain::DataSource::Ptr &source, int role) -> QVariant {
        if (role != Qt::DisplayRole
         && role != Qt::EditRole
         && role != Qt::DecorationRole
         && role != QueryTreeModelBase::IconNameRole) {
            return QVariant();
        }

        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return source->name();
        } else if (role == Qt::DecorationRole || role == QueryTreeModelBase::IconNameRole) {
            const QString iconName = source->iconName().isEmpty() ? "folder" : source->iconName();

            if (role == Qt::DecorationRole)
                return QVariant::fromValue(QIcon::fromTheme(iconName));
            else
                return iconName;
        } else {
            return QVariant();
        }
    };

    auto setData = [this] (const Domain::DataSource::Ptr &source, const QVariant &value, int role) {
        Q_UNUSED(source)
        Q_UNUSED(value)
        Q_UNUSED(role)
        return false;
    };

    auto drop = [] (const QMimeData *mimeData, Qt::DropAction, const Domain::DataSource::Ptr &source) {
        Q_UNUSED(mimeData)
        Q_UNUSED(source)
        return false;
    };

    auto drag = [](const Domain::DataSource::List &) -> QMimeData* {
        return 0;
    };

    return new QueryTreeModel<Domain::DataSource::Ptr>(query, flags, data, setData, drop, drag, this);
}

QString AvailableSourcesModel::searchTerm() const
{
    return m_dataSourceQueries->searchTerm();
}

void AvailableSourcesModel::setSearchTerm(const QString &term)
{
    if (term == searchTerm())
        return;

    m_dataSourceQueries->setSearchTerm(term);
    emit searchTermChanged(term);
}

ErrorHandler *AvailableSourcesModel::errorHandler() const
{
    return m_errorHandler;
}

void AvailableSourcesModel::setErrorHandler(ErrorHandler *errorHandler)
{
    m_errorHandler = errorHandler;
}
