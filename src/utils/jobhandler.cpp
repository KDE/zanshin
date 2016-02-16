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

public slots:
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
    foreach (auto *job, jobs.keys()) {
        QObject::disconnect(job, 0, self, 0);
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
