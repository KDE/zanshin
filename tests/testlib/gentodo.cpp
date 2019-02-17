/* This file is part of Zanshin

   Copyright 2015 Kevin Ottens <ervin@kde.org>

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

#include "gentodo.h"

#include <KCalCore/Todo>
#include <QDateTime>

using namespace Testlib;

static const char s_contextListProperty[] = "ContextList";
static const char s_appName[] = "Zanshin";

GenTodo::GenTodo(const Akonadi::Item &item)
    : m_item(item)
{
    m_item.setMimeType(KCalCore::Todo::todoMimeType());
    if (!m_item.hasPayload<KCalCore::Todo::Ptr>())
        m_item.setPayload(KCalCore::Todo::Ptr::create());
}

Testlib::GenTodo::operator Akonadi::Item()
{
    return m_item;
}

GenTodo &GenTodo::withId(Akonadi::Item::Id id)
{
    m_item.setId(id);
    return *this;
}

GenTodo &GenTodo::withParent(Akonadi::Collection::Id id)
{
    m_item.setParentCollection(Akonadi::Collection(id));
    return *this;
}

GenTodo &GenTodo::withContexts(const QStringList &contextUids)
{
    auto todo = m_item.payload<KCalCore::Todo::Ptr>();
    if (contextUids.isEmpty())
        todo->removeCustomProperty(s_appName, s_contextListProperty);
    else
        todo->setCustomProperty(s_appName, s_contextListProperty, contextUids.join(','));
    m_item.setPayload<KCalCore::Todo::Ptr>(todo);
    return *this;
}

GenTodo &GenTodo::asProject(bool value)
{
    auto todo = m_item.payload<KCalCore::Todo::Ptr>();
    if (value)
        todo->setCustomProperty("Zanshin", "Project", QStringLiteral("1"));
    else
        todo->removeCustomProperty("Zanshin", "Project");
    return *this;
}

GenTodo &GenTodo::asContext(bool value)
{
    auto todo = m_item.payload<KCalCore::Todo::Ptr>();
    if (value)
        todo->setCustomProperty("Zanshin", "Context", QStringLiteral("1"));
    else
        todo->removeCustomProperty("Zanshin", "Context");
    return *this;
}

GenTodo &GenTodo::withUid(const QString &uid)
{
    m_item.payload<KCalCore::Todo::Ptr>()->setUid(uid);
    return *this;
}

GenTodo &GenTodo::withParentUid(const QString &uid)
{
    m_item.payload<KCalCore::Todo::Ptr>()->setRelatedTo(uid);
    return *this;
}

GenTodo &GenTodo::withTitle(const QString &title)
{
    m_item.payload<KCalCore::Todo::Ptr>()->setSummary(title);
    return *this;
}

GenTodo &GenTodo::withText(const QString &text)
{
    m_item.payload<KCalCore::Todo::Ptr>()->setDescription(text);
    return *this;
}

GenTodo &GenTodo::done(bool value)
{
    m_item.payload<KCalCore::Todo::Ptr>()->setCompleted(value);
    return *this;
}

GenTodo &GenTodo::withDoneDate(const QString &date)
{
    m_item.payload<KCalCore::Todo::Ptr>()->setCompleted(QDateTime(QDate::fromString(date, Qt::ISODate)));
    return *this;
}

GenTodo &GenTodo::withDoneDate(const QDate &date)
{
    m_item.payload<KCalCore::Todo::Ptr>()->setCompleted(QDateTime(date));
    return *this;
}

GenTodo &GenTodo::withStartDate(const QString &date)
{
    m_item.payload<KCalCore::Todo::Ptr>()->setDtStart(QDateTime(QDate::fromString(date, Qt::ISODate)));
    return *this;
}

GenTodo &GenTodo::withStartDate(const QDate &date)
{
    m_item.payload<KCalCore::Todo::Ptr>()->setDtStart(QDateTime(date));
    return *this;
}

GenTodo &GenTodo::withDueDate(const QString &date)
{
    m_item.payload<KCalCore::Todo::Ptr>()->setDtDue(QDateTime(QDate::fromString(date, Qt::ISODate)));
    return *this;
}

GenTodo &GenTodo::withDueDate(const QDate &date)
{
    m_item.payload<KCalCore::Todo::Ptr>()->setDtDue(QDateTime(date));
    return *this;
}
