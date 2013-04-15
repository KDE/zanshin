/* This file is part of Zanshin Todo.

   Copyright 2011 Christian Mollekopf <chrigi_1@fastmail.fm>

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


#ifndef ABSTRACTPIMITEM_H
#define ABSTRACTPIMITEM_H

#include <Akonadi/Item>
#include <KDateTime>
#include <QStringList>
#include <kcalcore/attachment.h>
#include "pimitemrelations.h"

class KJob;

namespace Akonadi {
    class Monitor;
    class Session;
}

/**
 * A wrapper around akonadi item
 *
 * There are two subclasses for Notes and Incidences
 *
 * This class should allow to deal with notes and various incidences in a uniform way in a lot of cases.
 */
class PimItem
{
public:
    typedef QSharedPointer<PimItem> Ptr;
    PimItem();
    PimItem(const Akonadi::Item &);
    virtual ~PimItem();

    enum ItemType {
        None = 0,
        Unknown = 1,
        Note = 2,
        //Calendar Items
        Event = 4,
        Todo = 8,
        Journal = 16,
        Incidence = Event | Todo | Journal, //All calendar Items
        All = Note | Event | Todo | Journal
    };
    Q_DECLARE_FLAGS(ItemTypes, ItemType)

    //based on item mimetype of item
    static ItemType itemType(const Akonadi::Item &);
    virtual ItemType itemType() = 0;

    virtual QString mimeType() = 0 ;
    static QString mimeType(ItemType);
    //Returns a list of all supported mimetypes
    static QStringList mimeTypes();
    
    virtual bool hasValidPayload() = 0;

    enum ItemStatus {
        Complete = 1,
        NotComplete,
        Now,
        Later,
        Attention
    };

    /**
     * Note: Always Later
     * Todo: set by user (priority)
     * Event: In Future/ Today/ Passed
     */
    virtual ItemStatus getStatus() const = 0;

    virtual QString getUid() = 0;
    virtual void setText(const QString &, bool isRich = false) = 0;
    virtual QString getText() = 0;
    virtual void setTitle(const QString &, bool isRich = false) = 0;
    virtual QString getTitle() = 0;
    virtual void setCreationDate(const KDateTime &) = 0;
    virtual KDateTime getCreationDate() = 0;
    virtual KDateTime getLastModifiedDate();
    virtual QString getIconName() = 0;
    virtual bool textIsRich();
    virtual bool titleIsRich();
    /**
     * Note: last modified
     * Todo: todo due date
     * Event: start date
     */
    virtual KDateTime getPrimaryDate() = 0;

    virtual void setRelations(const QList<PimItemRelation> &) = 0;
    virtual QList<PimItemRelation> getRelations() = 0;
    virtual void setCategories(const QStringList &);
    virtual QStringList getCategories();
    virtual const KCalCore::Attachment::List getAttachments();

    const Akonadi::Item &getItem() const;
    KJob *saveItem();

protected:
    Akonadi::Item m_item;
    virtual void setItem(const Akonadi::Item &);
private:
    friend class PimItemMonitor;
    Q_DISABLE_COPY(PimItem);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(PimItem::ItemTypes)
Q_DECLARE_METATYPE(PimItem::ItemTypes)
Q_DECLARE_METATYPE(PimItem::Ptr)

#endif // ABSTRACTPIMITEM_H
