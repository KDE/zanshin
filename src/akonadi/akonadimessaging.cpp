/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>

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


#include "akonadimessaging.h"

#include <QApplication>

#include <Akonadi/Calendar/ITIPHandler>

using namespace Akonadi;

Messaging::Messaging()
    : m_itip(new ITIPHandler)
{
    m_itip->setShowDialogsOnError(true);
}

Messaging::~Messaging()
{
    delete m_itip;
}

void Messaging::sendDelegationMessage(Item item)
{
    auto todo = item.payload<KCalCore::Todo::Ptr>();
    Q_ASSERT(todo);

    QWidget *window = 0;
    if (!QApplication::topLevelWidgets().isEmpty()) {
        window = QApplication::topLevelWidgets().first();
    }

    m_itip->sendiTIPMessage(KCalCore::iTIPRequest, todo, window);
}
