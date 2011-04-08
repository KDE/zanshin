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

#ifndef ZANSHIN_CATEGORYMANAGER_H
#define ZANSHIN_CATEGORYMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QStringList>
#include "globaldefs.h"

class QAbstractItemModel;
class QModelIndex;
class TodoCategoriesModel;

class CategoryManager : public QObject
{
    Q_OBJECT

public:
    static CategoryManager &instance();

    CategoryManager(QObject *parent = 0);
    virtual ~CategoryManager();

    void setModel(QAbstractItemModel *model);
    QAbstractItemModel *model() { return m_model; }

    void addCategory(const QString &category, const QString &parentCategory);
    void addCategory(const QString &categoryPath);
    bool removeCategory(QWidget *parent, const QModelIndex &categoryIndex);
    bool removeTodoFromCategory(const QModelIndex &index, const QString &categoryPath);
    void renameCategory(const QString &oldCategoryPath, const QString &newCategoryPath);
    void moveCategory(const QString &oldCategoryPath, const QString &parentCategoryPath, Zanshin::ItemType parentType);
    bool moveTodoToCategory(const QModelIndex &index, const QString &categoryPath, const Zanshin::ItemType parentType);
    bool moveTodoToCategory(const Akonadi::Item &item, const QString &categoryPath, const Zanshin::ItemType parentType);

    QStringList categories();

    static const QChar pathSeparator();

Q_SIGNALS:
    void categoryAdded(const QString &categoryPath);
    void categoryRemoved(const QString &categoryPath);
    void categoryRenamed(const QString &oldCategoryPath, const QString &newCategoryPath);
    void categoryMoved(const QString &oldCategoryPath, const QString &newCategoryPath);

private slots:
    void onSourceInsertRows(const QModelIndex &sourceIndex, int begin, int end);
    void onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end);

private:
    friend class TodoCategoriesModel;

    bool removeCategory(const QString &categoryPath);
    void removeCategoryFromTodo(const QModelIndex &sourceIndex, const QString &categoryPath);
    void renameCategory(const QModelIndex &sourceIndex, const QString &oldCategoryPath, const QString &newCategoryPath);

    QStringList m_categories;
    QPointer<QAbstractItemModel> m_model;
};

#endif

