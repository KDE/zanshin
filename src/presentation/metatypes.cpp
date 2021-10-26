/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "metatypes.h"

#include "domain/datasource.h"
#include "domain/task.h"

using namespace Presentation;

void MetaTypes::registerAll()
{
    qRegisterMetaType<QAbstractItemModel*>();
    qRegisterMetaType<QObjectPtr>();
    qRegisterMetaType<QObjectPtrList>();
    qRegisterMetaType<Domain::Task::Ptr>();
    qRegisterMetaType<Domain::DataSource::Ptr>();
}
