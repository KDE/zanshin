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

private slots:
    void handleJobResult(KJob *job)
    {
        Q_ASSERT(m_handlers.contains(job) || m_handlersWithJob.contains(job));

        for (auto handler : m_handlers.take(job)) {
            handler();
        }

        for (auto handler : m_handlersWithJob.take(job)) {
            handler(job);
        }
    }

public:
    QHash<KJob *, QList<JobHandler::ResultHandler>> m_handlers;
    QHash<KJob *, QList<JobHandler::ResultHandlerWithJob>> m_handlersWithJob;
};

Q_GLOBAL_STATIC(JobHandlerInstance, jobHandlerInstance)

void JobHandler::install(KJob *job, const ResultHandler &handler)
{
    auto self = jobHandlerInstance();
    QObject::connect(job, SIGNAL(result(KJob*)), self, SLOT(handleJobResult(KJob*)), Qt::UniqueConnection);
    self->m_handlers[job] << handler;
    job->start();
}

void JobHandler::install(KJob *job, const ResultHandlerWithJob &handler)
{
    auto self = jobHandlerInstance();
    QObject::connect(job, SIGNAL(result(KJob*)), self, SLOT(handleJobResult(KJob*)), Qt::UniqueConnection);
    self->m_handlersWithJob[job] << handler;
    job->start();
}

#include "jobhandler.moc"
