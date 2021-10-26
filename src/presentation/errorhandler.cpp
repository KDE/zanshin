/*
 * SPDX-FileCopyrightText: 2014 Mario Bensi <mbensi@ipsquad.net>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "errorhandler.h"

#include <QCoreApplication>
#include <KJob>

#include "utils/jobhandler.h"
#include "utils/mem_fn.h"

using namespace Presentation;

ErrorHandler::~ErrorHandler()
{
}

void ErrorHandler::installHandler(KJob *job, const QString &message)
{
    auto resultHandler = std::function<void()>(std::bind(Utils::mem_fn(&ErrorHandler::displayMessage),
                                               this, job, message));
    Utils::JobHandler::install(job, resultHandler);
}

void ErrorHandler::displayMessage(KJob *job, const QString &message)
{
    if (job->error() != KJob::NoError) {
        doDisplayMessage(QCoreApplication::translate("ErrorHandler", "%1: %2").arg(message, job->errorString()));
    }
}
