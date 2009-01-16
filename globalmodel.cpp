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

#include "globalmodel.h"

#include <akonadi/attributefactory.h>
#include <akonadi/collectionmodel.h>
#include <akonadi/collectionfilterproxymodel.h>

#include <kglobal.h>

#include "contextsmodel.h"
#include "librarymodel.h"
#include "projectsmodel.h"
#include "todocategoriesattribute.h"
#include "todocategoriesmodel.h"
#include "todoflatmodel.h"
#include "todotreemodel.h"

class GlobalModelPrivate
{
public:
    GlobalModelPrivate()
    {
        Akonadi::AttributeFactory::registerAttribute<TodoCategoriesAttribute>();

        todoFlat = new TodoFlatModel();

        todoTree = new TodoTreeModel();
        todoTree->setSourceModel(todoFlat);

        todoCategories = new TodoCategoriesModel();
        todoCategories->setSourceModel(todoFlat);

        contexts = new ContextsModel();
        contexts->setSourceModel(todoCategories);

        projects = new ProjectsModel();
        projects->setSourceModel(todoTree);

        contextsLibrary = new LibraryModel();
        contextsLibrary->setSourceModel(contexts);
        contextsLibrary->setType(LibraryModel::Contexts);

        projectsLibrary = new LibraryModel();
        projectsLibrary->setSourceModel(projects);

        todoCollections = new Akonadi::CollectionFilterProxyModel();
        Akonadi::CollectionModel *collectionModel = new Akonadi::CollectionModel(todoCollections);
        todoCollections->setSourceModel(collectionModel);
        todoCollections->addMimeTypeFilter("application/x-vnd.akonadi.calendar.todo");
    }

    ~GlobalModelPrivate()
    {
        delete projectsLibrary;
        delete contextsLibrary;
        delete projects;
        delete contexts;
        delete todoCategories;
        delete todoTree;
        delete todoFlat;
        delete todoCollections;
    }

    TodoFlatModel *todoFlat;
    TodoTreeModel *todoTree;
    TodoCategoriesModel *todoCategories;

    ContextsModel *contexts;
    ProjectsModel *projects;

    LibraryModel *contextsLibrary;
    LibraryModel *projectsLibrary;

    Akonadi::CollectionFilterProxyModel *todoCollections;
};

K_GLOBAL_STATIC(GlobalModelPrivate, globalModel)

TodoFlatModel *GlobalModel::todoFlat()
{
    return globalModel->todoFlat;
}

TodoTreeModel *GlobalModel::todoTree()
{
    return globalModel->todoTree;
}

TodoCategoriesModel *GlobalModel::todoCategories()
{
    return globalModel->todoCategories;
}

QAbstractItemModel *GlobalModel::todoCollections()
{
    return globalModel->todoCollections;
}

ContextsModel *GlobalModel::contexts()
{
    return globalModel->contexts;
}

ProjectsModel *GlobalModel::projects()
{
    return globalModel->projects;
}

LibraryModel *GlobalModel::contextsLibrary()
{
    return globalModel->contextsLibrary;
}

LibraryModel *GlobalModel::projectsLibrary()
{
    return globalModel->projectsLibrary;
}
