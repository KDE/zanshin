/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>
   Copyright 2008, 2009 Mario Bensi <nef@ipsquad.net>

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

#ifndef ZANSHIN_LIBRARYMODEL_H
#define ZANSHIN_LIBRARYMODEL_H

#include <QtCore/QHash>
#include <QtGui/QAbstractProxyModel>

class LibraryModel : public QAbstractProxyModel
{
    Q_OBJECT

public:
    enum LibraryType {
        Projects = 0,
        Contexts
    };


    LibraryModel(QObject *parent = 0);
    virtual ~LibraryModel();

    LibraryType type() const;
    void setType(LibraryType type);

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    Qt::DropActions supportedDropActions() const;
    virtual QStringList mimeTypes() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;

    bool isInbox(const QModelIndex &index) const;
    bool isLibraryRoot(const QModelIndex &index) const;

    virtual void setSourceModel(QAbstractItemModel *sourceModel);

    virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
    virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;

private slots:
    void onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end);
    void onSourceRowsAboutToBeInserted(const QModelIndex &sourceIndex, int begin, int end);
    void onSourceRowsInserted(const QModelIndex &sourceIndex, int begin, int end);
    void onSourceRowsAboutToBeRemoved(const QModelIndex &sourceIndex, int begin, int end);
    void onSourceRowsRemoved(const QModelIndex &sourceIndex, int begin, int end);
    void onSourceLayoutChanged();

private:
    const qint64 m_inboxToken;
    const qint64 m_libraryToken;
    const qint64 m_tokenShift;
    mutable QList<QPersistentModelIndex> m_sourceIndexesList;
    LibraryType m_type;
};

#endif

