/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.
*/

#include "librarymodel.h"

#include <KIcon>
#include <KLocale>

LibraryModel::LibraryModel(QObject *parent)
    : QAbstractProxyModel(parent), m_inboxToken(1), m_libraryToken(2),
      m_tokenShift(m_libraryToken+1), m_type(Projects)
{
}

LibraryModel::~LibraryModel()
{

}

LibraryModel::LibraryType LibraryModel::type() const
{
    return m_type;
}

void LibraryModel::setType(LibraryType type)
{
    m_type = type;
}

QModelIndex LibraryModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column!=0) return QModelIndex();

    if (parent==QModelIndex()) {
        switch (row) {
        case 0:
            return createIndex(row, column, (void*)m_inboxToken);
        case 1:
            return createIndex(row, column, (void*)m_libraryToken);
        default:
            return QModelIndex();
        }
    }

    return mapFromSource(sourceModel()->index(row, column, mapToSource(parent)));
}

QModelIndex LibraryModel::parent(const QModelIndex &index) const
{
    if (index.column()!=0 || !index.isValid()
     || isInbox(index) || isLibraryRoot(index)) {
        return QModelIndex();
    }

    return mapFromSource(sourceModel()->parent(mapToSource(index)));
}

int LibraryModel::rowCount(const QModelIndex &parent) const
{
    if (parent==QModelIndex()) {
        return 2;
    }

    if (parent.column()!=0) return -1;

    if (isInbox(parent)) {
        return 0;
    }

    if (isLibraryRoot(parent)) {
        return sourceModel()->rowCount();
    }

    return sourceModel()->rowCount(mapToSource(parent));
}

int LibraryModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QStringList LibraryModel::mimeTypes() const
{
    return sourceModel()->mimeTypes();
}

Qt::DropActions LibraryModel::supportedDropActions() const
{
    return sourceModel()->supportedDropActions();
}

Qt::ItemFlags LibraryModel::flags(const QModelIndex &index) const
{
    if (isInbox(index) || isLibraryRoot(index)) {
        return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
    }

    return QAbstractProxyModel::flags(index);
}

QMimeData *LibraryModel::mimeData(const QModelIndexList &indexes) const
{
    QModelIndexList sourceIndexes;
    foreach (const QModelIndex &proxyIndex, indexes) {
        sourceIndexes << mapToSource(proxyIndex);
    }

    return sourceModel()->mimeData(sourceIndexes);
}

bool LibraryModel::dropMimeData(const QMimeData *mimeData, Qt::DropAction action,
                                int row, int column, const QModelIndex &parent)
{
    QModelIndex sourceParent = mapToSource(parent);
    return sourceModel()->dropMimeData(mimeData, action, row, column, sourceParent);
}

QVariant LibraryModel::data(const QModelIndex &index, int role) const
{
    if (index.column()!=0 || !index.isValid()) return QVariant();

    if (isInbox(index)) {
        switch (role) {
        case Qt::DisplayRole:
            switch (m_type) {
            case Contexts:
                return i18n("No Context");
            default:
                return i18n("Inbox");
            }
        case Qt::DecorationRole:
            return KIcon("mail-folder-inbox");
        default:
            return QVariant();
        }
    }

    if (isLibraryRoot(index)) {
        switch (role) {
        case Qt::DisplayRole:
            switch (m_type) {
            case Contexts:
                return i18n("Contexts");
            default:
                return i18n("Library");
            }
        case Qt::DecorationRole:
            return KIcon("document-multiple");
        default:
            return QVariant();
        }
    }

    return QAbstractProxyModel::data(index, role);
}

QVariant LibraryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return sourceModel()->headerData(section, orientation, role);
}

bool LibraryModel::isInbox(const QModelIndex &index) const
{
    return index.internalId()==m_inboxToken;
}

bool LibraryModel::isLibraryRoot(const QModelIndex &index) const
{
    return index.internalId()==m_libraryToken;
}

QModelIndex LibraryModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (proxyIndex.column()!=0
     || isInbox(proxyIndex)
     || isLibraryRoot(proxyIndex)) {
        return QModelIndex();
    }

    return m_sourceIndexesList[proxyIndex.internalId()-m_tokenShift];
}

QModelIndex LibraryModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid()) {
        return index(1, 0); // Index of the library item
    } else {
        qint64 indexOf = m_sourceIndexesList.indexOf(sourceIndex);
        if (indexOf==-1) {
            indexOf = m_sourceIndexesList.size();
            m_sourceIndexesList << sourceIndex;
        }

        return createIndex(sourceIndex.row(), sourceIndex.column(), (void*)(indexOf+m_tokenShift));
    }
}

void LibraryModel::setSourceModel(QAbstractItemModel *source)
{
    if (sourceModel()) {
        disconnect(sourceModel());
    }

    QAbstractProxyModel::setSourceModel(source);

    connect(sourceModel(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(onSourceDataChanged(const QModelIndex&, const QModelIndex&)));

    connect(sourceModel(), SIGNAL(rowsAboutToBeInserted(const QModelIndex&, int, int)),
            this, SLOT(onSourceRowsAboutToBeInserted(const QModelIndex&, int, int)));
    connect(sourceModel(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(onSourceRowsInserted(const QModelIndex&, int, int)));

    connect(sourceModel(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
            this, SLOT(onSourceRowsAboutToBeRemoved(const QModelIndex&, int, int)));
    connect(sourceModel(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
            this, SLOT(onSourceRowsRemoved(const QModelIndex&, int, int)));

    connect(sourceModel(), SIGNAL(layoutAboutToBeChanged()),
            this, SIGNAL(layoutAboutToBeChanged()));
    connect(sourceModel(), SIGNAL(layoutChanged()),
            this, SLOT(onSourceLayoutChanged()));
}

void LibraryModel::onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end)
{
    emit dataChanged(mapFromSource(begin), mapFromSource(end));
}

void LibraryModel::onSourceRowsAboutToBeInserted(const QModelIndex &sourceIndex, int begin, int end)
{
    m_sourceIndexesList << sourceIndex;
    beginInsertRows(mapFromSource(sourceIndex), begin, end);
}

void LibraryModel::onSourceRowsInserted(const QModelIndex &sourceIndex, int begin, int end)
{
    for (int i=begin; i<=end; i++) {
        QModelIndex child = sourceModel()->index(i, 0, sourceIndex);
        m_sourceIndexesList << child;
    }
    endInsertRows();
}

void LibraryModel::onSourceRowsAboutToBeRemoved(const QModelIndex &sourceIndex, int begin, int end)
{
    beginRemoveRows(mapFromSource(sourceIndex), begin, end);
    for (int i=begin; i<=end; i++) {
        QModelIndex child = sourceModel()->index(i, 0, sourceIndex);
        m_sourceIndexesList.removeAll(child);
    }
}

void LibraryModel::onSourceRowsRemoved(const QModelIndex &/*sourceIndex*/, int /*begin*/, int /*end*/)
{
    endRemoveRows();
}

void LibraryModel::onSourceLayoutChanged()
{
    m_sourceIndexesList.clear();
    emit layoutChanged();
}
