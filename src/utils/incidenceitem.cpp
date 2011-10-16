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


#include "incidenceitem.h"

#include <Akonadi/EntityDisplayAttribute>
#include <kcalcore/incidence.h>

#include "../calendarsupport/utils.h"

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
    KCalCore::Incidence::Ptr old = CalendarSupport::incidence(m_item);
    if (!old) {
        kDebug() << "invalid item, cannot commit data";
        return;
    }

    old->setDescription(m_text, m_textIsRich);
    old->setSummary(m_title, m_titleIsRich);
    if (m_creationDate.isValid()) {
        old->setCreated(m_creationDate);
    }

    m_item.setPayload<KCalCore::Incidence::Ptr>(old);
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

    m_title = inc->summary();
    m_titleIsRich = inc->summaryIsRich();
    m_text = inc->description();
    m_textIsRich = inc->descriptionIsRich();
    m_creationDate = inc->created();
    m_dataFetched = true;
}




QString IncidenceItem::mimeType()
{
    const KCalCore::Incidence::Ptr old = CalendarSupport::incidence(m_item); //same as hasValidPayload + getting payload
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
    if ( const KCalCore::Event::Ptr t = CalendarSupport::event(m_item) ) {
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
    if ( const KCalCore::Event::Ptr t = CalendarSupport::event(m_item) ) {
        return t->dtStart();
    }
    kWarning() << "not an event, or no start date";
    return KDateTime();
}

void IncidenceItem::setEventStart(const KDateTime &date)
{
    if ( const KCalCore::Event::Ptr t = CalendarSupport::event(m_item) ) {
        t->setDtStart(date);
    }
}


void IncidenceItem::setParentTodo(const IncidenceItem &parent)
{
    if ( const KCalCore::Todo::Ptr t = CalendarSupport::todo(m_item) ) {
        const KCalCore::Todo::Ptr p = CalendarSupport::todo(parent.getItem());
        t->setRelatedTo(p->uid());
    }
}


void IncidenceItem::setDueDate(const KDateTime &date, bool hasDueDate)
{
    if ( const KCalCore::Todo::Ptr t = CalendarSupport::todo(m_item) ) {
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
    if ( const KCalCore::Todo::Ptr t = CalendarSupport::todo(m_item) ) {
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
    if ( const KCalCore::Todo::Ptr t = CalendarSupport::todo(m_item) ) {
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
    if ( const KCalCore::Todo::Ptr t = CalendarSupport::todo(m_item) ) {
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
    if ( const KCalCore::Todo::Ptr t = CalendarSupport::todo(m_item) ) {
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
    if ( const KCalCore::Todo::Ptr t = CalendarSupport::todo(m_item) ) {
        switch (status) {
            case Complete:
                t->setCompleted(true);
                break;
            case Later:
                t->setCompleted(false);
                t->setPriority(9);
                break;
            case Now:
                t->setCompleted(false);
                t->setPriority(1);
                break;
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
    if ( const KCalCore::Todo::Ptr t = CalendarSupport::todo(m_item) ) {
        if (t->isCompleted()) {
            //kDebug() << "iscomplete";
            return Complete;
        }
        if (t->priority() == 1) {
            //kDebug() << "Now";
            return Now;
        }
        if (t->hasDueDate() && (t->dtDue() <= KDateTime::currentLocalDateTime())) {
            return Attention;
        }
        //kDebug() << "Later";
        return Later;
    }
    if ( const KCalCore::Event::Ptr t = CalendarSupport::event(m_item) ) {
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
    if ( const KCalCore::Todo::Ptr t = CalendarSupport::todo(m_item) ) {
        if (t->hasDueDate()) {
            //kDebug() << "due date: " << t->dtDue();
            return t->dtDue();
        } else {
            //kDebug() << "mod date: " << modificationTime();
            return getLastModifiedDate();
        }
    } else if ( const KCalCore::Event::Ptr e = CalendarSupport::event(m_item) ) {
        //if ( !e->recurs() && !e->isMultiDay() ) {
            return e->dtStart();
        //}
    } else if ( const KCalCore::Journal::Ptr j = CalendarSupport::journal(m_item) ) {
        return j->dtStart();
    }
    kWarning() << "unknown item";
    return KDateTime();
}

QString IncidenceItem::getIconName()
{
    KCalCore::Incidence::Ptr old = CalendarSupport::incidence(m_item);
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
    KCalCore::Incidence::Ptr old = CalendarSupport::incidence(m_item);
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


