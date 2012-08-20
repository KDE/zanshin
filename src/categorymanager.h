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
#include "reparentingmodel/pimitemrelations.h"

class QAbstractItemModel;
class QModelIndex;
class TodoCategoriesModel;
class CategoriesStrategy;

/**
 * TODO VirtualNodeManager
 *
 * The interface to edit(create/remove/rename/move) virtual nodes.
 *
 * TODO possibly use globally unique ids here (in case of multiple structures)
 */
class CategoryManager : public QObject
{
    Q_OBJECT

public:
    static CategoryManager &contextInstance();
    static CategoryManager &topicInstance();

    CategoryManager(QObject *parent = 0);
    virtual ~CategoryManager();

    void setCategoriesStructure(PimItemRelationsStructure *);

    void addCategory(const QString &category, const IdList &parentCategory = IdList());
    bool removeCategories(QWidget *parent, const IdList &categoryIndex);
    bool dissociateFromCategory(const Akonadi::Item &item, Id category);
    bool moveToCategory(Id id, Id category, Zanshin::ItemType parentType);
    bool renameCategory(Id id, const QString &name);

private:
    friend class PimItemRelationsStructure;

    bool removeCategory(const Id &categoryPath);

    IdList getAncestors(Id id) const;
    IdList getAncestors(const Akonadi::Item &item) const;
    IdList getParents(const Akonadi::Item &item) const;

    QPointer<PimItemRelationsStructure> m_categoriesStructure;
};

#endif

