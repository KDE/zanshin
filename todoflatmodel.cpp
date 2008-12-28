/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

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

#include <KLocale>
#include <KDebug>

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
    if (parent.isValid())
        return 0;
    else
        return TodoFlatModel::LastColumn + 1;
}

int TodoFlatModelImpl::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return ItemModel::rowCount(parent);
}

QVariant TodoFlatModelImpl::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount())
        return QVariant();

    const Akonadi::Item item = itemForIndex(index);

    if (!item.hasPayload<IncidencePtr>())
        return QVariant();

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
            if (role == Qt::EditRole)
                return incidence->categories().join(", ");
            else
                return incidence->categories();
        case TodoFlatModel::ParentRemoteId:
            return incidence->relatedToUid();
        case TodoFlatModel::DueDate: {
            KCal::Todo *todo = dynamic_cast<KCal::Todo*>(incidence.get());
            if (todo)
                return todo->dtDue().toString();
            else
                return QVariant();
        }
        case TodoFlatModel::FlagImportant:
            return QVariant();
        }
        break;
    case Qt::CheckStateRole:
        switch(index.column()) {
        case TodoFlatModel::Summary:
            return Qt::Unchecked;
        default:
            return QVariant();
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
        case TodoFlatModel::ParentRemoteId:
            return i18n("Parent Id");
        case TodoFlatModel::DueDate:
            return i18n("Due Date");
        case TodoFlatModel::FlagImportant:
            return QString();
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
}

Akonadi::ItemModel *TodoFlatModel::itemModel() const
{
    return qobject_cast<Akonadi::ItemModel*>(sourceModel());
}

Qt::ItemFlags TodoFlatModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= rowCount())
        return Qt::NoItemFlags;

    Qt::ItemFlags f = Qt::ItemIsEnabled|Qt::ItemIsSelectable;

    switch(index.column()) {
    case TodoFlatModel::Summary:
        f |= Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
        break;
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
    if (!index.isValid() || index.row() >= rowCount())
        return false;

    const Akonadi::Item item = itemForIndex(index);

    if (!item.hasPayload<IncidencePtr>())
        return false;

    const IncidencePtr incidence = item.payload<IncidencePtr>();
    KCal::Todo *todo = dynamic_cast<KCal::Todo*>(incidence.get());

    if (!todo)
        return false;

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
            QSet<QString> ids;
            for (int i=0; i<rowCount(); i++)
                ids << data(this->index(i, TodoFlatModel::RemoteId)).toString();

            if (ids.contains(value.toString())) {
                todo->setRelatedToUid(value.toString());
                return modifyItemHelper(item);
            } else
                return false;
        }
        case TodoFlatModel::DueDate: {
            QDate date = QDate::fromString(value.toString(), Qt::ISODate);
            if (date.isValid()) {
                todo->setDtDue(KDateTime(date));
                todo->setHasDueDate(true);
                todo->setAllDay(true);
                return modifyItemHelper(item);
            } else
                return false;
        }
        default:
            break;
        }
        break;

    case Qt::CheckStateRole:
        switch(index.column()) {
        case TodoFlatModel::Summary:
            return Qt::Unchecked;
        default:
            break;
        }
        break;
    }

    return false;
}

