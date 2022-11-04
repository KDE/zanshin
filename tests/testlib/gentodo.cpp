/*
 * SPDX-FileCopyrightText: 2015 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include "gentodo.h"

#include <KCalCore/Todo>
#include <QDateTime>
#include <akonadi/akonadiserializer.h>

using namespace Testlib;
using Akonadi::Serializer;

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
        todo->removeCustomProperty(Serializer::customPropertyAppName(), Serializer::customPropertyContextList());
    else
        todo->setCustomProperty(Serializer::customPropertyAppName(), Serializer::customPropertyContextList(), contextUids.join(','));
    return *this;
}

GenTodo &GenTodo::asProject(bool value)
{
    auto todo = m_item.payload<KCalCore::Todo::Ptr>();
    if (value)
        todo->setCustomProperty(Serializer::customPropertyAppName(), Serializer::customPropertyIsProject(), QStringLiteral("1"));
    else
        todo->removeCustomProperty(Serializer::customPropertyAppName(), Serializer::customPropertyIsProject());
    return *this;
}

GenTodo &GenTodo::asContext(bool value)
{
    auto todo = m_item.payload<KCalCore::Todo::Ptr>();
    if (value)
        todo->setCustomProperty(Serializer::customPropertyAppName(), Serializer::customPropertyIsContext(), QStringLiteral("1"));
    else
        todo->removeCustomProperty(Serializer::customPropertyAppName(), Serializer::customPropertyIsContext());
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
    m_item.payload<KCalCore::Todo::Ptr>()->setCompleted(QDate::fromString(date, Qt::ISODate).startOfDay());
    return *this;
}

GenTodo &GenTodo::withDoneDate(const QDate &date)
{
    m_item.payload<KCalCore::Todo::Ptr>()->setCompleted(date.startOfDay());
    return *this;
}

GenTodo &GenTodo::withStartDate(const QString &date)
{
    m_item.payload<KCalCore::Todo::Ptr>()->setDtStart(QDate::fromString(date, Qt::ISODate).startOfDay());
    return *this;
}

GenTodo &GenTodo::withStartDate(const QDate &date)
{
    m_item.payload<KCalCore::Todo::Ptr>()->setDtStart(date.startOfDay());
    return *this;
}

GenTodo &GenTodo::withDueDate(const QString &date)
{
    m_item.payload<KCalCore::Todo::Ptr>()->setDtDue(QDate::fromString(date, Qt::ISODate).startOfDay());
    return *this;
}

GenTodo &GenTodo::withDueDate(const QDate &date)
{
    m_item.payload<KCalCore::Todo::Ptr>()->setDtDue(date.startOfDay());
    return *this;
}
