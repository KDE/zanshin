/* This file is part of Zanshin Todo.

   Copyright 2011 Kevin Ottens <ervin@kde.org>

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

#include "modelnode.h"

#include <algorithm>

using namespace Zanshin::Test;

ModelNode::ModelNode()
{
}

ModelNode::ModelNode(const C &collection, const Indent &indent)
    : m_entity(QVariant::fromValue(collection)),
      m_indent(indent)
{
}

ModelNode::ModelNode(const T &todo, const Indent &indent)
    : m_entity(QVariant::fromValue(todo)),
      m_indent(indent)
{
}

ModelNode::ModelNode(const ModelNode &other)
    : m_entity(other.m_entity),
      m_indent(other.m_indent)
{
}

ModelNode &ModelNode::operator=(const ModelNode &other)
{
    ModelNode node(other);
    std::swap(*this, node);
    return *this;
}

quint64 ModelNode::indent() const
{
    return m_indent.size;
}

QVariant ModelNode::entity() const
{
    return m_entity;
}

ModelNode operator+(const Indent& indent, const C &collection)
{
    return ModelNode(collection, indent);
}

ModelNode operator+(const Indent& indent, const T &todo)
{
    return ModelNode(todo, indent);
}

