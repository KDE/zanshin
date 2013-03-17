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

#include "searchbar.h"

#include <QSortFilterProxyModel>
#include <QTimer>
#include "filterproxymodel.h"

SearchBar::SearchBar(FilterProxyModel *filter, QWidget* parent)
: QLineEdit(parent),
 m_filterProxy(filter)
{
    connect (this, SIGNAL(textChanged(QString)), this, SLOT(evaluateInput()));
    setPlaceholderText("Filter...");
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), SLOT(validateCommand()));
    connect(this, SIGNAL(returnPressed()), SLOT(validateCommand()));
}

void SearchBar::evaluateInput()
{
    m_timer->start(300);
}

void SearchBar::validateCommand()
{
    m_filterProxy->setFilterString(text());
}

