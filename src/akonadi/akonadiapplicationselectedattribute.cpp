/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "akonadiapplicationselectedattribute.h"

using namespace Akonadi;

ApplicationSelectedAttribute::ApplicationSelectedAttribute()
    : Attribute(),
      m_selected(true)
{
}

ApplicationSelectedAttribute::~ApplicationSelectedAttribute()
{
}

void ApplicationSelectedAttribute::setSelected(bool selected)
{
    m_selected = selected;
}

bool ApplicationSelectedAttribute::isSelected() const
{
    return m_selected;
}

ApplicationSelectedAttribute *ApplicationSelectedAttribute::clone() const
{
    auto attr = new ApplicationSelectedAttribute();
    attr->m_selected = m_selected;
    return attr;
}

QByteArray ApplicationSelectedAttribute::type() const
{
    return "ZanshinSelected";
}

QByteArray ApplicationSelectedAttribute::serialized() const
{
    return m_selected ? "true" : "false";
}

void ApplicationSelectedAttribute::deserialize(const QByteArray &data)
{
    m_selected = (data == "true");
}
