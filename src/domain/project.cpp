/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "project.h"

using namespace Domain;

Project::Project(QObject *parent)
    : QObject(parent)
{
}

Project::~Project()
{
}

QString Project::name() const
{
    return m_name;
}

void Project::setName(const QString &name)
{
    if (m_name == name)
        return;

    m_name = name;
    Q_EMIT nameChanged(name);
}
