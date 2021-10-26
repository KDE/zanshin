/*
 * SPDX-FileCopyrightText: 2014 Mario Bensi <mbensi@ipsquad.net>
   SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
   * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
        IsChildRole,
        ProjectRole,
        DataSourceRole,
        ContextListRole,
        UserRole
    };

    ~QueryTreeModelBase();

    QHash<int, QByteArray> roleNames() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    QStringList mimeTypes() const override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;

protected:
    explicit QueryTreeModelBase(QueryTreeNodeBase *rootNode,
                                QObject *parent = nullptr);
    virtual QMimeData *createMimeData(const QModelIndexList &indexes) const = 0;
    virtual void fetchAdditionalInfo(const QModelIndex &) {}
    QueryTreeNodeBase *nodeFromIndex(const QModelIndex &index) const;
    void setRootIndexFlag(Qt::ItemFlags flags);

private:
    friend class QueryTreeNodeBase;
    bool isModelIndexValid(const QModelIndex &index) const;
    Qt::ItemFlags m_rootIndexFlag;
    QueryTreeNodeBase *m_rootNode;
};

}

#endif // PRESENTATION_QUERYTREEMODELBASE_H
