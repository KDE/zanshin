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

class KAction;
class KActionCollection;
class QStackedWidget;
class QTreeView;

class SideBar : public QWidget
{
    Q_OBJECT

public:
    SideBar(QWidget *parent, KActionCollection *ac);

public slots:
    void switchToProjectMode();
    void switchToContextMode();

signals:
    void noProjectInboxActivated();
    void noContextInboxActivated();

    void projectActivated(const QModelIndex &index);
    void contextActivated(const QModelIndex &index);

private slots:
    void updateActions(const QModelIndex &index);
    void onAddFolder();
    void onAddItem();
    void onRemoveItem();
    void onCurrentProjectChangeDetected();
    void applyCurrentProjectChange();
    void onCurrentContextChangeDetected();
    void applyCurrentContextChange();

private:
    void setupProjectPage();
    void setupContextPage();
    void setupActions(KActionCollection *ac);

    void addNewProject();
    void removeCurrentProject();
    void addNewContext();
    void removeCurrentContext();

    enum {
        ProjectPageIndex = 0,
        ContextPageIndex = 1
    };

    QStackedWidget *m_stack;
    QTreeView *m_projectTree;
    QTreeView *m_contextTree;

    KAction *m_add;
    KAction *m_addFolder;
    KAction *m_remove;
    KAction *m_previous;
    KAction *m_next;
};

#endif

