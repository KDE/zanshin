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

#ifndef AKONADI_MONITORIMPL_H
#define AKONADI_MONITORIMPL_H

#include "akonadimonitorinterface.h"
#include <AkonadiCore/Akonadi/Item>

namespace Akonadi {

class Monitor;

class MonitorImpl : public MonitorInterface
{
    Q_OBJECT
public:
    MonitorImpl();
    virtual ~MonitorImpl();

private slots:
    void onCollectionChanged(const Akonadi::Collection &collection, const QSet<QByteArray> &parts);

private:
    bool hasSupportedMimeTypes(const Collection &collection);
    Akonadi::Monitor *m_monitor;
};

}

#endif // AKONADI_MONITORIMPL_H
