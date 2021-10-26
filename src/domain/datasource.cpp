/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
