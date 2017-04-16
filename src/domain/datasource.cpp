/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>

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


#include "datasource.h"

using namespace Domain;

DataSource::DataSource(QObject *parent)
    : QObject(parent),
      m_contentTypes(NoContent),
      m_selected(false)
{
}

DataSource::~DataSource()
{
}

QString DataSource::name() const
{
    return m_name;
}

QString DataSource::iconName() const
{
    return m_iconName;
}

DataSource::ContentTypes DataSource::contentTypes() const
{
    return m_contentTypes;
}

bool DataSource::isSelected() const
{
    return m_selected;
}

void DataSource::setName(const QString &name)
{
    if (m_name == name)
        return;

    m_name = name;
    emit nameChanged(name);
}

void DataSource::setIconName(const QString &iconName)
{
    if (m_iconName == iconName)
        return;

    m_iconName = iconName;
    emit iconNameChanged(iconName);
}

void DataSource::setContentTypes(ContentTypes types)
{
    if (m_contentTypes == types)
        return;

    m_contentTypes = types;
    emit contentTypesChanged(types);
}

void DataSource::setSelected(bool selected)
{
    if (m_selected == selected)
        return;

    m_selected = selected;
    emit selectedChanged(selected);
}
