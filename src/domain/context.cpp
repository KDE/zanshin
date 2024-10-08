/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "context.h"

using namespace Domain;

Context::Context(QObject *parent)
    : QObject(parent)
{
}

Context::~Context()
{
}

QString Context::name() const
{
    return m_name;
}

void Context::setName(const QString &name)
{
    if (m_name == name)
        return;

    m_name = name;
    emit nameChanged(name);
}

QDebug operator<<(QDebug dbg, const Domain::Context &context)
{
    dbg << "Context" << context.name();
    return dbg;
}

QDebug operator<<(QDebug dbg, const Domain::Context::Ptr &context)
{
    if (!context)
        dbg << "Ptr to null context";
    else
        dbg << "Ptr to" << *context;
    return dbg;
}

#include "moc_context.cpp"
