/*
 * SPDX-FileCopyrightText: 2014 Mario Bensi <mbensi@ipsquad.net>
   SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
   * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
   */



#ifndef PRESENTATION_QUERYTREENODE_H
#define PRESENTATION_QUERYTREENODE_H

#include <functional>

#include "domain/task.h"

#include "domain/queryresultinterface.h"

#include "querytreemodelbase.h"

namespace Presentation {

template<typename ItemType, typename AdditionalInfo>
class QueryTreeNode : public QueryTreeNodeBase
{
public:
    typedef Domain::QueryResultInterface<ItemType> ItemQuery;
    typedef typename ItemQuery::Ptr ItemQueryPtr;

    typedef std::function<ItemQueryPtr(const ItemType &)> QueryGenerator;
    typedef std::function<Qt::ItemFlags(const ItemType &)> FlagsFunction;
    typedef std::function<QVariant(const ItemType &, int, const AdditionalInfo &)> DataFunction;
    typedef std::function<bool(const ItemType &, const QVariant &, int)> SetDataFunction;
    typedef std::function<bool(const QMimeData *, Qt::DropAction, const ItemType &)> DropFunction;

    QueryTreeNode(const ItemType &item, QueryTreeNodeBase *parentNode, QueryTreeModelBase *model,
                  const QueryGenerator &queryGenerator,
                  const FlagsFunction &flagsFunction,
                  const DataFunction &dataFunction,
                  const SetDataFunction &setDataFunction)
        : QueryTreeNodeBase(parentNode, model),
          m_item(item),
          m_flagsFunction(flagsFunction),
          m_dataFunction(dataFunction),
          m_setDataFunction(setDataFunction)
    {
        init(model, queryGenerator);
    }

    QueryTreeNode(const ItemType &item, QueryTreeNodeBase *parentNode, QueryTreeModelBase *model,
                  const QueryGenerator &queryGenerator,
                  const FlagsFunction &flagsFunction,
                  const DataFunction &dataFunction,
                  const SetDataFunction &setDataFunction,
                  const DropFunction &dropFunction)
        : QueryTreeNodeBase(parentNode, model),
          m_item(item),
          m_flagsFunction(flagsFunction),
          m_dataFunction(dataFunction),
          m_setDataFunction(setDataFunction),
          m_dropFunction(dropFunction)
    {
        init(model, queryGenerator);
    }

    ItemType item() const { return m_item; }

    Qt::ItemFlags flags() const override { return m_flagsFunction(m_item); }

    QVariant data(int role) const override
    {
        if (role == QueryTreeModelBase::ObjectRole)
            return QVariant::fromValue(m_item);

        return m_dataFunction(m_item, role, m_additionalInfo);
    }

    bool setData(const QVariant &value, int role) override { return m_setDataFunction(m_item, value, role); }

    bool dropMimeData(const QMimeData *data, Qt::DropAction action) override
    {
        if (m_dropFunction)
            return m_dropFunction(data, action, m_item);
        else
            return false;
    }

    bool hasAdditionalInfo() const { return m_additionalInfo; }
    void setAdditionalInfo(const AdditionalInfo &info) { m_additionalInfo = info; }

private:
    void init(QueryTreeModelBase *model, const QueryGenerator &queryGenerator)
    {
        m_children = queryGenerator(m_item);

        if (!m_children)
            return;

        for (auto child : m_children->data()) {
            QueryTreeNodeBase *node = new QueryTreeNode<ItemType, AdditionalInfo>(child, this,
                                                                  model, queryGenerator,
                                                                  m_flagsFunction,
                                                                  m_dataFunction, m_setDataFunction,
                                                                  m_dropFunction);
            appendChild(node);
        }

        m_children->addPreInsertHandler([this](const ItemType &, int index) {
            QModelIndex parentIndex = parent() ? createIndex(row(), 0, this) : QModelIndex();
            beginInsertRows(parentIndex, index, index);
        });
        m_children->addPostInsertHandler([this, model, queryGenerator](const ItemType &item, int index) {
            QueryTreeNodeBase *node = new QueryTreeNode<ItemType, AdditionalInfo>(item, this,
                                                                  model, queryGenerator,
                                                                  m_flagsFunction,
                                                                  m_dataFunction, m_setDataFunction,
                                                                  m_dropFunction);
            insertChild(index, node);
            endInsertRows();
        });
        m_children->addPreRemoveHandler([this](const ItemType &, int index) {
            QModelIndex parentIndex = parent() ? createIndex(row(), 0, this) : QModelIndex();
            beginRemoveRows(parentIndex, index, index);
        });
        m_children->addPostRemoveHandler([this](const ItemType &, int index) {
            removeChildAt(index);
            endRemoveRows();
        });
        m_children->addPostReplaceHandler([this](const ItemType &, int idx) {
            const QModelIndex parentIndex = parent() ? createIndex(row(), 0, this) : QModelIndex();
            const QModelIndex dataIndex = index(idx, 0, parentIndex);
            emitDataChanged(dataIndex, dataIndex);
        });
    }

    ItemType m_item;
    ItemQueryPtr m_children;
    mutable AdditionalInfo m_additionalInfo;

    FlagsFunction m_flagsFunction;
    DataFunction m_dataFunction;
    SetDataFunction m_setDataFunction;
    DropFunction m_dropFunction;
};

}

#endif // PRESENTATION_QUERYTREENODE_H
