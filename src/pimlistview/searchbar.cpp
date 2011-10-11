/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) <year>  <name of author>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include "searchbar.h"

#include <QSortFilterProxyModel>
#include <QTimer>
#include "notesortfilterproxymodel.h"

SearchBar::SearchBar(NoteSortFilterProxyModel *filter, QWidget* parent)
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

