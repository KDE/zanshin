/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>
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

#include "todoflatmodel.h"

#include <akonadi/collection.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemmodel.h>
#include <akonadi/itemmodifyjob.h>

#include <boost/shared_ptr.hpp>

#include <kcal/todo.h>

#include <KIcon>
#include <KLocale>
#include <KDebug>

#include <qmimedata.h>

typedef boost::shared_ptr<KCal::Incidence> IncidencePtr;

class TodoFlatModelImpl : public Akonadi::ItemModel
{
public:
    TodoFlatModelImpl(QObject *parent = 0);
    ~TodoFlatModelImpl();

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
};

TodoFlatModelImpl::TodoFlatModelImpl(QObject *parent)
    : Akonadi::ItemModel(parent)
{
}

TodoFlatModelImpl::~TodoFlatModelImpl()
{
}

int TodoFlatModelImpl::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return TodoFlatModel::LastColumn + 1;
    }
}

int TodoFlatModelImpl::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return ItemModel::rowCount(parent);
    }
}

QVariant TodoFlatModelImpl::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    const Akonadi::Item item = itemForIndex(index);

    if (!item.hasPayload<IncidencePtr>()) {
        return QVariant();
    }

    const IncidencePtr incidence = item.payload<IncidencePtr>();

    switch( role ) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch(index.column()) {
        case TodoFlatModel::RemoteId:
            return incidence->uid();
        case TodoFlatModel::Summary:
            return incidence->summary();
        case TodoFlatModel::Categories:
            if (role == Qt::EditRole) {
                return incidence->categories().join(", ");
            } else {
                return incidence->categories();
            }
        case TodoFlatModel::ParentRemoteId:
            return incidence->relatedToUid();
        case TodoFlatModel::DueDate: {
            KCal::Todo *todo = dynamic_cast<KCal::Todo*>(incidence.get());
            if (todo) {
                return todo->dtDue().toString();
            } else {
                return QString();
            }
        }
        }
        break;
    }

    return QVariant();
}

QVariant TodoFlatModelImpl::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch(section) {
        case TodoFlatModel::RemoteId:
            return i18n("Remote Id");
        case TodoFlatModel::Summary:
            return i18n("Summary");
        case TodoFlatModel::Categories:
            return i18n("Categories");
        case TodoFlatModel::ParentSummary:
            return i18n("Parent Summary");
        case TodoFlatModel::ParentRemoteId:
            return i18n("Parent Id");
        case TodoFlatModel::DueDate:
            return i18n("Due Date");
        case TodoFlatModel::RowType:
            return i18n("Row Type");
        }
    }

    return QVariant();
}



TodoFlatModel::TodoFlatModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    Akonadi::ItemModel *itemModel = new TodoFlatModelImpl(this);
    itemModel->fetchScope().fetchFullPayload();
    setSourceModel(itemModel);
}

TodoFlatModel::~TodoFlatModel()
{

}

Akonadi::Item TodoFlatModel::itemForIndex(const QModelIndex &index) const
{
    return itemModel()->itemForIndex(mapToSource(index));
}

QModelIndex TodoFlatModel::indexForItem(const Akonadi::Item &item, const int column) const
{
    return mapFromSource(itemModel()->indexForItem(item, column));
}

void TodoFlatModel::setCollection(const Akonadi::Collection &collection)
{
    m_reverseRemoteIdMap.clear();
    m_remoteIdMap.clear();
    m_parentMap.clear();
    m_childrenMap.clear();
    itemModel()->setCollection(collection);
}

Akonadi::Collection TodoFlatModel::collection() const
{
    return itemModel()->collection();
}

bool TodoFlatModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Akonadi::ItemModel *source = itemModel();
    Q_ASSERT(source);

    const Akonadi::Item item = source->itemForIndex(source->index(sourceRow, 0, sourceParent));

    return item.mimeType()=="application/x-vnd.akonadi.calendar.todo";
}

void TodoFlatModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    Q_ASSERT(sourceModel == 0 || qobject_cast<Akonadi::ItemModel*>(sourceModel) != 0);
    QSortFilterProxyModel::setSourceModel(sourceModel);

    connect(itemModel(), SIGNAL(collectionChanged(const Akonadi::Collection&)),
            this, SIGNAL(collectionChanged(const Akonadi::Collection&)));
    connect(itemModel(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(onSourceInsertRows(const QModelIndex&, int, int)));
    connect(itemModel(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
            this, SLOT(onSourceRemoveRows(const QModelIndex&, int, int)));
    connect(itemModel(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(onSourceDataChanged(const QModelIndex&, const QModelIndex&)));
}

Akonadi::ItemModel *TodoFlatModel::itemModel() const
{
    return qobject_cast<Akonadi::ItemModel*>(sourceModel());
}

QStringList TodoFlatModel::mimeTypes() const
{
    QStringList list;
    list << "text/uri-list";
    return list;
}

Qt::DropActions TodoFlatModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

Qt::ItemFlags TodoFlatModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags f = Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsDragEnabled|Qt::ItemIsDropEnabled;

    switch(index.column()) {
    case TodoFlatModel::Summary: {
        f |= Qt::ItemIsEditable;

        const Akonadi::Item item = itemForIndex(index);

        if (!item.hasPayload<IncidencePtr>()) {
            break;
        }

        const IncidencePtr incidence = item.payload<IncidencePtr>();
        ItemType type = todoType(incidence->uid());

        if (type==StandardTodo) {
            f |= Qt::ItemIsUserCheckable;
        }
        break;
    }
    case TodoFlatModel::Categories:
    case TodoFlatModel::ParentRemoteId:
    case TodoFlatModel::DueDate:
        f |= Qt::ItemIsEditable;
        break;
    default:
        break;
    }

    return f;
}

QVariant TodoFlatModel::data(const QModelIndex &index, int role) const
{
    if (role==Qt::DecorationRole && index.column()==Summary) {
        const Akonadi::Item item = itemForIndex(index);

        if (!item.hasPayload<IncidencePtr>()) {
            return QVariant();
        }

        const IncidencePtr incidence = item.payload<IncidencePtr>();
        ItemType type = todoType(incidence->uid());
        if (type==ProjectTodo) {
            return KIcon("view-pim-tasks");
        } else if (type==FolderTodo) {
            return KIcon("folder");
        } else {
            return QVariant();
        }
    } else if (role==Qt::CheckStateRole && index.column()==Summary) {
        const Akonadi::Item item = itemForIndex(index);

        if (!item.hasPayload<IncidencePtr>()) {
            return QVariant();
        }

        const IncidencePtr incidence = item.payload<IncidencePtr>();
        ItemType type = todoType(incidence->uid());

        if (type==StandardTodo) {
            KCal::Todo *todo = dynamic_cast<KCal::Todo*>(incidence.get());
            return (todo->isCompleted() ? Qt::Checked : Qt::Unchecked);
        }
    } else if (index.column()==ParentSummary) {
        if (role!=Qt::DisplayRole) {
            return QString();
        }

        const Akonadi::Item item = itemForIndex(index);

        if (!item.hasPayload<IncidencePtr>()) {
            return QVariant();
        }

        const IncidencePtr incidence = item.payload<IncidencePtr>();

        Akonadi::Item parentItem(m_reverseRemoteIdMap[incidence->relatedToUid()]);
        QModelIndex parent = indexForItem(parentItem, TodoFlatModel::Summary);
        return data(parent, role);
    } else if (index.column()==RowType) {
        if (role!=Qt::DisplayRole) {
            return QString();
        }

        const Akonadi::Item item = itemForIndex(index);

        if (!item.hasPayload<IncidencePtr>()) {
            return QVariant();
        }

        const IncidencePtr incidence = item.payload<IncidencePtr>();
        return todoType(incidence->uid());
    }

    return QSortFilterProxyModel::data(index, role);
}

static bool modifyItemHelper(const Akonadi::Item &item)
{
    Akonadi::ItemModifyJob *modifyJob = new Akonadi::ItemModifyJob(item);
    modifyJob->disableRevisionCheck(); //FIXME: I don't get why this one is needed...
    bool result = modifyJob->exec();

    if (!result) {
        kWarning() << "Couldn't apply modification to item" << item.id()
                   << "we got error:" << modifyJob->error() << modifyJob->errorString();
    }

    return result;
}

bool TodoFlatModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return false;
    }

    const Akonadi::Item item = itemForIndex(index);

    if (!item.hasPayload<IncidencePtr>()) {
        return false;
    }

    const IncidencePtr incidence = item.payload<IncidencePtr>();
    KCal::Todo *todo = dynamic_cast<KCal::Todo*>(incidence.get());

    if (!todo) {
        return false;
    }

    switch( role ) {
    case Qt::EditRole:
        switch(index.column()) {
        case TodoFlatModel::Summary:
            todo->setSummary(value.toString());
            return modifyItemHelper(item);

        case TodoFlatModel::Categories: {
            QStringList categories = value.toString().split(QRegExp("\\s*,\\s*"));
            todo->setCategories(categories);
            return modifyItemHelper(item);
        }
        case TodoFlatModel::ParentRemoteId: {
            // test existence and cycle
            TodoFlatModel::ItemType itemType = todoType(incidence->uid());
            TodoFlatModel::ItemType parentType = StandardTodo;
            if (!value.toString().isEmpty()) {
                parentType = todoType(value.toString());
            }
            if (!m_reverseRemoteIdMap.contains(value.toString())
             || isAncestorOf(incidence->uid(), value.toString())
             || (itemType == ProjectTodo && parentType == ProjectTodo)
             || (itemType == ProjectTodo && parentType == StandardTodo)
             || (itemType == FolderTodo && parentType == ProjectTodo)
             || (itemType == FolderTodo && parentType == StandardTodo)) {
                return false;
            }
            todo->setRelatedToUid(value.toString());

            if (itemType == StandardTodo
             && parentType == FolderTodo
             && !todo->comments().contains("X-Zanshin-Project")) {
                    todo->addComment("X-Zanshin-Project");
            }
            if (itemType == StandardTodo
             && value.toString().isEmpty()) {
                todo->addComment("X-Zanshin-Project");
            }

            return modifyItemHelper(item);
        }
        case TodoFlatModel::DueDate: {
            QDate date = QDate::fromString(value.toString(), Qt::ISODate);
            if (date.isValid()) {
                todo->setDtDue(KDateTime(date));
                todo->setHasDueDate(true);
                todo->setAllDay(true);
                return modifyItemHelper(item);
            } else {
                if (value.toString().isEmpty()) {
                    todo->setDtDue(KDateTime());
                    todo->setHasDueDate(false);
                    todo->setAllDay(false);
                    return modifyItemHelper(item);
                }
                return false;
            }
        }
        default:
            break;
        }
        break;

    case Qt::CheckStateRole:
        switch(index.column()) {
        case TodoFlatModel::Summary: {
            bool completed = false;
            if (value.toInt()==Qt::Checked) {
                completed = true;
            }
            if (completed!=todo->isCompleted()) {
                todo->setCompleted(completed);
                return modifyItemHelper(item);
            }
        }
        default:
            break;
        }
        break;
    }

    return false;
}

void TodoFlatModel::onSourceInsertRows(const QModelIndex&/*sourceIndex*/, int begin, int end)
{
    for (int i = begin; i <= end; i++) {
        // Retrieve the item from the source model
        Akonadi::Item item = itemModel()->itemForIndex(itemModel()->index(i, 0));
        QString remoteId = itemModel()->data(itemModel()->index(i, TodoFlatModel::RemoteId)).toString();
        m_remoteIdMap[item.id()] = remoteId;
        m_reverseRemoteIdMap[remoteId] = item.id();
    }

    for (int i = begin; i <= end; i++) {
        Akonadi::Item item = itemModel()->itemForIndex(itemModel()->index(i, 0));

        QString remoteId = m_remoteIdMap[item.id()];
        QString parentRemoteId = itemModel()->data(itemModel()->index(i, TodoFlatModel::ParentRemoteId)).toString();

        bool shouldEmit = !m_childrenMap.contains(parentRemoteId);

        m_parentMap[remoteId] = parentRemoteId;
        m_childrenMap[parentRemoteId] << remoteId;

        if (shouldEmit) {
            Akonadi::Item item(m_reverseRemoteIdMap[parentRemoteId]);
            QModelIndex start = indexForItem(item, 0);
            QModelIndex end = indexForItem(item, LastColumn);
            emit dataChanged(start, end);
        }
    }

}

void TodoFlatModel::onSourceRemoveRows(const QModelIndex&/*sourceIndex*/, int begin, int end)
{
    for (int i = begin; i <= end; ++i) {
        QModelIndex sourceIndex = itemModel()->index(i, 0);
        Akonadi::Item item = itemModel()->itemForIndex(sourceIndex);

        QString remoteId = m_remoteIdMap.take(item.id());
        QString parentRemoteId = itemModel()->data(itemModel()->index(i, TodoFlatModel::ParentRemoteId)).toString();

        m_parentMap.remove(remoteId);
        m_childrenMap[parentRemoteId].removeAll(remoteId);
        m_reverseRemoteIdMap.remove(remoteId);
    }
}

void TodoFlatModel::onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end)
{
    for (int row = begin.row(); row <= end.row(); ++row) {
        QModelIndex remoteIdIndex = itemModel()->index(row, TodoFlatModel::RemoteId);
        QString remoteId = itemModel()->data(remoteIdIndex).toString();

        QModelIndex parentRemoteIdIndex = itemModel()->index(row, TodoFlatModel::ParentRemoteId);
        QString newParentRemoteId = itemModel()->data(parentRemoteIdIndex).toString();

        QString oldParentRemoteId = m_parentMap[remoteId];

        if (newParentRemoteId!=oldParentRemoteId) {
            m_parentMap[remoteId] = newParentRemoteId;
            m_childrenMap[newParentRemoteId] << remoteId;
            m_childrenMap[oldParentRemoteId].removeAll(remoteId);
        }
    }
}

bool TodoFlatModel::isAncestorOf(const QString &ancestor, const QString &child)
{
    QString parent = m_parentMap[child];
    if (parent.isEmpty())
        return false;
    if (parent == ancestor)
        return true;
    return isAncestorOf(ancestor, parent);
}

TodoFlatModel::ItemType TodoFlatModel::todoType(const QString &remoteId, bool examinateSiblings) const
{
    const QStringList children = m_childrenMap[remoteId];
    if (children.size()>0) {
        foreach (const QString &child, children) {
            if (todoType(child) == ProjectTodo) {
                return FolderTodo;
            }
        }

        return ProjectTodo;
    }

    if (m_reverseRemoteIdMap.contains(remoteId)) {
        const Akonadi::Item it(m_reverseRemoteIdMap[remoteId]);
        QModelIndex index = indexForItem(it, 0);
        if (index != QModelIndex()) {
            const Akonadi::Item item = itemForIndex(index);
            const IncidencePtr incidence = item.payload<IncidencePtr>();
            KCal::Todo *todo = dynamic_cast<KCal::Todo*>(incidence.get());
            QStringList comments = todo->comments();
            if (comments.contains("X-Zanshin-Project"))
                return ProjectTodo;
            else if (comments.contains("X-Zanshin-Folder"))
                return FolderTodo;
        }
    }

    //FIXME: That's definitely expensive, optimize this one
    if (examinateSiblings && !m_parentMap[remoteId].isEmpty()) {
        const QStringList siblings = m_childrenMap[m_parentMap[remoteId]];
        foreach (const QString &sibling, siblings) {
            if (sibling == remoteId) continue;

            if (todoType(sibling, false) != StandardTodo) {
                return ProjectTodo;
            }
        }
    }

    return StandardTodo;
}

