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


#include "akonadidatastore.h"
#include "akonadiincidenceitem.h"
#include "pimitemrelations.h"
#include "pimitemservices.h"

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

AkonadiIncidenceItem::AkonadiIncidenceItem(ItemType type)
    : AkonadiBaseItem()
{
    KCalCore::Incidence *newItem = 0;
    if (type == Todo) {
        newItem = new KCalCore::Todo();
    } else if (type == Event) {
        newItem = new KCalCore::Event();
    }
    Q_ASSERT(newItem);
    KCalCore::Incidence::Ptr newPtr(newItem);
    m_item.setPayload<KCalCore::Incidence::Ptr>(newPtr);
    m_item.setMimeType(mimeType());
    Akonadi::EntityDisplayAttribute *eda = new Akonadi::EntityDisplayAttribute();
    eda->setIconName(iconName());
    m_item.addAttribute(eda);
}

AkonadiIncidenceItem::AkonadiIncidenceItem(const Akonadi::Item &item)
    : AkonadiBaseItem(item)
{
}

QString AkonadiIncidenceItem::uid() const
{
    return unwrap<KCalCore::Incidence>(m_item)->uid();
}

void AkonadiIncidenceItem::setTitle(const QString &title, bool isRich)
{
    unwrap<KCalCore::Incidence>(m_item)->setSummary(title, isRich);
    Akonadi::EntityDisplayAttribute *eda = m_item.attribute<Akonadi::EntityDisplayAttribute>(Akonadi::Entity::AddIfMissing);
    eda->setIconName(iconName());
    eda->setDisplayName(title);
}

QString AkonadiIncidenceItem::title() const
{
    return unwrap<KCalCore::Incidence>(m_item)->summary();
}

bool AkonadiIncidenceItem::isTitleRich() const
{
    return unwrap<KCalCore::Incidence>(m_item)->summaryIsRich();
}

KDateTime AkonadiIncidenceItem::date(PimItem::DateRole role) const
{
    switch (role) {
    case PimItem::CreationDate:
        return unwrap<KCalCore::Incidence>(m_item)->created().toLocalZone();

    case PimItem::LastModifiedDate:
    {
        const KDateTime lastMod = unwrap<KCalCore::Incidence>(m_item)->lastModified();
        if (lastMod.isValid())
            return lastMod.toLocalZone();
        else
            return AkonadiBaseItem::date(role);
    }

    case PimItem::StartDate:
        return unwrap<KCalCore::Incidence>(m_item)->dtStart();

    case PimItem::EndDate:
        if ( const KCalCore::Event::Ptr t = unwrap<KCalCore::Event>(m_item) ) {
            return t->dtEnd();
        } else {
            return KDateTime();
        }

    case PimItem::DueDate:
        if ( const KCalCore::Todo::Ptr t = unwrap<KCalCore::Todo>(m_item) ) {
            return t->dtDue();
        } else {
            return KDateTime();
        }

    default:
        return AkonadiBaseItem::date(role);
    }
}

bool AkonadiIncidenceItem::setDate(PimItem::DateRole role, const KDateTime &date)
{
    switch (role) {
    case PimItem::CreationDate:
        unwrap<KCalCore::Incidence>(m_item)->setCreated(date);
        return true;

    case PimItem::LastModifiedDate:
        unwrap<KCalCore::Incidence>(m_item)->setLastModified(date);
        return AkonadiBaseItem::setDate(role, date);

    case PimItem::StartDate:
        unwrap<KCalCore::Incidence>(m_item)->setDtStart(date);
        return true;

    case PimItem::EndDate:
        if ( const KCalCore::Event::Ptr t = unwrap<KCalCore::Event>(m_item) ) {
            t->setDtEnd(date);
            return true;
        } else {
            return false;
        }

    case PimItem::DueDate:
        if ( const KCalCore::Todo::Ptr t = unwrap<KCalCore::Todo>(m_item) ) {
            t->setDtDue(date);
            return true;
        } else {
            return false;
        }

    default:
        return AkonadiBaseItem::setDate(role, date);
    }
}

void AkonadiIncidenceItem::setText(const QString &text, bool isRich)
{
    unwrap<KCalCore::Incidence>(m_item)->setDescription(text, isRich);
}

QString AkonadiIncidenceItem::text() const
{
    return unwrap<KCalCore::Incidence>(m_item)->description();
}

bool AkonadiIncidenceItem::isTextRich() const
{
    return unwrap<KCalCore::Incidence>(m_item)->descriptionIsRich();
}

const KCalCore::Attachment::List AkonadiIncidenceItem::attachments() const
{
    return unwrap<KCalCore::Incidence>(m_item)->attachments();
}

QString AkonadiIncidenceItem::mimeType() const
{
    const KCalCore::Incidence::Ptr old = unwrap<KCalCore::Incidence>(m_item); //same as hasValidPayload + getting payload
    if (!old) {
        kWarning() << "invalid item";
        return QString();
    }
    return old->mimeType();
}

void AkonadiIncidenceItem::setParentTodo(const AkonadiIncidenceItem &parent)
{
    if ( const KCalCore::Todo::Ptr t = unwrap<KCalCore::Todo>(m_item) ) {
        const KCalCore::Todo::Ptr p = unwrap<KCalCore::Todo>(parent.getItem());
        t->setRelatedTo(p->uid());
    }
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

void AkonadiIncidenceItem::setTodoStatus(PimItem::ItemStatus status)
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
                t->setDtStart(KDateTime());
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


PimItem::ItemStatus AkonadiIncidenceItem::status() const
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

QString AkonadiIncidenceItem::iconName() const
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

PimItem::ItemType AkonadiIncidenceItem::itemType() const
{
    KCalCore::Incidence::Ptr old = unwrap<KCalCore::Incidence>(m_item);
    if (!old) {
        kWarning() << "invalid item";
        return NoType;
    }
    if ( old->type() == KCalCore::IncidenceBase::TypeTodo ) {
        if (isProject())
            return Project;
        else
            return Todo;
    } else if ( old->type() == KCalCore::IncidenceBase::TypeJournal ) {
        return Journal;
    } else if ( old->type() == KCalCore::IncidenceBase::TypeEvent ) {
        return Event;
    }
    return NoType;
}

void AkonadiIncidenceItem::setRelations(const QList< PimItemRelation > &relations)
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

QList< PimItemRelation > AkonadiIncidenceItem::relations() const
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

void AkonadiIncidenceItem::setContexts(const QStringList &contexts)
{
    unwrap<KCalCore::Incidence>(m_item)->setCategories(contexts);
}

QStringList AkonadiIncidenceItem::contexts() const
{
    return unwrap<KCalCore::Incidence>(m_item)->categories();
}

void AkonadiIncidenceItem::setProject()
{
    if (isProject()) {
        return;
    }
    unwrap<KCalCore::Incidence>(m_item)->setCustomProperty("Zanshin", "Project", "true");
}

bool AkonadiIncidenceItem::isProject() const
{
    return AkonadiDataStore::instance().isProject(m_item);
}
