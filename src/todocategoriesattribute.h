/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.
*/

#ifndef ZANSHIN_TODOCATEGORIESATTRIBUTE_H
#define ZANSHIN_TODOCATEGORIESATTRIBUTE_H

#include <akonadi/attribute.h>

#include <QtCore/QStringList>

class TodoCategoriesAttribute : public Akonadi::Attribute
{
public:
    TodoCategoriesAttribute();

    QStringList parentList() const;
    void setParentList(const QStringList &parentList);

    virtual QByteArray type() const;
    virtual Akonadi::Attribute *clone() const;
    virtual QByteArray serialized() const;
    virtual void deserialize(const QByteArray &data);

private:
    QStringList m_parentList;
};

#endif
