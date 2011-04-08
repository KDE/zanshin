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

#include "modelstructure.h"

#include <algorithm>

using namespace Zanshin::Test;

ModelStructure::ModelStructure()
    : m_latestNode(0),
      m_latestIndent(0)
{
}

ModelStructure::~ModelStructure()
{
    qDeleteAll(m_roots);
}

ModelStructure::ModelStructure(const ModelNode &node)
    : m_latestNode(new ModelStructureTreeNode(node)),
      m_latestIndent(0)
{
    Q_ASSERT(node.indent()==0);
    m_roots << m_latestNode;
}

static void _structureRecursiveReplay(ModelStructure &target, const QList<ModelStructureTreeNode*> nodes)
{
    foreach (ModelStructureTreeNode *node, nodes) {
        target << node->modelNode();
        _structureRecursiveReplay(target, node->children());
    }
}

ModelStructure::ModelStructure(const ModelStructure &other)
    : m_latestNode(0),
      m_latestIndent(0)
{
    _structureRecursiveReplay(*this, other.m_roots);
}

ModelStructure &ModelStructure::operator=(const ModelStructure &other)
{
    ModelStructure s(other);
    std::swap(*this, s);
    return *this;
}

ModelStructure &ModelStructure::operator<<(const ModelNode &node)
{
    Q_ASSERT((node.indent()<=m_latestIndent)
          || (node.indent()==m_latestIndent+1));

    if (node.indent()==m_latestIndent) {
        if (m_latestNode) {
            m_latestNode = new ModelStructureTreeNode(node, m_latestNode->parent());
        } else {
            m_latestNode = new ModelStructureTreeNode(node, 0);
        }

        if (m_latestNode->parent()==0) {
            m_roots << m_latestNode;
        }

    } else if (node.indent()==m_latestIndent+1) {
        m_latestNode = new ModelStructureTreeNode(node, m_latestNode);

    } else /*if (node.indent()<=m_latestIndent-1)*/ {
        quint64 tmpIndent = m_latestIndent;
        ModelStructureTreeNode *parent = m_latestNode->parent();
        while (tmpIndent>node.indent()) {
            tmpIndent--;
            parent = parent->parent();
        }

        m_latestNode = new ModelStructureTreeNode(node, parent);

        if (parent==0) { // We created a root
            m_roots << m_latestNode;
        }
    }

    m_latestIndent = node.indent();

    return *this;
}

void ModelStructure::clear()
{
    qDeleteAll(m_roots);
    m_roots.clear();
    m_latestIndent = 0;
    m_latestNode = 0;
}

ModelStructureTreeNode::ModelStructureTreeNode()
    : m_modelNode(ModelNode()),
      m_parent(0)
{
}

ModelStructureTreeNode::ModelStructureTreeNode(const ModelNode &node, ModelStructureTreeNode *parent)
    : m_modelNode(node),
      m_parent(parent)
{
    if (m_parent) {
        m_parent->m_children << this;
    }
}

ModelStructureTreeNode::~ModelStructureTreeNode()
{
    if (m_parent) {
        m_parent->m_children.removeAll(this);
    }
    qDeleteAll(m_children);
}

ModelNode ModelStructureTreeNode::modelNode() const
{
    return m_modelNode;
}

ModelStructureTreeNode *ModelStructureTreeNode::parent() const
{
    return m_parent;
}

QList<ModelStructureTreeNode*> ModelStructureTreeNode::children() const
{
    return m_children;
}
