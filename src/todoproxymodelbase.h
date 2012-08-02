/* This file is part of Zanshin Todo.

   Copyright 2008-2010 Kevin Ottens <ervin@kde.org>
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

#ifndef ZANSHIN_TODOPROXYMODELBASE_H
#define ZANSHIN_TODOPROXYMODELBASE_H

#include <QtGui/QAbstractProxyModel>
#include <akonadi/item.h>

class TodoNode;
class TodoNodeManager;

class TodoProxyModelBase : public QAbstractProxyModel
{
    Q_OBJECT

public:
    enum MappingType {
        SimpleMapping,
        MultiMapping
    };

    TodoProxyModelBase(MappingType type, QObject *parent = 0);
    virtual ~TodoProxyModelBase();

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
    virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;
    QList<QModelIndex> mapFromSourceAll(const QModelIndex &sourceIndex) const;
    virtual QModelIndex buddy(const QModelIndex &index) const;

    virtual void setSourceModel(QAbstractItemModel *model);

    static QModelIndexList modelIndexesForItem( const QAbstractItemModel *model, const Akonadi::Item &item );

private slots:
    virtual void onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end) = 0;
    virtual void onSourceInsertRows(const QModelIndex &sourceIndex, int begin, int end) = 0;
    virtual void onSourceRemoveRows(const QModelIndex &sourceIndex, int begin, int end) = 0;
    virtual void onRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow);
    virtual void onRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row);

protected slots:
    virtual void onModelReset();

protected:
    virtual void init() = 0;
    virtual void resetInternalData();

    TodoNode *addChildNode(const QModelIndex &sourceIndex, TodoNode *parent);

    friend class TodoNodeManager;
    TodoNodeManager *m_manager;

private:
    MappingType m_mappingType;
};

#endif

