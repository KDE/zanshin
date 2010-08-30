/* This file is part of Zanshin Todo.

   Copyright 2008-2010 Kevin Ottens <ervin@kde.org>

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

#ifndef ZANSHIN_MODELSTACK_H
#define ZANSHIN_MODELSTACK_H

#include <QtCore/QObject>

class QItemSelectionModel;
class QAbstractItemModel;

class ModelStack : public QObject
{
    Q_OBJECT

public:
    explicit ModelStack(QObject *parent = 0);

    QAbstractItemModel *baseModel();

    QAbstractItemModel *treeModel();
    QAbstractItemModel *treeSideBarModel();
    QAbstractItemModel *treeSelectionModel(QItemSelectionModel *selection = 0);
    QAbstractItemModel *treeComboModel();

    QAbstractItemModel *categoriesModel();
    QAbstractItemModel *categoriesSideBarModel();
    QAbstractItemModel *categoriesSelectionModel(QItemSelectionModel *selection = 0);
    QAbstractItemModel *categoriesComboModel();

private:
    QAbstractItemModel *m_baseModel;

    QAbstractItemModel *m_treeModel;
    QAbstractItemModel *m_treeSideBarModel;
    QAbstractItemModel *m_treeSelectionModel;
    QAbstractItemModel *m_treeComboModel;

    QAbstractItemModel *m_categoriesModel;
    QAbstractItemModel *m_categoriesSideBarModel;
    QAbstractItemModel *m_categoriesSelectionModel;
    QAbstractItemModel *m_categoriesComboModel;
};

#endif

