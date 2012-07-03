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

#include <KDE/Akonadi/ItemModifyJob>
#include <KDE/KCalCore/Todo>
#include <KDE/KDebug>
#include <KDE/KGlobal>
#include <KDE/KLocale>
#include <KDE/KMessageBox>

#include "globaldefs.h"
#include "todohelpers.h"

K_GLOBAL_STATIC(CategoryManager, s_contextManager)
K_GLOBAL_STATIC(CategoryManager, s_topicManager)

CategoryManager &CategoryManager::contextInstance()
{
    return *s_contextManager;
}

CategoryManager &CategoryManager::topicInstance()
{
    return *s_contextManager;
}


CategoryManager::CategoryManager(QObject *parent)
    : QObject(parent)
{
}

CategoryManager::~CategoryManager()
{
}

void CategoryManager::setCategoriesStructure(PimItemRelationsStructure *s)
{
    m_categoriesStructure = s;
}

void CategoryManager::addCategory(const QString &category, const IdList &parentCategory)
{
    kDebug() << category << parentCategory;
    m_categoriesStructure->addNode(category, parentCategory);
}

bool CategoryManager::removeCategories(QWidget* parent, const IdList& categories)
{
    if (parent) {
        QStringList categoryList;
        foreach (Id category, categories) {
            categoryList << m_categoriesStructure->getName(category);
        }
        QString categoryName = categoryList.join(", ");
        kDebug() << categories << categoryList;
        QString title;
        QString text;
        if (categories.size() > 1) {
            text = i18n("Do you really want to delete the context '%1'? All actions won't be associated to this context anymore.", categoryName);
            title = i18n("Delete Context");
        } else {
            text = i18n("Do you really want to delete the contexts '%1'? All actions won't be associated to those contexts anymore.", categoryName);
            title = i18n("Delete Contexts");
        }
        int button = KMessageBox::questionYesNo(parent, text, title);
        bool canRemove = (button==KMessageBox::Yes);

        if (!canRemove) {
            return false;
        }
    }
    kDebug() << "remove " << categories;
    foreach (Id id, categories) {
        m_categoriesStructure->removeNode(id);
    }
    return true;
}

bool CategoryManager::removeCategory(const Id &id)
{
    kDebug() << id;
    m_categoriesStructure->removeNode(id);
    return true;
}

bool CategoryManager::dissociateFromCategory(const Akonadi::Item& item, Id category)
{
    kDebug() << item.id() << category;
    if (!item.isValid()) {
        return false;
    }
    Id id = m_categoriesStructure->getItemId(item);
    IdList parents = m_categoriesStructure->getParents(id);
    parents.removeAll(category);
    m_categoriesStructure->moveNode(id, parents);
    return true;
}

bool CategoryManager::moveToCategory(Id id, Id category, Zanshin::ItemType parentType)
{
    kDebug() << id << category;
    if (parentType!=Zanshin::Category && parentType!=Zanshin::CategoryRoot) { //TODO shouldn't be necessary
        return false;
    }
    m_categoriesStructure->moveNode(id, IdList() << category);
    return true;
}

bool CategoryManager::renameCategory(Id id, const QString &newName)
{
    m_categoriesStructure->renameNode(id, newName);
    return true;
}

IdList CategoryManager::getParents(const Akonadi::Item& item) const
{
    if (!item.isValid()) {
        return IdList();
    }
    Id id = m_categoriesStructure->getItemId(item);
    return m_categoriesStructure->getParents(id);
}

IdList CategoryManager::getAncestors(Id id) const
{
    IdList ancestors;
    foreach (Id parent, m_categoriesStructure->getParents(id)) {
        kDebug() << m_categoriesStructure->getPath(parent) << parent;
        ancestors << parent;
        ancestors << getAncestors(parent);
    }
    return ancestors;
}

IdList CategoryManager::getAncestors(const Akonadi::Item& item) const
{
    if (!item.isValid()) {
        return IdList();
    }
    IdList ancestors;
    Id id = m_categoriesStructure->getItemId(item);
    kDebug() << "ancestors of " << m_categoriesStructure->getPath(id) << id;
    return getAncestors(id);
}


