/*
 * SPDX-FileCopyrightText: 2014 Mario Bensi <mbensi@ipsquad.net>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "compositejob.h"

using namespace Utils;

CompositeJob::CompositeJob(QObject *parent)
    : KCompositeJob(parent)
{

}

void CompositeJob::start()
{
    if (hasSubjobs()) {
        subjobs().first()->start();
    } else {
        emitResult();
    }
}

bool CompositeJob::install(KJob *job, const JobHandler::ResultHandlerWithJob &handler)
{
    if (!addSubjob(job))
        return false;

    JobHandler::install(job, handler);
    return true;
}

bool CompositeJob::install(KJob *job, const JobHandler::ResultHandler &handler)
{
    JobHandler::install(job, handler);

    if (!addSubjob(job))
        return false;

    return true;
}

void CompositeJob::emitError(const QString &errorText)
{
    setError(KJob::UserDefinedError);
    setErrorText(errorText);
    emitResult();
}

void CompositeJob::slotResult(KJob *job)
{
    if (job->error()) {
        KCompositeJob::slotResult(job);
    } else {
        removeSubjob(job);
        if (!hasSubjobs())
            emitResult();
    }
}
