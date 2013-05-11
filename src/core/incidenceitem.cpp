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
#include "pimitemrelations.h"

#include <Akonadi/EntityDisplayAttribute>

#include <kcalcore/event.h>
#include <kcalcore/incidence.h>
#include <kcalcore/journal.h>
#include <kcalcore/todo.h>

template<class T>
typename T::Ptr unwrap(const Akonadi::Item &item)
{
    Q_ASSERT(item.hasPayload<typename T::Ptr>());
    return item.payload< typename T::Ptr>();
}

IncidenceItem::IncidenceItem(PimItem::ItemType type)
: PimItem()
{
    KCalCore::Incidence *newItem = 0;
    if (type == PimItem::Todo) {
        newItem = new KCalCore::Todo();
    } else if (type == PimItem::Event) {
        newItem = new KCalCore::Event();
    }
    Q_ASSERT(newItem);
    KCalCore::Incidence::Ptr newPtr(newItem);
    m_item.setPayload<KCalCore::Incidence::Ptr>(newPtr);
    m_item.setMimeType(mimeType());
    Akonadi::EntityDisplayAttribute *eda = new Akonadi::EntityDisplayAttribute();
    eda->setIconName(getIconName());
    m_item.addAttribute(eda);
}

IncidenceItem::IncidenceItem(const Akonadi::Item &item)
:   PimItem(item)
{
}

QString IncidenceItem::getUid()
{
    return unwrap<KCalCore::Incidence>(m_item)->uid();
}

void IncidenceItem::setTitle(const QString &title, bool isRich)
{
    unwrap<KCalCore::Incidence>(m_item)->setSummary(title, isRich);
    Akonadi::EntityDisplayAttribute *eda = m_item.attribute<Akonadi::EntityDisplayAttribute>(Akonadi::Entity::AddIfMissing);
    eda->setIconName(getIconName());
    eda->setDisplayName(title);
}

QString IncidenceItem::getTitle()
{
    return unwrap<KCalCore::Incidence>(m_item)->summary();
}

bool IncidenceItem::titleIsRich()
{
    return unwrap<KCalCore::Incidence>(m_item)->summaryIsRich();
}

void IncidenceItem::setText(const QString &text, bool isRich)
{
    unwrap<KCalCore::Incidence>(m_item)->setDescription(text, isRich);
}

QString IncidenceItem::getText()
{
    return unwrap<KCalCore::Incidence>(m_item)->description();
}

bool IncidenceItem::textIsRich()
{
    return unwrap<KCalCore::Incidence>(m_item)->descriptionIsRich();
}

void IncidenceItem::setCreationDate(const KDateTime &dt)
{
    unwrap<KCalCore::Incidence>(m_item)->setCreated(dt);
}

KDateTime IncidenceItem::getCreationDate()
{
    return unwrap<KCalCore::Incidence>(m_item)->created();
}

KDateTime IncidenceItem::getLastModifiedDate()
{
    return unwrap<KCalCore::Incidence>(m_item)->lastModified();
}

bool IncidenceItem::hasValidPayload()
{
    return m_item.hasPayload<KCalCore::Incidence::Ptr>();
}

const KCalCore::Attachment::List IncidenceItem::getAttachments()
{
    return unwrap<KCalCore::Incidence>(m_item)->attachments();
}

QString IncidenceItem::mimeType()
{
    const KCalCore::Incidence::Ptr old = unwrap<KCalCore::Incidence>(m_item); //same as hasValidPayload + getting payload
    if (!old) {
        kWarning() << "invalid item";
        return QString();
    }
    return old->mimeType();
}

bool IncidenceItem::hasStartDate() const
{
    if ( const KCalCore::Event::Ptr t = unwrap<KCalCore::Event>(m_item) ) {
        return t->dtStart().isValid();
    }
    return false;
}


KDateTime IncidenceItem::getEventStart()
{
    if ( const KCalCore::Event::Ptr t = unwrap<KCalCore::Event>(m_item) ) {
        return t->dtStart();
    }
    kWarning() << "not an event, or no start date";
    return KDateTime();
}

void IncidenceItem::setEventStart(const KDateTime &date)
{
    if ( const KCalCore::Event::Ptr t = unwrap<KCalCore::Event>(m_item) ) {
        t->setDtStart(date);
    }
}

void IncidenceItem::setParentTodo(const IncidenceItem &parent)
{
    if ( const KCalCore::Todo::Ptr t = unwrap<KCalCore::Todo>(m_item) ) {
        const KCalCore::Todo::Ptr p = unwrap<KCalCore::Todo>(parent.getItem());
        t->setRelatedTo(p->uid());
    }
}

void IncidenceItem::setDueDate(const KDateTime &date, bool hasDueDate)
{
    if ( const KCalCore::Todo::Ptr t = unwrap<KCalCore::Todo>(m_item) ) {
        t->setDtDue(date);
        t->setHasDueDate(hasDueDate);
    }
}

KDateTime IncidenceItem::getDueDate()
{
    if ( const KCalCore::Todo::Ptr t = unwrap<KCalCore::Todo>(m_item) ) {
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
    if ( const KCalCore::Todo::Ptr t = unwrap<KCalCore::Todo>(m_item) ) {
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

void IncidenceItem::setTodoStatus(PimItem::ItemStatus status)
{
    //kDebug() << status;
    if ( const KCalCore::Todo::Ptr t = unwrap<KCalCore::Todo>(m_item) ) {
        switch (status) {
            case NotComplete:
                t->setCompleted(false);
                break;
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


PimItem::ItemStatus IncidenceItem::getStatus() const
{
    if ( const KCalCore::Todo::Ptr t = unwrap<KCalCore::Todo>(m_item) ) {
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
    if ( const KCalCore::Event::Ptr t = unwrap<KCalCore::Event>(m_item) ) {
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
    if ( const KCalCore::Todo::Ptr t = unwrap<KCalCore::Todo>(m_item) ) {
        if (t->hasDueDate()) {
            //kDebug() << "due date: " << t->dtDue();
            return t->dtDue();
        } else {
            //kDebug() << "mod date: " << modificationTime();
            return getLastModifiedDate();
        }
    } else if ( const KCalCore::Event::Ptr e = unwrap<KCalCore::Event>(m_item) ) {
        //if ( !e->recurs() && !e->isMultiDay() ) {
            return e->dtStart();
        //}
    } else if ( const KCalCore::Journal::Ptr j = unwrap<KCalCore::Journal>(m_item) ) {
        return j->dtStart();
    }
    kWarning() << "unknown item";
    return KDateTime();
}

QString IncidenceItem::getIconName()
{
    KCalCore::Incidence::Ptr old = unwrap<KCalCore::Incidence>(m_item);
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

PimItem::ItemType IncidenceItem::itemType()
{
    KCalCore::Incidence::Ptr old = unwrap<KCalCore::Incidence>(m_item);
    if (!old) {
        kWarning() << "invalid item";
        return PimItem::Incidence;
    }
    if ( old->type() == KCalCore::IncidenceBase::TypeTodo ) {
        return PimItem::Todo;
    } else if ( old->type() == KCalCore::IncidenceBase::TypeJournal ) {
        return PimItem::Journal;
    } else if ( old->type() == KCalCore::IncidenceBase::TypeEvent ) {
        return PimItem::Event;
    }
    return PimItem::Incidence;
}

void IncidenceItem::setRelations(const QList< PimItemRelation > &relations)
{
    KCalCore::Incidence::Ptr i = unwrap<KCalCore::Incidence>(m_item);
    QMap<QByteArray, QString> map = i->customProperties();
    map.remove("X-pimitemrelation");
    i->removeNonKDECustomProperty("X-pimitemrelation");
    foreach (const PimItemRelation &rel, relations) {
        if (rel.parentNodes.isEmpty()) {
            continue;
        }
        if (rel.type == PimItemRelation::Project) {
            i->setRelatedTo(rel.parentNodes.first().uid);
        } else {
            map.insertMulti("X-pimitemrelation", relationToXML(rel));
        }
    }
    i->setCustomProperties(map);
}

QList< PimItemRelation > IncidenceItem::getRelations()
{
    KCalCore::Incidence::Ptr i = unwrap<KCalCore::Incidence>(m_item);
    QList<PimItemRelation> relations;
    if (!i->relatedTo().isEmpty()) {
        relations << PimItemRelation(PimItemRelation::Project, QList<PimItemTreeNode>() << PimItemTreeNode(i->relatedTo().toUtf8()));
    }
    foreach(const QByteArray &key, i->customProperties().keys()) {
//         kDebug() <<  key << i->customProperties().value(key);
        if (key != "X-pimitemrelation") {
            continue;
        }
        relations << relationFromXML(i->customProperties().value(key).toLatin1());
    }
    return relations;
}

void IncidenceItem::setContexts(const QStringList &contexts)
{
    unwrap<KCalCore::Incidence>(m_item)->setCategories(contexts);
}

QStringList IncidenceItem::getContexts()
{
    return unwrap<KCalCore::Incidence>(m_item)->categories();
}

void IncidenceItem::setProject()
{
    if (isProject()) {
        return;
    }
    unwrap<KCalCore::Incidence>(m_item)->setCustomProperty("Zanshin", "Project", "true");
}

bool IncidenceItem::isProject() const
{
    const KCalCore::Incidence::Ptr i = unwrap<KCalCore::Incidence>(m_item);
    if (i->comments().contains("X-Zanshin-Project")) {
        return true;
    }
    return !i->customProperty("Zanshin", "Project").isEmpty();
}
