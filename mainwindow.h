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

#ifndef ZANSHIN_MAINWINDOW_H
#define ZANSHIN_MAINWINDOW_H

#include <KDE/KXmlGuiWindow>

#include <akonadi/collection.h>

class TodoFlatModel;
class TodoCategoriesModel;
class TodoTreeModel;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

private slots:
    void collectionClicked(const Akonadi::Collection &collection);

private:
    Akonadi::Collection m_currentCollection;
    TodoFlatModel *m_globalModel;
    TodoTreeModel *m_projectModel;
    TodoCategoriesModel *m_contextModel;
};

#endif

