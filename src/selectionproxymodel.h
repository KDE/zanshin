/* This file is part of Zanshin Todo.

   Copyright 2008-2010 Kevin Ottens <ervin@kde.org>

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

#ifndef ZANSHIN_SELECTIONPROXYMODEL_H
#define ZANSHIN_SELECTIONPROXYMODEL_H

#include <krecursivefilterproxymodel.h>

class QItemSelectionModel;

class SelectionProxyModel : public KRecursiveFilterProxyModel
{
    Q_OBJECT

public:
    SelectionProxyModel(QObject *parent = 0);
    virtual ~SelectionProxyModel();

    void setSelectionModel(QItemSelectionModel *selectionModel);
    virtual void setSourceModel(QAbstractItemModel *model);

    QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const;

protected:
    virtual bool acceptRow(int sourceRow, const QModelIndex &sourceParent) const;

private slots:
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    QModelIndex mapFromSelectionToSource(const QModelIndex &index) const;
    QList<QAbstractItemModel*> buildModelStack(QAbstractItemModel *topModel) const;
    QAbstractItemModel *findCommonModel(const QList<QAbstractItemModel*> &leftStack,
                                        const QList<QAbstractItemModel*> &rightStack) const;
    QList<QAbstractProxyModel*> createProxyChain(const QList<QAbstractItemModel*> &modelStack,
                                                QAbstractItemModel *commonModel, bool isBackward);
    void initializeSelection();

    QItemSelectionModel *m_selectionModel;

    QList<QAbstractProxyModel*> m_selectionChain;
    QList<QAbstractProxyModel*> m_sourceChain;

    QList<QPersistentModelIndex> m_selectedRows;
    QList<QPersistentModelIndex> m_sourceSelectedRows;
};

#endif

