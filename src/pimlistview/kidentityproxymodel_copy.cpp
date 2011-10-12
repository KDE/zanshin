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

#include "kidentityproxymodel_copy.h"

#include <QtGui/QItemSelection>

#include <kdebug.h>

/*!
 *    \since 4.6
 *    \class KIdentityProxyModelCopy
 *    \brief The KIdentityProxyModelCopy class proxies its source model unmodified
 *
 *    \ingroup model-view
 *
 *    KIdentityProxyModelCopy can be used to forward the structure of a source model exactly, with no sorting, filtering or other transformation.
 *    This is similar in concept to an identity matrix where A.I = A.
 *
 *    Because it does no sorting or filtering, this class is most suitable to proxy models which transform the data() of the source model.
 *    For example, a proxy model could be created to define the font used, or the background colour, or the tooltip etc. This removes the
 *    need to implement all data handling in the same class that creates the structure of the model, and can also be used to create
 *    re-usable components.
 *
 *    This also provides a way to change the data in the case where a source model is supplied by a third party which can not be modified.
 *
 *    \code
 *      class DateFormatProxyModel : public QIdentityProxyModel
 *      {
 *        // ...
 *
 *        void setDateFormatString(const QString &formatString)
 *        {
 *          m_formatString = formatString;
 }

 QVariant data(const QModelIndex &index, int role)
 {
     if (role != Qt::DisplayRole)
         return KIdentityProxyModelCopy::data(index, role);

     const QDateTime dateTime = sourceModel()->data(SourceClass::DateRole).toDateTime();

     return dateTime.toString(m_formatString);
 }

 private:
     QString m_formatString;
 };
 \endcode

 \since 4.6
 \author Stephen Kelly <stephen@kdab.com>

 */

/*!
 *    Constructs an identity model with the given \a parent.
 */
KIdentityProxyModelCopy::KIdentityProxyModelCopy(QObject* parent)
: QAbstractProxyModel(parent), ignoreNextLayoutAboutToBeChanged(false),
ignoreNextLayoutChanged(false)
{

}

/*!
 *    Destroys this identity model.
 */
KIdentityProxyModelCopy::~KIdentityProxyModelCopy()
{

}

/*!
 *    \reimp
 */
int KIdentityProxyModelCopy::columnCount(const QModelIndex& parent) const
{
    if (!sourceModel())
        return 0;
    Q_ASSERT(parent.isValid() ? parent.model() == this : true);
    return sourceModel()->columnCount(mapToSource(parent));
}

/*!
 *    \reimp
 */
bool KIdentityProxyModelCopy::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    if (!sourceModel())
        return false;
    Q_ASSERT(parent.isValid() ? parent.model() == this : true);
    return sourceModel()->dropMimeData(data, action, row, column, mapToSource(parent));
}

/*!
 *    \reimp
 */
QModelIndex KIdentityProxyModelCopy::index(int row, int column, const QModelIndex& parent) const
{
    if (!sourceModel())
        return QModelIndex();
    Q_ASSERT(parent.isValid() ? parent.model() == this : true);
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    const QModelIndex sourceParent = mapToSource(parent);
    const QModelIndex sourceIndex = sourceModel()->index(row, column, sourceParent);
    Q_ASSERT(sourceIndex.isValid());
    return mapFromSource(sourceIndex);
}

/*!
 *    \reimp
 */
bool KIdentityProxyModelCopy::insertColumns(int column, int count, const QModelIndex& parent)
{
    if (!sourceModel())
        return false;
    Q_ASSERT(parent.isValid() ? parent.model() == this : true);
    return sourceModel()->insertColumns(column, count, mapToSource(parent));
}

/*!
 *    \reimp
 */
bool KIdentityProxyModelCopy::insertRows(int row, int count, const QModelIndex& parent)
{
    if (!sourceModel())
        return false;
    Q_ASSERT(parent.isValid() ? parent.model() == this : true);
    return sourceModel()->insertRows(row, count, mapToSource(parent));
}

/*!
 *    \reimp
 */
QModelIndex KIdentityProxyModelCopy::mapFromSource(const QModelIndex& sourceIndex) const
{
    if (!sourceModel() || !sourceIndex.isValid())
        return QModelIndex();

    Q_ASSERT(sourceIndex.model() == sourceModel());
    return createIndex(sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer());
}

/*!
 *    \reimp
 */
QItemSelection KIdentityProxyModelCopy::mapSelectionFromSource(const QItemSelection& selection) const
{
    kDebug();
    QItemSelection proxySelection;

    if (!sourceModel())
        return proxySelection;

    QItemSelection::const_iterator it = selection.constBegin();
    const QItemSelection::const_iterator end = selection.constEnd();
    for ( ; it != end; ++it) {
        Q_ASSERT(it->model() == sourceModel());
        const QItemSelectionRange range(mapFromSource(it->topLeft()), mapFromSource(it->bottomRight()));
        proxySelection.append(range);
    }

    return proxySelection;
}

/*!
 *    \reimp
 */
QItemSelection KIdentityProxyModelCopy::mapSelectionToSource(const QItemSelection& selection) const
{
    kDebug();
    QItemSelection sourceSelection;

    if (!sourceModel())
        return sourceSelection;

    QItemSelection::const_iterator it = selection.constBegin();
    const QItemSelection::const_iterator end = selection.constEnd();
    for ( ; it != end; ++it) {
        Q_ASSERT(it->model() == this);
        const QItemSelectionRange range(mapToSource(it->topLeft()), mapToSource(it->bottomRight()));
        sourceSelection.append(range);
    }

    return sourceSelection;
}

struct SourceModelIndex
{
    SourceModelIndex(int _r, int _c, void *_p, QAbstractItemModel *_m)
    : r(_r), c(_c), p(_p), m(_m)
    {

    }

    operator QModelIndex() { return reinterpret_cast<QModelIndex&>(*this); }

    int r, c;
    void *p;
    const QAbstractItemModel *m;
};

/*!
 *    \reimp
 */
QModelIndex KIdentityProxyModelCopy::mapToSource(const QModelIndex& proxyIndex) const
{
    if (!sourceModel() || !proxyIndex.isValid())
        return QModelIndex();
    Q_ASSERT(proxyIndex.model() == this);
    return SourceModelIndex(proxyIndex.row(), proxyIndex.column(), proxyIndex.internalPointer(), sourceModel());
}

/*!
 *    \reimp
 */
QModelIndexList KIdentityProxyModelCopy::match(const QModelIndex& start, int role, const QVariant& value, int hits, Qt::MatchFlags flags) const
{
    Q_ASSERT(start.isValid() ? start.model() == this : true);
    if (!sourceModel())
        return QModelIndexList();

    const QModelIndexList sourceList = sourceModel()->match(mapToSource(start), role, value, hits, flags);
    QModelIndexList::const_iterator it = sourceList.constBegin();
    const QModelIndexList::const_iterator end = sourceList.constEnd();
    QModelIndexList proxyList;
    for ( ; it != end; ++it)
        proxyList.append(mapFromSource(*it));
    return proxyList;
}

/*!
 *    \reimp
 */
QModelIndex KIdentityProxyModelCopy::parent(const QModelIndex& child) const
{
    if (!sourceModel())
        return QModelIndex();

    Q_ASSERT(child.isValid() ? child.model() == this : true);
    const QModelIndex sourceIndex = mapToSource(child);
    const QModelIndex sourceParent = sourceIndex.parent();
    return mapFromSource(sourceParent);
}

/*!
 *    \reimp
 */
bool KIdentityProxyModelCopy::removeColumns(int column, int count, const QModelIndex& parent)
{
    if (!sourceModel())
        return false;

    Q_ASSERT(parent.isValid() ? parent.model() == this : true);
    return sourceModel()->removeColumns(column, count, mapToSource(parent));
}

/*!
 *    \reimp
 */
bool KIdentityProxyModelCopy::removeRows(int row, int count, const QModelIndex& parent)
{
    if (!sourceModel())
        return false;

    Q_ASSERT(parent.isValid() ? parent.model() == this : true);
    return sourceModel()->removeRows(row, count, mapToSource(parent));
}

/*!
 *    \reimp
 */
int KIdentityProxyModelCopy::rowCount(const QModelIndex& parent) const
{
    if (!sourceModel())
        return 0;
    Q_ASSERT(parent.isValid() ? parent.model() == this : true);
    return sourceModel()->rowCount(mapToSource(parent));
}

/*!
 *    \reimp
 */
void KIdentityProxyModelCopy::setSourceModel(QAbstractItemModel* sourceModel)
{
    beginResetModel();

    if (sourceModel) {
        disconnect(sourceModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
                   this, SLOT(sourceRowsAboutToBeInserted(const QModelIndex &, int, int)));
        disconnect(sourceModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                   this, SLOT(sourceRowsInserted(const QModelIndex &, int, int)));
        disconnect(sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
                   this, SLOT(sourceRowsAboutToBeRemoved(const QModelIndex &, int, int)));
        disconnect(sourceModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                   this, SLOT(sourceRowsRemoved(const QModelIndex &, int, int)));
        disconnect(sourceModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                   this, SLOT(sourceRowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        disconnect(sourceModel, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                   this, SLOT(sourceRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        disconnect(sourceModel, SIGNAL(columnsAboutToBeInserted(const QModelIndex &, int, int)),
                   this, SLOT(sourceColumnsAboutToBeInserted(const QModelIndex &, int, int)));
        disconnect(sourceModel, SIGNAL(columnsInserted(const QModelIndex &, int, int)),
                   this, SLOT(sourceColumnsInserted(const QModelIndex &, int, int)));
        disconnect(sourceModel, SIGNAL(columnsAboutToBeRemoved(const QModelIndex &, int, int)),
                   this, SLOT(sourceColumnsAboutToBeRemoved(const QModelIndex &, int, int)));
        disconnect(sourceModel, SIGNAL(columnsRemoved(const QModelIndex &, int, int)),
                   this, SLOT(sourceColumnsRemoved(const QModelIndex &, int, int)));
        disconnect(sourceModel, SIGNAL(columnsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                   this, SLOT(sourceColumnsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        disconnect(sourceModel, SIGNAL(columnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                   this, SLOT(sourceColumnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        disconnect(sourceModel, SIGNAL(modelAboutToBeReset()),
                   this, SLOT(sourceModelAboutToBeReset()));
        //        disconnect(sourceModel, SIGNAL(internalDataReset()),
        //                   this, SLOT(resetInternalData()));
        disconnect(sourceModel, SIGNAL(modelReset()),
                   this, SLOT(sourceModelReset()));
        disconnect(sourceModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                   this, SLOT(sourceDataChanged(const QModelIndex &, const QModelIndex &)));
        disconnect(sourceModel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
                   this, SLOT(sourceHeaderDataChanged(Qt::Orientation,int,int)));
        disconnect(sourceModel, SIGNAL(layoutAboutToBeChanged()),
                   this, SLOT(sourceLayoutAboutToBeChanged()));
        disconnect(sourceModel, SIGNAL(layoutChanged()),
                   this, SLOT(sourceLayoutChanged()));
        //         disconnect(sourceModel, SIGNAL(childrenLayoutsAboutToBeChanged(QModelIndex,QModelIndex)),
        //                    this, SLOT(sourceChildrenLayoutsAboutToBeChanged(QModelIndex,QModelIndex)));
        //         disconnect(sourceModel, SIGNAL(childrenLayoutsChanged(QModelIndex,QModelIndex)),
        //                    this, SLOT(sourceChildrenLayoutsChanged(QModelIndex,QModelIndex)));
        disconnect(sourceModel, SIGNAL(destroyed()),
                   this, SLOT(sourceModelDestroyed()));
    }

    QAbstractProxyModel::setSourceModel(sourceModel);

    if (sourceModel) {
        connect(sourceModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
                SLOT(sourceRowsAboutToBeInserted(const QModelIndex &, int, int)));
        connect(sourceModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                SLOT(sourceRowsInserted(const QModelIndex &, int, int)));
        connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
                SLOT(sourceRowsAboutToBeRemoved(const QModelIndex &, int, int)));
        connect(sourceModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                SLOT(sourceRowsRemoved(const QModelIndex &, int, int)));
        connect(sourceModel, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                SLOT(sourceRowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        connect(sourceModel, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                SLOT(sourceRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        connect(sourceModel, SIGNAL(columnsAboutToBeInserted(const QModelIndex &, int, int)),
                SLOT(sourceColumnsAboutToBeInserted(const QModelIndex &, int, int)));
        connect(sourceModel, SIGNAL(columnsInserted(const QModelIndex &, int, int)),
                SLOT(sourceColumnsInserted(const QModelIndex &, int, int)));
        connect(sourceModel, SIGNAL(columnsAboutToBeRemoved(const QModelIndex &, int, int)),
                SLOT(sourceColumnsAboutToBeRemoved(const QModelIndex &, int, int)));
        connect(sourceModel, SIGNAL(columnsRemoved(const QModelIndex &, int, int)),
                SLOT(sourceColumnsRemoved(const QModelIndex &, int, int)));
        connect(sourceModel, SIGNAL(columnsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                SLOT(sourceColumnsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        connect(sourceModel, SIGNAL(columnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                SLOT(sourceColumnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
        connect(sourceModel, SIGNAL(modelAboutToBeReset()),
                SLOT(sourceModelAboutToBeReset()));
        //        connect(sourceModel, SIGNAL(internalDataReset()),
        //                SLOT(resetInternalData()));
        connect(sourceModel, SIGNAL(modelReset()),
                SLOT(sourceModelReset()));
        connect(sourceModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                SLOT(sourceDataChanged(const QModelIndex &, const QModelIndex &)));
        connect(sourceModel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
                SLOT(sourceHeaderDataChanged(Qt::Orientation,int,int)));
        connect(sourceModel, SIGNAL(layoutAboutToBeChanged()),
                SLOT(sourceLayoutAboutToBeChanged()));
        connect(sourceModel, SIGNAL(layoutChanged()),
                SLOT(sourceLayoutChanged()));
        // Hopefully this will be in Qt4.8
        //         connect(sourceModel, SIGNAL(childrenLayoutsAboutToBeChanged(QModelIndex,QModelIndex)),
        //                 SLOT(sourceChildrenLayoutsAboutToBeChanged(QModelIndex,QModelIndex)));
        //         connect(sourceModel, SIGNAL(childrenLayoutsChanged(QModelIndex,QModelIndex)),
        //                 SLOT(sourceChildrenLayoutsChanged(QModelIndex,QModelIndex)));
        connect(sourceModel, SIGNAL(destroyed()),
                SLOT(sourceModelDestroyed()));
    }

    endResetModel();
}

void KIdentityProxyModelCopy::sourceColumnsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{

    Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
    beginInsertColumns(mapFromSource(parent), start, end);
}

void KIdentityProxyModelCopy::sourceColumnsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest)
{

    Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == sourceModel() : true);
    Q_ASSERT(destParent.isValid() ? destParent.model() == sourceModel() : true);
    beginMoveColumns(mapFromSource(sourceParent), sourceStart, sourceEnd, mapFromSource(destParent), dest);
}

void KIdentityProxyModelCopy::sourceColumnsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{

    Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
    beginRemoveColumns(mapFromSource(parent), start, end);
}

void KIdentityProxyModelCopy::sourceColumnsInserted(const QModelIndex &parent, int start, int end)
{

    Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)
    endInsertColumns();
}

void KIdentityProxyModelCopy::sourceColumnsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest)
{

    Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == sourceModel() : true);
    Q_ASSERT(destParent.isValid() ? destParent.model() == sourceModel() : true);
    Q_UNUSED(sourceParent)
    Q_UNUSED(sourceStart)
    Q_UNUSED(sourceEnd)
    Q_UNUSED(destParent)
    Q_UNUSED(dest)
    endMoveColumns();
}

void KIdentityProxyModelCopy::sourceColumnsRemoved(const QModelIndex &parent, int start, int end)
{

    Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)
    endRemoveColumns();
}

void KIdentityProxyModelCopy::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{

    Q_ASSERT(topLeft.model() == sourceModel());
    Q_ASSERT(bottomRight.model() == sourceModel());
    dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight));
}

void KIdentityProxyModelCopy::sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{

    headerDataChanged(orientation, first, last);
}

void KIdentityProxyModelCopy::sourceLayoutAboutToBeChanged()
{
    kDebug();

    if (ignoreNextLayoutAboutToBeChanged)
        return;

    //remember all persistent indexes and their corresponding sourceindex
    //FIXME doesn't make sense as we have persistent indexes to source and proxy items
    Q_FOREACH(const QPersistentModelIndex &proxyPersistentIndex, persistentIndexList()) {
        Q_ASSERT(proxyPersistentIndex.isValid());
        const QPersistentModelIndex srcPersistentIndex = mapToSource(proxyPersistentIndex);
        //Q_ASSERT(srcPersistentIndex.isValid());
        if (srcPersistentIndex.isValid()) { //all items which were created in this model (respectively a subclass) are not available in the sourcemodel
            proxyIndexes << proxyPersistentIndex;
            layoutChangePersistentIndexes << srcPersistentIndex;
        }
    }

    emit layoutAboutToBeChanged();
}

void KIdentityProxyModelCopy::sourceLayoutChanged()
{
    kDebug();
    if (ignoreNextLayoutChanged)
        return;

    //change the proxyindexes to new ones, with the correct layout
        //mapFromSource must therefore return the new correct proxy index, which refers to the old sourceindex
        //the internalid stays, but the row changes
    for (int i = 0; i < proxyIndexes.size(); ++i) {
        changePersistentIndex(proxyIndexes.at(i), mapFromSource(layoutChangePersistentIndexes.at(i)));
    }

    layoutChangePersistentIndexes.clear();
    proxyIndexes.clear();

    emit layoutChanged();
}


void KIdentityProxyModelCopy::sourceChildrenLayoutsAboutToBeChanged(const QModelIndex &parent1, const QModelIndex &parent2)
{

    Q_ASSERT(parent1.isValid() ? parent1.model() == sourceModel() : true);
    Q_ASSERT(parent2.isValid() ? parent2.model() == sourceModel() : true);


    ignoreNextLayoutAboutToBeChanged = true;

    const QModelIndex proxyParent1 = mapFromSource(parent1);
    const QModelIndex proxyParent2 = mapFromSource(parent2);
    //emit childrenLayoutsAboutToBeChanged(proxyParent1, proxyParent2);
    emit layoutAboutToBeChanged();

    if (persistentIndexList().isEmpty())
        return;

    Q_FOREACH(const QPersistentModelIndex &proxyPersistentIndex, persistentIndexList()) {
        const QPersistentModelIndex srcPersistentIndex = mapToSource(proxyPersistentIndex);
        Q_ASSERT(proxyPersistentIndex.isValid());
        Q_ASSERT(srcPersistentIndex.isValid());
        const QModelIndex idxParent = srcPersistentIndex.parent();
        if (idxParent != parent1 && idxParent != parent2)
            continue;
        proxyIndexes << proxyPersistentIndex;
        layoutChangePersistentIndexes << srcPersistentIndex;
    }
}

void KIdentityProxyModelCopy::sourceChildrenLayoutsChanged(const QModelIndex &parent1, const QModelIndex &parent2)
{

    Q_ASSERT(parent1.isValid() ? parent1.model() == sourceModel() : true);
    Q_ASSERT(parent2.isValid() ? parent2.model() == sourceModel() : true);

    ignoreNextLayoutChanged = true;

    QModelIndexList oldList, newList;
    for( int i = 0; i < layoutChangePersistentIndexes.size(); ++i) {
        const QModelIndex srcIdx = layoutChangePersistentIndexes.at(i);
        const QModelIndex oldProxyIdx = proxyIndexes.at(i);
        oldList << oldProxyIdx;
        newList << mapFromSource(srcIdx);
    }
    changePersistentIndexList(oldList, newList);
    layoutChangePersistentIndexes.clear();
    proxyIndexes.clear();

    const QModelIndex proxyParent1 = mapFromSource(parent1);
    const QModelIndex proxyParent2 = mapFromSource(parent2);
    //     emit childrenLayoutsChanged(proxyParent1, proxyParent2);
    emit layoutChanged();
}

void KIdentityProxyModelCopy::sourceModelAboutToBeReset()
{
    kDebug();
    beginResetModel();
}

void KIdentityProxyModelCopy::sourceModelReset()
{
    kDebug();
    endResetModel();
}

void KIdentityProxyModelCopy::sourceModelDestroyed()
{

    //   endResetModel();
}

void KIdentityProxyModelCopy::sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{

    Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
    beginInsertRows(mapFromSource(parent), start, end);
}

void KIdentityProxyModelCopy::sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest)
{

    Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == sourceModel() : true);
    Q_ASSERT(destParent.isValid() ? destParent.model() == sourceModel() : true);
    beginMoveRows(mapFromSource(sourceParent), sourceStart, sourceEnd, mapFromSource(destParent), dest);
}

void KIdentityProxyModelCopy::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{

    Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
    beginRemoveRows(mapFromSource(parent), start, end);
}

void KIdentityProxyModelCopy::sourceRowsInserted(const QModelIndex &parent, int start, int end)
{

    Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)
    endInsertRows();
}

void KIdentityProxyModelCopy::sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest)
{

    Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == sourceModel() : true);
    Q_ASSERT(destParent.isValid() ? destParent.model() == sourceModel() : true);
    Q_UNUSED(sourceParent)
    Q_UNUSED(sourceStart)
    Q_UNUSED(sourceEnd)
    Q_UNUSED(destParent)
    Q_UNUSED(dest)
    endMoveRows();
}

void KIdentityProxyModelCopy::sourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)
    endRemoveRows();
}

void KIdentityProxyModelCopy::sort(int column, Qt::SortOrder order)
{
    kDebug();
    if (!sourceModel()) {
        return;
    }
    kDebug() << column << order;
    sourceModel()->sort(column, order);
}


/*!
 *  This slot is automatically invoked when the model is reset. It can be used to clear any data internal to the proxy at the approporiate time.
 *
 *  \sa QAbstractItemModel::internalDataReset()
 */
void KIdentityProxyModelCopy::resetInternalData()
{

}

Qt::DropActions KIdentityProxyModelCopy::supportedDropActions() const
{
    if (!sourceModel()) {
        return 0;
    }
    return sourceModel()->supportedDropActions();
}


#include "kidentityproxymodel_copy.moc"
