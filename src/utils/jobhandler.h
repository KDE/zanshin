/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef UTILS_JOBHANDLER_H
#define UTILS_JOBHANDLER_H

#include <functional>

class KJob;

namespace Utils {

namespace JobHandler
{
    enum StartMode {
        AutoStart,
        ManualStart
    };

    typedef std::function<void(KJob*)> ResultHandlerWithJob;
    typedef std::function<void()> ResultHandler;

    void install(KJob *job, const ResultHandler &handler, StartMode startMode = AutoStart);
    void install(KJob *job, const ResultHandlerWithJob &handler, StartMode startMode = AutoStart);

    void clear();

    int jobCount();
}

}

#endif // UTILS_JOBHANDLER_H
