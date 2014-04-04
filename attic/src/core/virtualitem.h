/* This file is part of Zanshin Todo.

   Copyright 2013 Kevin Ottens <ervin@kde.org>

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

#ifndef VIRTUALITEM_H
#define VIRTUALITEM_H

#include "pimitem.h"

class VirtualItem : public PimItem
{
public:
    typedef QSharedPointer<VirtualItem> Ptr;

    VirtualItem();
    VirtualItem(ItemType type, const QString &title = QString());

    qint64 relationId() const;
    void setRelationId(qint64 id);

    virtual ItemType itemType() const;

    virtual QString mimeType() const;
    virtual ItemStatus status() const;

    virtual QString uid() const;

    virtual QString iconName() const;

    virtual void setText(const QString &, bool = false);
    virtual QString text() const;
    virtual void setTitle(const QString &, bool = false);
    virtual QString title() const;

    virtual KDateTime date(DateRole) const;
    virtual bool setDate(DateRole, const KDateTime &);
    virtual void setRelations(const QList<PimItemRelation> &);
    virtual QList<PimItemRelation> relations() const;

    virtual KJob *saveItem();

private:
    ItemType m_type;
    QString m_title;
    qint64 m_relationId;
};

#endif
