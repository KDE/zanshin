/*
 * SPDX-FileCopyrightText: 2014 Mario Bensi <mbensi@ipsquad.net>
   SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
   * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
   */



#ifndef PRESENTATION_QUERYTREEMODEL_H
#define PRESENTATION_QUERYTREEMODEL_H

#include "querytreenode.h"

#include <functional>
#include <algorithm>

namespace Presentation {

template<typename ItemType, typename AdditionalInfo = int>
class QueryTreeModel : public QueryTreeModelBase
{
public:
    typedef typename QueryTreeNode<ItemType, AdditionalInfo>::QueryGenerator QueryGenerator;
    typedef typename QueryTreeNode<ItemType, AdditionalInfo>::FlagsFunction FlagsFunction;
    typedef typename QueryTreeNode<ItemType, AdditionalInfo>::DataFunction DataFunction;
    typedef typename QueryTreeNode<ItemType, AdditionalInfo>::SetDataFunction SetDataFunction;
    typedef typename QueryTreeNode<ItemType, AdditionalInfo>::DropFunction DropFunction;
    typedef std::function<QMimeData*(const QList<ItemType> &)> DragFunction;
    using FetchAdditionalInfoFunction = std::function<AdditionalInfo(const QModelIndex &index, ItemType)>;
    using NodeType = QueryTreeNode<ItemType, AdditionalInfo>;

    explicit QueryTreeModel(const QueryGenerator &queryGenerator,
                            const FlagsFunction &flagsFunction,
                            const DataFunction &dataFunction,
                            const SetDataFunction &setDataFunction,
                            QObject *parent = nullptr)
        : QueryTreeModelBase(new QueryTreeNode<ItemType, AdditionalInfo>(ItemType(), nullptr, this,
                                                         queryGenerator, flagsFunction,
                                                         dataFunction, setDataFunction),
                             parent)
    {
    }

    explicit QueryTreeModel(const QueryGenerator &queryGenerator,
                            const FlagsFunction &flagsFunction,
                            const DataFunction &dataFunction,
                            const SetDataFunction &setDataFunction,
                            const DropFunction &dropFunction,
                            const DragFunction &dragFunction,
                            const FetchAdditionalInfoFunction &fetchAdditionalInfoFunction,
                            QObject *parent = nullptr)
        : QueryTreeModelBase(new QueryTreeNode<ItemType, AdditionalInfo>(ItemType(), nullptr, this,
                                                         queryGenerator, flagsFunction,
                                                         dataFunction, setDataFunction,
                                                         dropFunction),
                             parent),
          m_dragFunction(dragFunction),
          m_fetchAdditionalInfoFunction(fetchAdditionalInfoFunction)
    {
    }

protected:
    QMimeData *createMimeData(const QModelIndexList &indexes) const override
    {
        if (m_dragFunction) {
            QList<ItemType> items;
            std::transform(indexes.begin(), indexes.end(),
                           std::back_inserter(items),
                           [this](const QModelIndex &index) {
                               return itemAtIndex(index);
                           });
            return m_dragFunction(items);
        } else {
            return nullptr;
        }
    }


    void fetchAdditionalInfo(const QModelIndex &index) override
    {
        if (m_fetchAdditionalInfoFunction) {
            auto theNode = node(index);
            if (!theNode->hasAdditionalInfo())
                theNode->setAdditionalInfo(m_fetchAdditionalInfoFunction(index, theNode->item()));
        }
    }

    ItemType itemAtIndex(const QModelIndex &index) const
    {
        return node(index)->item();
    }

    NodeType *node(const QModelIndex &index) const
    {
        return static_cast<NodeType *>(nodeFromIndex(index));
    }

private:
    DragFunction m_dragFunction;
    FetchAdditionalInfoFunction m_fetchAdditionalInfoFunction;
};

}

#endif // PRESENTATION_QUERYTREEMODEL_H
