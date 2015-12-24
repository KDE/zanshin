/* This file is part of Zanshin

   Copyright 2014 Mario Bensi <mbensi@ipsquad.net>
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


#ifndef PRESENTATION_QUERYTREEMODELBASE_H
#define PRESENTATION_QUERYTREEMODELBASE_H

#include <QAbstractItemModel>

namespace Presentation {

class QueryTreeModelBase;

class QueryTreeNodeBase
{
public:
    QueryTreeNodeBase(QueryTreeNodeBase *parent, QueryTreeModelBase *model);
    virtual ~QueryTreeNodeBase();

    virtual Qt::ItemFlags flags() const = 0;
    virtual QVariant data(int role) const = 0;
    virtual bool setData(const QVariant &value, int role) = 0;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action) = 0;

    int row();
    QueryTreeNodeBase *parent() const;
    QueryTreeNodeBase *child(int row) const;
    void insertChild(int row, QueryTreeNodeBase *node);
    void appendChild(QueryTreeNodeBase *node);
    void removeChildAt(int row);
    int childCount() const;

protected:
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex createIndex(int row, int column, void *data) const;
    void beginInsertRows(const QModelIndex &parent, int first, int last);
    void endInsertRows();
    void beginRemoveRows(const QModelIndex &parent, int first, int last);
    void endRemoveRows();
    void emitDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private:
    QueryTreeNodeBase *m_parent;
    QList<QueryTreeNodeBase*> m_childNode;
    QueryTreeModelBase *m_model;
};

class QueryTreeModelBase : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum {
        ObjectRole = Qt::UserRole + 1,
        IconNameRole,
        IsDefaultRole,
        UserRole
    };

    ~QueryTreeModelBase();

    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) Q_DECL_OVERRIDE;
    QMimeData *mimeData(const QModelIndexList &indexes) const Q_DECL_OVERRIDE;
    QStringList mimeTypes() const Q_DECL_OVERRIDE;
    Qt::DropActions supportedDropActions() const Q_DECL_OVERRIDE;

    // TODO Qt5: Remove but needed in Qt4, so that we can trigger it from the outside
    using QAbstractItemModel::dataChanged;

protected:
    explicit QueryTreeModelBase(QueryTreeNodeBase *rootNode,
                                QObject *parent = Q_NULLPTR);
    virtual QMimeData *createMimeData(const QModelIndexList &indexes) const = 0;
    QueryTreeNodeBase *nodeFromIndex(const QModelIndex &index) const;
    void setRootIndexFlag(const Qt::ItemFlags &flags);

private:
    friend class QueryTreeNodeBase;
    bool isModelIndexValid(const QModelIndex &index) const;
    Qt::ItemFlags m_rootIndexFlag;
    QueryTreeNodeBase *m_rootNode;
};

}

#endif // PRESENTATION_QUERYTREEMODELBASE_H
