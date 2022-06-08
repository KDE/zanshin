/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef AKONADI_MONITORIMPL_H
#define AKONADI_MONITORIMPL_H

#include "akonadimonitorinterface.h"
#include <Akonadi/Item>

namespace Akonadi {

class Monitor;

class MonitorImpl : public MonitorInterface
{
    Q_OBJECT
public:
    MonitorImpl();
    virtual ~MonitorImpl();

private Q_SLOTS:
    void onCollectionChanged(const Akonadi::Collection &collection, const QSet<QByteArray> &parts);

private:
    bool hasSupportedMimeTypes(const Collection &collection);
    Akonadi::Monitor *m_monitor;
};

}

#endif // AKONADI_MONITORIMPL_H
