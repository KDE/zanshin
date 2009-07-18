/* This file is part of Zanshin Todo.

   Copyright 2009 Kevin Ottens <ervin@kde.org>

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

#ifndef ZANSHIN_QUICKSELECTDIALOG_H
#define ZANSHIN_QUICKSELECTDIALOG_H

#include <KDE/KDialog>

class QTreeView;

class QuickSelectDialog : public KDialog
{
    Q_OBJECT
    Q_ENUMS(Mode)

public:
    enum Mode {
        ContextMode,
        ProjectMode
    };

    enum ActionType {
        MoveAction,
        CopyAction,
        JumpAction
    };

    QuickSelectDialog(QWidget *parent, Mode mode, ActionType action);

    QString selectedId() const;

private:
    QString contextSelectedId() const;
    QString projectSelectedId() const;

    QTreeView *m_tree;
    Mode m_mode;
};

#endif

