/* This file is part of Zanshin Todo.

   Copyright 2008-2009 Kevin Ottens <ervin@kde.org>

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

#ifndef ZANSHIN_SIDEBAR_H
#define ZANSHIN_SIDEBAR_H

#include <QtCore/QModelIndex>
#include <QtGui/QWidget>

#include "globaldefs.h"

class KAction;
class KActionCollection;
class QAbstractItemModel;
class QItemSelectionModel;
class QStackedWidget;
class ModelStack;
class SideBarPage;


class SideBar : public QWidget
{
    Q_OBJECT

public:
    SideBar(ModelStack *models, KActionCollection *ac, QWidget *parent=0);

    void setMode(Zanshin::ApplicationMode mode);

private slots:
    void updateActions(const QModelIndex &index);
    void onAddItem();
    void onRemoveItem();
    void onRenameItem();
    void onPreviousItem();
    void onNextItem();
    void onSynchronize();

private:
    void createPage(QAbstractItemModel *model, QItemSelectionModel *selectionModel);
    void setupToolBar();
    void setupActions(KActionCollection *ac);

    SideBarPage *currentPage() const;
    SideBarPage *page(int idx) const;

    void addNewProject();
    void removeCurrentProject();
    void addNewContext();
    void removeCurrentContext();

    QStackedWidget *m_stack;

    KAction *m_add;
    KAction *m_remove;
    KAction *m_rename;
    KAction *m_previous;
    KAction *m_next;
    KAction *m_synchronize;
};

#endif

