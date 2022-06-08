/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "availablesourcesmodel.h"

#include <QIcon>

#include <KLocalizedString>

#include "domain/datasourcequeries.h"
#include "domain/datasourcerepository.h"

#include "presentation/querytreemodel.h"

using namespace Presentation;

AvailableSourcesModel::AvailableSourcesModel(const Domain::DataSourceQueries::Ptr &dataSourceQueries,
                                             const Domain::DataSourceRepository::Ptr &dataSourceRepository,
                                             QObject *parent)
    : QObject(parent),
      m_sourceListModel(nullptr),
      m_dataSourceQueries(dataSourceQueries),
      m_dataSourceRepository(dataSourceRepository)
{
}

QAbstractItemModel *AvailableSourcesModel::sourceListModel()
{
    if (!m_sourceListModel)
        m_sourceListModel = createSourceListModel();
    return m_sourceListModel;
}

void AvailableSourcesModel::showConfigDialog()
{
    m_dataSourceRepository->showConfigDialog();
}

QAbstractItemModel *AvailableSourcesModel::createSourceListModel()
{
    auto query = [this] (const Domain::DataSource::Ptr &source) {
        if (!source)
            return m_dataSourceQueries->findTopLevel();
        else
            return m_dataSourceQueries->findChildren(source);
    };

    auto flags = [] (const Domain::DataSource::Ptr &source) -> Qt::ItemFlags {
        const Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable
                                         | Qt::ItemIsEnabled;
        if (source->contentTypes() != Domain::DataSource::NoContent)
            return defaultFlags | Qt::ItemIsUserCheckable;
        else
            return defaultFlags;
    };

    auto data = [this] (const Domain::DataSource::Ptr &source, int role, int) -> QVariant {
        if (role != Qt::DisplayRole
         && role != Qt::EditRole
         && role != Qt::DecorationRole
         && role != Qt::CheckStateRole
         && role != QueryTreeModelBase::IconNameRole
         && role != QueryTreeModelBase::IsDefaultRole) {
            return QVariant();
        }

        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return source->name();
        } else if (role == Qt::DecorationRole || role == QueryTreeModelBase::IconNameRole) {
            const QString iconName = source->iconName().isEmpty() ? QStringLiteral("folder") : source->iconName();

            if (role == Qt::DecorationRole)
                return QVariant::fromValue(QIcon::fromTheme(iconName));
            else
                return iconName;
        } else if (role == Qt::CheckStateRole) {
            if (source->contentTypes() != Domain::DataSource::NoContent)
                return source->isSelected() ? Qt::Checked : Qt::Unchecked;
            else
                return QVariant();
        } else if (role == QueryTreeModelBase::IsDefaultRole) {
            return m_dataSourceQueries->isDefaultSource(source);
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
        installHandler(job, i18n("Cannot modify source %1", source->name()));
        return true;
    };

    auto drop = [] (const QMimeData *mimeData, Qt::DropAction, const Domain::DataSource::Ptr &source) {
        Q_UNUSED(mimeData)
        Q_UNUSED(source)
        return false;
    };

    auto drag = [](const Domain::DataSource::List &) -> QMimeData* {
        return nullptr;
    };

    connect(m_dataSourceQueries->notifier(), &Domain::DataSourceQueriesNotifier::defaultSourceChanged,
            this, &AvailableSourcesModel::onDefaultSourceChanged);
    return new QueryTreeModel<Domain::DataSource::Ptr>(query, flags, data, setData, drop, drag, nullptr, this);
}

void AvailableSourcesModel::setDefaultItem(const QModelIndex &index)
{
    auto source = index.data(QueryTreeModelBase::ObjectRole).value<Domain::DataSource::Ptr>();
    Q_ASSERT(source);
    m_dataSourceQueries->setDefaultSource(source);
}

void AvailableSourcesModel::onDefaultSourceChanged()
{
    emitDefaultSourceChanged(QModelIndex());
}

void AvailableSourcesModel::emitDefaultSourceChanged(const QModelIndex &root)
{
    const auto rowCount = m_sourceListModel->rowCount(root);
    for (int row = 0; row < rowCount; row++) {
        const auto index = m_sourceListModel->index(row, 0, root);
        Q_EMIT m_sourceListModel->dataChanged(index, index);
        emitDefaultSourceChanged(index);
    }
}
