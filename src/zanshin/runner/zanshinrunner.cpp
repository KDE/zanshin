/* This file is part of Zanshin

   Copyright 2011 Kevin Ottens <ervin@kde.org>
   Copyright 2014 Mario Bensi <mbensi@ipsquad.net>

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

#include "zanshinrunner.h"

#include "domain/task.h"
#include "akonadi/akonaditaskrepository.h"
#include "akonadi/akonadiserializer.h"
#include "akonadi/akonadistorage.h"

#include <QIcon>

#include <KConfig>

K_EXPORT_PLASMA_RUNNER(zanshin, ZanshinRunner)
Domain::TaskRepository::Ptr createTaskRepository()
{
    using namespace Akonadi;
    auto repository = new TaskRepository(StorageInterface::Ptr(new Storage),
                                         SerializerInterface::Ptr(new Serializer),
                                         MessagingInterface::Ptr());
    return Domain::TaskRepository::Ptr(repository);
}

ZanshinRunner::ZanshinRunner(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args),
      m_taskRepository(createTaskRepository())
{
    setObjectName(QStringLiteral("Zanshin"));
    setIgnoredTypes(Plasma::RunnerContext::Directory | Plasma::RunnerContext::File |
                    Plasma::RunnerContext::NetworkLocation | Plasma::RunnerContext::Help);
}

ZanshinRunner::~ZanshinRunner()
{
}

void ZanshinRunner::match(Plasma::RunnerContext &context)
{
    const QString command = context.query().trimmed();

    if (!command.startsWith(QStringLiteral("todo:"), Qt::CaseInsensitive)) {
        return;
    }

    const QString summary = command.mid(5).trimmed();

    if (summary.isEmpty()) {
        return;
    }

    QList<Plasma::QueryMatch> matches;

    Plasma::QueryMatch match(this);
    match.setData(summary);
    match.setType(Plasma::QueryMatch::ExactMatch);
    match.setIcon(QIcon::fromTheme(QStringLiteral("zanshin")));
    match.setText(tr("Add \"%1\" to your todo list").arg(summary));
    match.setRelevance(1.0);

    matches << match;
    context.addMatches(matches);
}

void ZanshinRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)

    KConfig::setMainConfigName("zanshinrc");

    auto task = Domain::Task::Ptr::create();
    task->setTitle(match.data().toString());
    m_taskRepository->create(task);

    KConfig::setMainConfigName(QString());
}

#include "zanshinrunner.moc"
