/* This file is part of Zanshin

   Copyright 2015 Theo Vaucher <theo.vaucher@gmail.com>

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


#ifndef SCRIPTING_TASKACTION_H
#define SCRIPTING_TASKACTION_H

#include <QObject>

#include "domain/taskrepository.h"

#include "presentation/errorhandlingmodelbase.h"

namespace Scripting {

class TaskAction : public QObject, public Presentation::ErrorHandlingModelBase
{
    Q_OBJECT
public:
    explicit TaskAction(const Domain::TaskRepository::Ptr &taskRepository, QObject *parent = Q_NULLPTR);

public slots:

private:
    Domain::TaskRepository::Ptr m_taskRepository;
};

}

#endif // SCRIPTING_TASKACTION_H
