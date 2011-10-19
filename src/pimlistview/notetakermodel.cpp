/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) <year>  <name of author>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include "notetakermodel.h"

#include <kmime/kmime_message.h>
#include <kmime/kmime_dateformatter.h>

#include <KDateTime>
#include <KIconLoader>
#include <KLocale>
#include <QPixmap>
#include <QBrush>

#include <KDebug>
#include <QItemSelectionModel>

#include "pimitem.h"
#include "incidenceitem.h"
#include "datestringbuilder.h"
#include "tagmanager.h"
#include <globaldefs.h>


NotetakerModel::NotetakerModel(ChangeRecorder* monitor, QObject* parent)
: EntityTreeModel(monitor, parent)
{
    m_itemHeaders = new QStringList();
    *m_itemHeaders << QLatin1String( "Title" ) << QLatin1String( "Date" ) << QLatin1String( "Status" );

    QHash<int, QByteArray> roles = EntityTreeModel::roleNames();
    roles[TitleRole] = "title";
    roles[DateRole] = "date";
    setRoleNames(roles);
}

NotetakerModel::~NotetakerModel()
{
    delete m_itemHeaders;
}

int NotetakerModel::entityColumnCount(EntityTreeModel::HeaderGroup headerGroup) const
{
    Q_UNUSED(headerGroup);
    return ColumnCount;
}

QVariant NotetakerModel::entityData(const Akonadi::Item& item, int column, int role) const
{
    QScopedPointer<AbstractPimItem> pimitem(PimItemUtils::getItem(item));
    if (pimitem.isNull()) {
        return QVariant();
    }
    Q_ASSERT(pimitem);
    switch(role) {
        case Qt::DisplayRole: {
            switch (column) {
                case Title:
                    return pimitem->getTitle();
                case Date:
                    return DateStringBuilder::getShortDate(pimitem->getPrimaryDate());
                case Status:
                    //kDebug() << " type: " << pimitem->itemType();
                    //kDebug() << "status: " <<inc->getTodoStatus();
                    switch (pimitem->getStatus()) {
                        case AbstractPimItem::Now:
                            return QBrush(Qt::green);
                        case AbstractPimItem::Later:
                            return QBrush(Qt::yellow);
                        case AbstractPimItem::Complete:
                            return QBrush(Qt::lightGray);
                        case AbstractPimItem::Attention:
                            return QBrush(Qt::red);
                    }
                    break;
            }
            break;
        }
        case Qt::EditRole:
            return pimitem->getTitle();
        case Qt::ToolTipRole: {
            QString d;
            d.append(QString::fromLatin1("Subject: %1\n").arg(pimitem->getTitle()));
            //kDebug() << pimitem->getCreationDate().dateTime() << pimitem->getLastModifiedDate().dateTime();
            d.append(QString::fromLatin1("Created: %1\n").arg(DateStringBuilder::getFullDateTime(pimitem->getCreationDate())));
            d.append(QString::fromLatin1("Modified: %1\n").arg(DateStringBuilder::getFullDateTime(pimitem->getLastModifiedDate())));
            if (pimitem->itemType()&AbstractPimItem::Todo && static_cast<IncidenceItem*>(pimitem.data())->hasDueDate()) {
                d.append(QString::fromLatin1("Due: %1\n").arg(DateStringBuilder::getFullDateTime(pimitem->getPrimaryDate())));
            }
            d.append(QString::fromLatin1("Akonadi: %1\n").arg(item.url().url()));
            d.append(QString::fromLatin1("Nepomuk Resource: %1\n").arg(PimItemUtils::getResource(item).uri()));
            d.append(QString::fromLatin1("Nepomuk Thing: %1\n").arg(PimItemUtils::getThing(item).uri()));
            d.append(QString::fromLatin1("Akonadi Collection: %1\n").arg(item.parentCollection().id()));
            return d;
        }
        case Qt::DecorationRole: { //only needed because the calendar doesnt set the display attribute properly, so we cant rely on it
            if ( column != Title ) {
                return QVariant();
            }
            return SmallIcon(pimitem->getIconName());
        }
        /*case Qt::BackgroundRole: {
            if (pimitem->itemType() & AbstractPimItem::Todo) {
                IncidenceItem *inc = static_cast<IncidenceItem*>(pimitem);
                if (inc->getTodoStatus() == IncidenceItem::Now) {
                    return QBrush(Qt::green);
                } else if (inc->getTodoStatus() == IncidenceItem::Later) {
                    return QBrush(Qt::yellow);
                } else if (inc->getTodoStatus() == IncidenceItem::Complete) {
                    return QBrush(Qt::lightGray);
                }
            }
            break;
        }*/
        case SortRole: {
            switch( column ) {
                case Title:
                    return pimitem->getTitle();
                case Date:
                    return pimitem->getPrimaryDate().dateTime();
                case Status: {
                        //kDebug() << "status: " <<inc->getTodoStatus();
                    switch (pimitem->getStatus()) {
                        case IncidenceItem::Attention:
                            return 0;
                        case IncidenceItem::Now:
                            return 1;
                        case IncidenceItem::Later:
                            return 2;
                        case IncidenceItem::Complete:
                            return 3;
                    }
                }
                default:
                    return QVariant();
            }
        }
        case TitleRole:
            return pimitem->getTitle();
        case DateRole:
            return pimitem->getPrimaryDate().dateTime().toString("ddd, hh:mm:ss");
        case ItemTypeRole:
            return pimitem->itemType();
        default:
            return QVariant();
    }

    //kWarning() << "Not a message" << item.id() << item.remoteId() << item.mimeType();

    return Akonadi::EntityTreeModel::entityData(item, column, role);

}

QVariant NotetakerModel::entityHeaderData(int section, Qt::Orientation orientation, int role, EntityTreeModel::HeaderGroup headerGroup) const
{
    Q_UNUSED(headerGroup);//since we have only items we don't care about the header group
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            if (section >= m_itemHeaders->size() || section < 0 ) {
                return QVariant();
            }
            //kDebug() << m_itemHeaders->at(section);
            return m_itemHeaders->at(section);
        }
    }
    return Akonadi::EntityTreeModel::entityHeaderData(section, orientation, role, headerGroup);
}


QVariant NotetakerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::SizeHintRole) {
        if (section == 1) { //Weekdays
            return QSize(100, EntityTreeModel::headerData(section, orientation, role).toSize().height());
        }
        if (section == 2) { //Weekdays
            return QSize(60, EntityTreeModel::headerData(section, orientation, role).toSize().height());
        }
    }
    return Akonadi::EntityTreeModel::headerData(section, orientation, role);
}

