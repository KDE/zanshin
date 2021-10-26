/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "akonadimonitorinterface.h"

using namespace Akonadi;

MonitorInterface::MonitorInterface(QObject *parent)
    : QObject(parent)
{
}

MonitorInterface::~MonitorInterface()
{
}
