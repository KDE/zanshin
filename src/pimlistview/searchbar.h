/* This file is part of Zanshin Todo.

   Copyright 2011 Christian Mollekopf <chrigi_1@fastmail.fm>

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

#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <QLineEdit>

class NoteSortFilterProxyModel;
class QTimer;
/**
 * A Filter can have the following items:
 * -regex (for header, tags, fulltext)
 * -date
 *
 */

class SearchBar : public QLineEdit
{
    Q_OBJECT
public:
    explicit SearchBar(NoteSortFilterProxyModel *filterProxy, QWidget* parent = 0);

signals:
    void filterChanged();

private slots:
    void evaluateInput();
    void validateCommand();
private:
    NoteSortFilterProxyModel *m_filterProxy;
    QTimer *m_timer;
    
};

#endif // COMMANDLINE_H
