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


#include "incidenceitem.h"

#include <Akonadi/EntityDisplayAttribute>

#include <kcalcore/event.h>
#include <kcalcore/incidence.h>
#include <kcalcore/journal.h>
#include <kcalcore/todo.h>

template<class T>
T unwrap(const Akonadi::Item &item)
{
    return item.hasPayload<T>() ? item.payload<T>() : T();
}

IncidenceItem::IncidenceItem(AbstractPimItem::ItemType type, QObject *parent)
: AbstractPimItem(parent)
{
    KCalCore::Incidence *newItem = 0;
    if (type == AbstractPimItem::Todo) {
        newItem = new KCalCore::Todo();
    } else if (type == AbstractPimItem::Event) {
        newItem = new KCalCore::Event();
    }
    Q_ASSERT(newItem);
    KCalCore::Incidence::Ptr newPtr(newItem);
    m_item.setPayload<KCalCore::Incidence::Ptr>(newPtr);
    m_item.setMimeType(mimeType());
    commitData();
}

IncidenceItem::IncidenceItem(const Akonadi::Item &item, QObject *parent)
:   AbstractPimItem(item, parent)
{
    fetchData();
}

IncidenceItem::IncidenceItem(AbstractPimItem::ItemType type, AbstractPimItem &item, QObject* parent)
:   AbstractPimItem(item, parent)
{
    KCalCore::Incidence *newItem = 0;
    if (type == AbstractPimItem::Todo) {
        newItem = new KCalCore::Todo();
    } else if (type == AbstractPimItem::Event) {
        newItem = new KCalCore::Event();
    }
    Q_ASSERT(newItem);
    KCalCore::Incidence::Ptr newPtr(newItem);
    m_item.setPayload<KCalCore::Incidence::Ptr>(newPtr);
    m_item.setMimeType(mimeType());
    commitData();
}




void IncidenceItem::commitData()
{
    KCalCore::Incidence::Ptr old = unwrap<KCalCore::Incidence::Ptr>(m_item);
    if (!old) {
        kDebug() << "invalid item, cannot commit data";
        return;
    }

    old->setDescription(m_text, m_textIsRich);
    old->setSummary(m_title, m_titleIsRich);
    if (m_creationDate.isValid()) {
        old->setCreated(m_creationDate);
    }

    m_item.setPayload<KCalCore::Incidence::Ptr>(old); //TODO probably not required (shared ptr)
    m_item.setMimeType(mimeType());

    //kDebug() << m_title;
    Akonadi::EntityDisplayAttribute *eda = new Akonadi::EntityDisplayAttribute();
    eda->setIconName(getIconName());
    eda->setDisplayName(m_title);
    m_item.addAttribute(eda);
}

bool IncidenceItem::hasValidPayload()
{
    return m_item.hasPayload<KCalCore::Incidence::Ptr>();
}

void IncidenceItem::fetchData()
{
    if (m_dataFetched) {
        //kDebug() << "payload already fetched";
        return;
    }

    if (!hasValidPayload()) {
        kDebug() << "invalid payload" << m_item.payloadData();
        return;
    }

    KCalCore::Incidence::Ptr inc = m_item.payload<KCalCore::Incidence::Ptr>();
    Q_ASSERT(inc);

    m_uid = inc->uid();
    m_title = inc->summary();
    m_titleIsRich = inc->summaryIsRich();
    m_text = inc->description();
    m_textIsRich = inc->descriptionIsRich();
    m_creationDate = inc->created();
    m_dataFetched = true;
}




QString IncidenceItem::mimeType()
{
    const KCalCore::Incidence::Ptr old = unwrap<KCalCore::Incidence::Ptr>(m_item); //same as hasValidPayload + getting payload
    if (!old) {
        kWarning() << "invalid item";
        return QString();
    }
    return old->mimeType();
}

bool IncidenceItem::hasStartDate() const
{
    if (!m_item.hasPayload()) {
        kWarning() << "no payload";
    }
    if ( const KCalCore::Event::Ptr t = unwrap<KCalCore::Event::Ptr>(m_item) ) {
        return t->dtStart().isValid();
    }
    return false;
}


KDateTime IncidenceItem::getEventStart()
{
    if (!m_item.hasPayload()) {
        kWarning() << "no payload";
        //        fetchPayload(true);
    }
    if ( const KCalCore::Event::Ptr t = unwrap<KCalCore::Event::Ptr>(m_item) ) {
        return t->dtStart();
    }
    kWarning() << "not an event, or no start date";
    return KDateTime();
}

void IncidenceItem::setEventStart(const KDateTime &date)
{
    if ( const KCalCore::Event::Ptr t = unwrap<KCalCore::Event::Ptr>(m_item) ) {
        t->setDtStart(date);
    }
}


void IncidenceItem::setParentTodo(const IncidenceItem &parent)
{
    if ( const KCalCore::Todo::Ptr t = unwrap<KCalCore::Todo::Ptr>(m_item) ) {
        const KCalCore::Todo::Ptr p = unwrap<KCalCore::Todo::Ptr>(parent.getItem());
        t->setRelatedTo(p->uid());
    }
}


void IncidenceItem::setDueDate(const KDateTime &date, bool hasDueDate)
{
    if ( const KCalCore::Todo::Ptr t = unwrap<KCalCore::Todo::Ptr>(m_item) ) {
        t->setDtDue(date);
        t->setHasDueDate(hasDueDate);
    }
}

KDateTime IncidenceItem::getDueDate()
{
    if (!m_item.hasPayload()) {
        kWarning() << "no payload";
        //        fetchPayload(true);
    }
    if ( const KCalCore::Todo::Ptr t = unwrap<KCalCore::Todo::Ptr>(m_item) ) {
        if (t->hasDueDate()) {
            //kDebug() << "due date: " << t->dtDue();
            return t->dtDue();
        }
    }
    kWarning() << "not a todo, or no due date";
    return KDateTime();
}

bool IncidenceItem::hasDueDate() const
{
    if (!m_item.hasPayload()) {
        kWarning() << "no payload";
    }
    if ( const KCalCore::Todo::Ptr t = unwrap<KCalCore::Todo::Ptr>(m_item) ) {
        return t->hasDueDate();
    }
    return false;
}

/*
bool IncidenceItem::isComplete()
{
    if (!m_item.hasPayload()) {
        kDebug() << "no payload";
        //        fetchPayload(true);
    }
    if ( const KCalCore::Todo::Ptr t = unwrap<KCalCore::Todo::Ptr>(m_item) ) {
        return t->isCompleted();
    }
    kWarning() << "not a todo";
    return false;
}

void IncidenceItem::setComplete(bool state)
{
    if (!m_item.hasPayload()) {
        kDebug() << "no payload";
        //        fetchPayload(true);
    }
    if ( const KCalCore::Todo::Ptr t = unwrap<KCalCore::Todo::Ptr>(m_item) ) {
        return t->setCompleted(state);
    }
    kWarning() << "not a todo";
}*/

void IncidenceItem::setTodoStatus(AbstractPimItem::ItemStatus status)
{
    if (!m_item.hasPayload()) {
        kDebug() << "no payload";
        //        fetchPayload(true);
    }
    //kDebug() << status;
    if ( const KCalCore::Todo::Ptr t = unwrap<KCalCore::Todo::Ptr>(m_item) ) {
        switch (status) {
            case Complete:
                t->setCompleted(true);
                break;
            case Later:
                t->setCompleted(false);
                t->setHasStartDate(false);
                break;
            case Now:
                t->setCompleted(false);
                t->setDtStart(KDateTime::currentLocalDateTime());
                break;
            default:
                kDebug() << "tried to set unhandled status: " << status;
        }
        return;
    }
    kWarning() << "not a todo";
}


AbstractPimItem::ItemStatus IncidenceItem::getStatus() const
{
    if (!m_item.hasPayload()) {
        kDebug() << "no payload";
        //        fetchPayload(true);
    }
    if ( const KCalCore::Todo::Ptr t = unwrap<KCalCore::Todo::Ptr>(m_item) ) {
        if (t->isCompleted()) {
            //kDebug() << "iscomplete";
            return Complete;
        }
        if (t->hasStartDate() && (t->dtStart() <= KDateTime::currentLocalDateTime())) {
            //kDebug() << "Now";
            return Now;
        }
        if (t->hasDueDate() && (t->dtDue() <= KDateTime::currentLocalDateTime())) {
            return Attention;
        }
        //kDebug() << "Later";
        return Later;
    }
    if ( const KCalCore::Event::Ptr t = unwrap<KCalCore::Event::Ptr>(m_item) ) {
        if (!t->dtStart().isValid() || t->dtStart() > KDateTime::currentLocalDateTime()) {
            return Later;
        }
        if (t->dtEnd() > KDateTime::currentLocalDateTime()) {
            return Now;
        }
        return Complete;
    }
    kWarning() << "not a todo/event";
    return Later;
}


KDateTime IncidenceItem::getPrimaryDate()
{
    if (!m_item.hasPayload()) {
        kDebug() << "no payload";
//        fetchPayload(true);
    }
    if ( const KCalCore::Todo::Ptr t = unwrap<KCalCore::Todo::Ptr>(m_item) ) {
        if (t->hasDueDate()) {
            //kDebug() << "due date: " << t->dtDue();
            return t->dtDue();
        } else {
            //kDebug() << "mod date: " << modificationTime();
            return getLastModifiedDate();
        }
    } else if ( const KCalCore::Event::Ptr e = unwrap<KCalCore::Event::Ptr>(m_item) ) {
        //if ( !e->recurs() && !e->isMultiDay() ) {
            return e->dtStart();
        //}
    } else if ( const KCalCore::Journal::Ptr j = unwrap<KCalCore::Journal::Ptr>(m_item) ) {
        return j->dtStart();
    }
    kWarning() << "unknown item";
    return KDateTime();
}

QString IncidenceItem::getIconName()
{
    KCalCore::Incidence::Ptr old = unwrap<KCalCore::Incidence::Ptr>(m_item);
    if (!old) {
        kWarning() << "invalid item";
        return QLatin1String( "network-wired" );
    }
    if ( old->type() == KCalCore::IncidenceBase::TypeTodo ) {
        return QLatin1String( "view-pim-tasks" );
    } else if ( old->type() == KCalCore::IncidenceBase::TypeJournal ) {
        return QLatin1String( "view-pim-journal" );
    } else if ( old->type() == KCalCore::IncidenceBase::TypeEvent ) {
        return QLatin1String( "view-calendar" );
    }
    kWarning() << "unknown item";
    return QLatin1String( "network-wired" );
}

AbstractPimItem::ItemType IncidenceItem::itemType()
{
    KCalCore::Incidence::Ptr old = unwrap<KCalCore::Incidence::Ptr>(m_item);
    if (!old) {
        kWarning() << "invalid item";
        return AbstractPimItem::Incidence;
    }
    if ( old->type() == KCalCore::IncidenceBase::TypeTodo ) {
        return AbstractPimItem::Todo;
    } else if ( old->type() == KCalCore::IncidenceBase::TypeJournal ) {
        return AbstractPimItem::Journal;
    } else if ( old->type() == KCalCore::IncidenceBase::TypeEvent ) {
        return AbstractPimItem::Event;
    }
    return AbstractPimItem::Incidence;
}

QList< PimItemRelation > IncidenceItem::getRelations()
{
    KCalCore::Incidence::Ptr i = unwrap<KCalCore::Incidence::Ptr>(m_item);
    if (i->relatedTo().isEmpty()) {
        return QList<PimItemRelation>();
    }
    PimItemRelation rel(PimItemRelation::Project, QList<PimItemTreeNode>() << PimItemTreeNode(i->relatedTo().toUtf8()));
    return QList<PimItemRelation>() << rel;
}


