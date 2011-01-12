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
#include <QtCore/QStringList>

class QAbstractItemModel;
class QModelIndex;

class CategoryManager : public QObject
{
    Q_OBJECT

public:
    static CategoryManager &instance();

    CategoryManager(QObject *parent = 0);
    virtual ~CategoryManager();

    void setModel(QAbstractItemModel *model);
    QAbstractItemModel *model() { return m_model; }

    void addCategory(const QString &category);
    bool removeCategory(const QString &category);

    QStringList categories();

Q_SIGNALS:
    void categoryAdded(const QString &category);
    void categoryRemoved(const QString &category);

private slots:
    void onSourceInsertRows(const QModelIndex &sourceIndex, int begin, int end);
    void onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end);

private:
    void removeCategoryFromTodo(const QModelIndex &sourceIndex, const QString &category);

    QStringList m_categories;
    QAbstractItemModel *m_model;
};

#endif

