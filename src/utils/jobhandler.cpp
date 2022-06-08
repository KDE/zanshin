/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "jobhandler.h"

#include <QHash>
#include <QObject>

#include <KJob>

using namespace Utils;

class JobHandlerInstance : public QObject
{
    Q_OBJECT
public:
    JobHandlerInstance()
        : QObject() {}

public Q_SLOTS:
    void handleJobResult(KJob *job)
    {
        Q_ASSERT(m_handlers.contains(job) || m_handlersWithJob.contains(job));

        foreach (const auto &handler, m_handlers.take(job)) {
            handler();
        }

        foreach (const auto &handler, m_handlersWithJob.take(job)) {
            handler(job);
        }
    }

    void onDestroyed(QObject *o)
    {
        auto job = static_cast<KJob*>(o);
        Q_ASSERT(job);
        m_handlers.remove(job);
        m_handlersWithJob.remove(job);
    }

public:
    QHash<KJob *, QList<JobHandler::ResultHandler>> m_handlers;
    QHash<KJob *, QList<JobHandler::ResultHandlerWithJob>> m_handlersWithJob;
};

Q_GLOBAL_STATIC(JobHandlerInstance, jobHandlerInstance)

void JobHandler::install(KJob *job, const ResultHandler &handler, StartMode startMode)
{
    auto self = jobHandlerInstance();
    QObject::connect(job, &KJob::result, self, &JobHandlerInstance::handleJobResult, Qt::UniqueConnection);
    QObject::connect(job, &KJob::destroyed, self, &JobHandlerInstance::onDestroyed, Qt::UniqueConnection);
    self->m_handlers[job] << handler;
    if (startMode == AutoStart)
        job->start();
}

void JobHandler::install(KJob *job, const ResultHandlerWithJob &handler, StartMode startMode)
{
    auto self = jobHandlerInstance();
    QObject::connect(job, &KJob::result, self, &JobHandlerInstance::handleJobResult, Qt::UniqueConnection);
    QObject::connect(job, &KJob::destroyed, self, &JobHandlerInstance::onDestroyed, Qt::UniqueConnection);
    self->m_handlersWithJob[job] << handler;
    if (startMode == AutoStart)
        job->start();
}

template<typename ResultHandler>
void clearJobs(JobHandlerInstance *self, QHash<KJob*, QList<ResultHandler>> &jobs)
{
    for (auto it = jobs.cbegin(); it != jobs.cend(); ++it) {
        QObject::disconnect(it.key(), 0, self, 0);
    }
    jobs.clear();
}

void JobHandler::clear()
{
    auto self = jobHandlerInstance();
    clearJobs(self, self->m_handlers);
    clearJobs(self, self->m_handlersWithJob);
}

int JobHandler::jobCount()
{
    auto self = jobHandlerInstance();
    return self->m_handlers.size() + self->m_handlersWithJob.size();
}

#include "jobhandler.moc"
