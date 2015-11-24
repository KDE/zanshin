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
#include <QInputDialog>

#include <KPIMIdentities/Identity>
#include <KPIMIdentities/IdentityManager>
#include <Akonadi/Calendar/ETMCalendar>
#include <Akonadi/Calendar/ITIPHandler>

#include "utils/mem_fn.h"

using namespace Akonadi;

Messaging::Messaging()
    : m_itip(new ITIPHandler)
{
    m_itip->setShowDialogsOnError(true);

    auto calendar = new ETMCalendar(QStringList() << KCalCore::Todo::todoMimeType());
    m_itip->setCalendar(CalendarBase::Ptr(calendar));
}

Messaging::~Messaging()
{
    delete m_itip;
}

void Messaging::sendDelegationMessage(Item item)
{
    auto todo = item.payload<KCalCore::Todo::Ptr>();
    Q_ASSERT(todo);

    QWidget *window = Q_NULLPTR;
    if (!QApplication::topLevelWidgets().isEmpty()) {
        window = QApplication::activeWindow();
    }

    KPIMIdentities::IdentityManager identities(true);
    auto emails = QStringList();
    std::transform(identities.begin(), identities.end(),
                   std::back_inserter(emails),
                   Utils::mem_fn(&KPIMIdentities::Identity::fullEmailAddr));
    const auto defaultIndex = emails.indexOf(identities.defaultIdentity().fullEmailAddr());
    const auto email = QInputDialog::getItem(window,
                                             QObject::tr("Choose an identity"),
                                             QObject::tr("Choose the identity to use for the groupware message"),
                                             emails,
                                             defaultIndex,
                                             false);

    if (!email.isEmpty()) {
        todo->setOrganizer(email);
        m_itip->sendiTIPMessage(KCalCore::iTIPRequest, todo, window);
    }
}
