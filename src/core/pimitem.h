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

#include <KDateTime>
#include <QStringList>
#include <kcalcore/attachment.h>
#include "pimitemrelations.h"
#include "pimitemindex.h"

class KJob;

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
    virtual ~PimItem();

    virtual PimItemIndex::ItemType itemType() const = 0;

    virtual QString mimeType() const = 0 ;
    static QString mimeType(PimItemIndex::ItemType);
    
    virtual bool hasValidPayload() const = 0;

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
    virtual ItemStatus status() const = 0;

    virtual QString uid() const = 0;
    virtual void setText(const QString &, bool isRich = false) = 0;
    virtual QString text() const = 0;
    virtual void setTitle(const QString &, bool isRich = false) = 0;
    virtual QString title() const = 0;
    virtual void setCreationDate(const KDateTime &) = 0;
    virtual KDateTime creationDate() const = 0;
    virtual KDateTime lastModifiedDate() const = 0;
    virtual QString iconName() const = 0;
    virtual bool isTextRich() const;
    virtual bool isTitleRich() const;
    /**
     * Note: last modified
     * Todo: todo due date
     * Event: start date
     */
    virtual KDateTime primaryDate() const = 0;

    virtual void setRelations(const QList<PimItemRelation> &) = 0;
    virtual QList<PimItemRelation> relations() const = 0;
    virtual void setContexts(const QStringList &);
    virtual QStringList contexts() const;
    virtual const KCalCore::Attachment::List attachments() const;

    virtual KJob *saveItem() = 0;

private:
    Q_DISABLE_COPY(PimItem)
};
//Q_DECLARE_OPERATORS_FOR_FLAGS(PimItem::ItemTypes)
//Q_DECLARE_METATYPE(PimItem::ItemTypes)
Q_DECLARE_METATYPE(PimItem::Ptr)

#endif // ABSTRACTPIMITEM_H
