/* This file is part of Zanshin Todo.

   Copyright 2011 Mario Bensi <nef@ipsquad.net>

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

#include "categorymanager.h"

#include <QtCore/QAbstractItemModel>

#include <KDE/KDebug>
#include "kglobal.h"

#include "todomodel.h"


K_GLOBAL_STATIC(CategoryManager, s_categoryManager);

CategoryManager &CategoryManager::instance()
{
    return *s_categoryManager;
}


CategoryManager::CategoryManager(QObject *parent)
    : QObject(parent)
{
}

CategoryManager::~CategoryManager()
{
}

void CategoryManager::setModel(QAbstractItemModel *model)
{
    if (m_model) {
        disconnect(m_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)));
        disconnect(m_model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)));
    }

    if (model) {
        connect(model, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                this, SLOT(onSourceInsertRows(const QModelIndex&, int, int)));
        connect(model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
                this, SLOT(onSourceDataChanged(const QModelIndex&, const QModelIndex&)));
    }

    m_categories.clear();
    m_model = model;
}

QStringList CategoryManager::categories()
{
    return m_categories;
}

void CategoryManager::addCategory(const QString &category)
{
    if (!m_categories.contains(category)) {
        m_categories << category;
        emit categoryAdded(category);
    }
}

bool CategoryManager::removeCategory(const QString &category)
{
    int pos = m_categories.indexOf(category);
    if (pos != -1) {
        removeCategoryFromTodo(QModelIndex(), category);
        m_categories.removeAt(pos);
        emit categoryRemoved(category);
        return true;
    }
    return false;
}

void CategoryManager::onSourceInsertRows(const QModelIndex &sourceIndex, int begin, int end)
{
    for (int i=begin; i<=end; ++i) {
        QModelIndex sourceChildIndex = m_model->index(i, 0, sourceIndex);
        if (!sourceChildIndex.isValid()) {
            continue;
        }
        TodoModel::ItemType type = (TodoModel::ItemType) sourceChildIndex.data(TodoModel::ItemTypeRole).toInt();
        if (type==TodoModel::StandardTodo) {
            QStringList categories = m_model->data(sourceChildIndex, TodoModel::CategoriesRole).toStringList();
            foreach (QString category, categories) {
                addCategory(category);
            }
        } else if (type==TodoModel::Collection) {
            onSourceInsertRows(sourceChildIndex, 0, m_model->rowCount(sourceChildIndex)-1);
        }
    }
}

void CategoryManager::onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end)
{
    for (int row=begin.row(); row<=end.row(); ++row) {
        QModelIndex sourceIndex = begin.sibling(row, 0);
        QSet<QString> newCategories = QSet<QString>::fromList(sourceIndex.data(TodoModel::CategoriesRole).toStringList());

        QSet<QString> oldCategories = QSet<QString>::fromList(m_categories);
        QSet<QString> interCategories = newCategories;
        interCategories.intersect(oldCategories);
        newCategories-= interCategories;

        foreach (const QString &newCategory, newCategories) {
            addCategory(newCategory);
        }
    }
}

void CategoryManager::removeCategoryFromTodo(const QModelIndex &sourceIndex, const QString &category)
{
    for (int i=0; i < m_model->rowCount(sourceIndex); ++i) {
        QModelIndex child = m_model->index(i, 0, sourceIndex);
        QStringList categories = m_model->data(child, TodoModel::CategoriesRole).toStringList();
        if (categories.contains(category)) {
            categories.removeOne(category);
            QModelIndex index = m_model->index(i, 2, sourceIndex);
            m_model->setData(index, categories);
        }
        removeCategoryFromTodo(child, category);
    }
}
