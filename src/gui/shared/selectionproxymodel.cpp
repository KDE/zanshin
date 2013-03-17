/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>

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

#include "selectionproxymodel.h"

#include <QtGui/QItemSelectionModel>
#include <QtCore/QSize>

#include <KDebug>

#include <kmodelindexproxymapper.h>

class TodoModel;

SelectionProxyModel::SelectionProxyModel(QObject *parent)
    : KRecursiveFilterProxyModel(parent),
      m_selectionModel(0)
{
    setDynamicSortFilter(true);
}

SelectionProxyModel::~SelectionProxyModel()
{
}

void SelectionProxyModel::setSelectionModel(QItemSelectionModel *selectionModel)
{
    if (m_selectionModel == selectionModel) {
        return;
    }

    if (m_selectionModel) {
        disconnect(m_selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)));
        disconnect(m_selectionModel->model(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
    }

    m_selectionModel = selectionModel;

    if (selectionModel) {
        connect(selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this, SLOT(onSelectionChanged(QItemSelection,QItemSelection)));
        connect(selectionModel->model(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(onSourceRemoveRows(QModelIndex,int,int)));
    }

    initializeSelection();
    invalidate();
}

void SelectionProxyModel::setSourceModel(QAbstractItemModel *model)
{
    if (model == sourceModel()) {
        return;
    }
    KRecursiveFilterProxyModel::setSourceModel(model);
    initializeSelection();
}

void SelectionProxyModel::initializeSelection()
{
    m_selectionChain.clear();
    m_sourceChain.clear();
    m_selectedRows.clear();
    m_sourceSelectedRows.clear();

    if (!m_selectionModel || !sourceModel()) {
        return;
    }

    QList<QAbstractItemModel*> selectionStack = buildModelStack(const_cast<QAbstractItemModel*>(m_selectionModel->model()));
    QList<QAbstractItemModel*> sourceStack = buildModelStack(sourceModel());
    QAbstractItemModel *commonModel = findCommonModel(selectionStack, sourceStack);

    Q_ASSERT(commonModel!=0);

    m_selectionChain = createProxyChain(selectionStack, commonModel, false);
    m_sourceChain = createProxyChain(sourceStack, commonModel, true);

    onSelectionChanged(QItemSelection(), QItemSelection());
}

QVariant SelectionProxyModel::data(const QModelIndex &index, int role) const
{
    if (role==Qt::SizeHintRole) {
        QModelIndex sourceRowIndex = mapToSource(index.sibling(index.row(), 0));
        while (sourceRowIndex.isValid()) {
            if (m_sourceSelectedRows.contains(sourceRowIndex)) {
                return KRecursiveFilterProxyModel::data(index, role);
            }
            sourceRowIndex = sourceRowIndex.parent();
        }

        return QSize(0, 0); // Ultimately we don't want to display those

    } else {
        return KRecursiveFilterProxyModel::data(index, role);
    }
}

bool SelectionProxyModel::acceptRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);

    while (sourceIndex.isValid()) {
        if (m_sourceSelectedRows.contains(sourceIndex)) {
            return true;
        }
        sourceIndex = sourceIndex.parent();
    }

    return false;
}

void SelectionProxyModel::onSelectionChanged(const QItemSelection &/*selected*/, const QItemSelection &/*deselected*/)
{
#if QT_VERSION < 0x040800
    // The QItemSelectionModel sometimes doesn't remove deselected items from its selection
    // Fixed in Qt 4.8 : http://qt.gitorious.org/qt/qt/merge_requests/2403
    QItemSelection selection = m_selectionModel->selection();
    selection.merge(deselected, QItemSelectionModel::Deselect);
#else
    QItemSelection selection = m_selectionModel->selection();
#endif

    m_selectedRows.clear();
    m_sourceSelectedRows.clear();

    foreach (const QModelIndex &index, selection.indexes()) {
        if (index.column()==0) {
            kDebug() << "Added index:" << index;
            m_selectedRows << index;
            m_sourceSelectedRows << mapFromSelectionToSource(index);
        }
    }

    kDebug() << "m_selectedRows" << m_selectedRows;
    kDebug() << "m_sourceSelectedRows" << m_sourceSelectedRows;
    if (!m_selectedRows.isEmpty())
        invalidate();
}

QModelIndex SelectionProxyModel::mapFromSelectionToSource(const QModelIndex &index) const
{
    QModelIndex result = index;
    Q_ASSERT(result.model()==m_selectionModel->model());

    foreach (QAbstractProxyModel *proxy, m_selectionChain) {
        result = proxy->mapToSource(result);
    }

    foreach (QAbstractProxyModel *proxy, m_sourceChain) {
        result = proxy->mapFromSource(result);
    }

    Q_ASSERT(result.model()==sourceModel());
    return result;
}

QList<QAbstractItemModel*> SelectionProxyModel::buildModelStack(QAbstractItemModel *topModel) const
{
    QList<QAbstractItemModel*> result;

    QAbstractItemModel *currentModel = topModel;
    result << currentModel;

    while (QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel*>(currentModel)) {
        currentModel = proxy->sourceModel();
        result << currentModel;
    }

    return result;
}

QAbstractItemModel *SelectionProxyModel::findCommonModel(const QList<QAbstractItemModel*> &leftStack,
                                                         const QList<QAbstractItemModel*> &rightStack) const
{
    foreach (QAbstractItemModel *model, leftStack) {
        if (rightStack.contains(model)) {
            return model;
        }
    }

    return 0;
}

QList<QAbstractProxyModel*> SelectionProxyModel::createProxyChain(const QList<QAbstractItemModel*> &modelStack,
                                                                  QAbstractItemModel *commonModel, bool isBackward)
{
    QList<QAbstractProxyModel*> result;

    foreach (QAbstractItemModel *model, modelStack) {
        if (model==commonModel) {
            break;
        }
        QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel*>(model);
        Q_ASSERT(proxy!=0);
        if (isBackward) {
            result.prepend(proxy);
        } else {
            result.append(proxy);
        }
    }

    return result;
}

void SelectionProxyModel::onSourceRemoveRows(const QModelIndex &sourceIndex, int /*begin*/, int /*end*/)
{
    if (m_selectedRows.isEmpty()) {
        m_selectionModel->select(sourceIndex, QItemSelectionModel::Select);
    }
}
