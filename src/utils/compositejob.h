/*
 * SPDX-FileCopyrightText: 2014 Mario Bensi <mbensi@ipsquad.net>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef COMPOSITE_JOB
#define COMPOSITE_JOB

#include "kcompositejob.h"
#include "jobhandler.h"

namespace Utils {

class CompositeJob : public KCompositeJob
{
    Q_OBJECT
public:
    explicit CompositeJob(QObject *parent = nullptr);

    using KCompositeJob::addSubjob;

    void start() override;
    virtual bool install(KJob *job, const JobHandler::ResultHandlerWithJob &handler);
    virtual bool install(KJob *job, const JobHandler::ResultHandler &handler);

    void emitError(const QString &errorText);

private slots:
    void slotResult(KJob *job) override;
};

}
#endif
