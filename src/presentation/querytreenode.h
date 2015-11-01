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


#ifndef PRESENTATION_QUERYTREENODE_H
#define PRESENTATION_QUERYTREENODE_H

#include <functional>

// Qt5 TODO, shouldn't be needed anymore, QVariant will do the right thing
#include "domain/note.h"
#include "domain/task.h"

#include "domain/queryresultinterface.h"

#include "querytreemodelbase.h"

namespace Presentation {

// Qt5 TODO, shouldn't be needed anymore, QVariant will do the right thing
namespace Internal {
    template<typename T>
    QVariant variantFromValue(const T &object)
    {
        return QVariant::fromValue(object);
    }

    template<>
    inline QVariant variantFromValue<Domain::Note::Ptr>(const Domain::Note::Ptr &task)
    {
        return QVariant::fromValue(task.staticCast<Domain::Artifact>());
    }

    template<>
    inline QVariant variantFromValue<Domain::Task::Ptr>(const Domain::Task::Ptr &note)
    {
        return QVariant::fromValue(note.staticCast<Domain::Artifact>());
    }
}

template<typename ItemType>
class QueryTreeNode : public QueryTreeNodeBase
{
public:
    typedef Domain::QueryResultInterface<ItemType> ItemQuery;
    typedef typename ItemQuery::Ptr ItemQueryPtr;

    typedef std::function<ItemQueryPtr(const ItemType &)> QueryGenerator;
    typedef std::function<Qt::ItemFlags(const ItemType &)> FlagsFunction;
    typedef std::function<QVariant(const ItemType &, int)> DataFunction;
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

    Qt::ItemFlags flags() const Q_DECL_OVERRIDE { return m_flagsFunction(m_item); }

    QVariant data(int role) const Q_DECL_OVERRIDE
    {
        if (role == QueryTreeModelBase::ObjectRole)
            return Internal::variantFromValue(m_item);

        return m_dataFunction(m_item, role);
    }

    bool setData(const QVariant &value, int role) Q_DECL_OVERRIDE { return m_setDataFunction(m_item, value, role); }

    bool dropMimeData(const QMimeData *data, Qt::DropAction action) Q_DECL_OVERRIDE
    {
        if (m_dropFunction)
            return m_dropFunction(data, action, m_item);
        else
            return false;
    }

private:
    void init(QueryTreeModelBase *model, const QueryGenerator &queryGenerator)
    {
        m_children = queryGenerator(m_item);

        if (!m_children)
            return;

        for (auto child : m_children->data()) {
            QueryTreeNodeBase *node = new QueryTreeNode<ItemType>(child, this,
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
            QueryTreeNodeBase *node = new QueryTreeNode<ItemType>(item, this,
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
            QModelIndex parentIndex = parent() ? createIndex(row(), 0, this) : QModelIndex();
            emitDataChanged(index(idx, 0, parentIndex), index(idx, 0, parentIndex));
        });
    }

    ItemType m_item;
    ItemQueryPtr m_children;

    FlagsFunction m_flagsFunction;
    DataFunction m_dataFunction;
    SetDataFunction m_setDataFunction;
    DropFunction m_dropFunction;
};

}

#endif // PRESENTATION_QUERYTREENODE_H
