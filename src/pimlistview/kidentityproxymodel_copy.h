/*
 *    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
 *        a KDAB Group company, info@kdab.net,
 *        author Stephen Kelly <stephen@kdab.com>
 * 
 *    This library is free software; you can redistribute it and/or modify it
 *    under the terms of the GNU Library General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or (at your
 *    option) any later version.
 * 
 *    This library is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 *    License for more details.
 * 
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to the
 *    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *    02110-1301, USA.
 */

#ifndef KIDENTITYPROXYMODELCOPY_H
#define KIDENTITYPROXYMODELCOPY_H

#include <QtGui/QAbstractProxyModel>


class KIdentityProxyModelCopy : public QAbstractProxyModel
{
    Q_OBJECT
public:
    explicit KIdentityProxyModelCopy(QObject* parent = 0);
    virtual ~KIdentityProxyModelCopy();
    
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;
    virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const;
    virtual QModelIndex parent(const QModelIndex& child) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
    virtual Qt::DropActions supportedDropActions() const;

    virtual QItemSelection mapSelectionFromSource(const QItemSelection& selection) const;
    virtual QItemSelection mapSelectionToSource(const QItemSelection& selection) const;
    virtual QModelIndexList match(const QModelIndex& start, int role, const QVariant& value, int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith|Qt::MatchWrap)) const;
    virtual void setSourceModel(QAbstractItemModel* sourceModel);
    
    virtual bool insertColumns(int column, int count, const QModelIndex& parent = QModelIndex());
    virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex());
    virtual bool removeColumns(int column, int count, const QModelIndex& parent = QModelIndex());
    virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());

    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
        
protected slots:
    virtual void resetInternalData();
    
    virtual void sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
    virtual void sourceRowsInserted(const QModelIndex &parent, int start, int end);
    virtual void sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    virtual void sourceRowsRemoved(const QModelIndex &parent, int start, int end);
    virtual void sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest);
    virtual void sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest);
    
    virtual void sourceColumnsAboutToBeInserted(const QModelIndex &parent, int start, int end);
    virtual void sourceColumnsInserted(const QModelIndex &parent, int start, int end);
    virtual void sourceColumnsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    virtual void sourceColumnsRemoved(const QModelIndex &parent, int start, int end);
    virtual void sourceColumnsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest);
    virtual void sourceColumnsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest);
    
    virtual void sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    virtual void sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last);
    
    virtual void sourceLayoutAboutToBeChanged();
    virtual void sourceLayoutChanged();
    virtual void sourceChildrenLayoutsAboutToBeChanged(const QModelIndex &parent1, const QModelIndex &parent2);
    virtual void sourceChildrenLayoutsChanged(const QModelIndex &parent1, const QModelIndex &parent2);
    virtual void sourceModelAboutToBeReset();
    virtual void sourceModelReset();
    virtual void sourceModelDestroyed();
    
private:
    // Q_DECLARE_PRIVATE(KIdentityProxyModelCopy)
    Q_DISABLE_COPY(KIdentityProxyModelCopy)
    
    bool ignoreNextLayoutAboutToBeChanged;
    bool ignoreNextLayoutChanged;
    QList<QPersistentModelIndex> layoutChangePersistentIndexes;
    QModelIndexList proxyIndexes;
};

#endif // KIDENTITYPROXYMODELCOPY_H
