/*
 * SPDX-FileCopyrightText: 2011 Kevin Ottens <ervin@kde.org>
 * SPDX-FileCopyrightText: 2014 Mario Bensi <mbensi@ipsquad.net>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "zanshinrunner.h"

#include "domain/task.h"
#include "akonadi/akonaditaskrepository.h"
#include "akonadi/akonadiserializer.h"
#include "akonadi/akonadistorage.h"

#include <QIcon>

#include <KConfig>
#include <KLocalizedString>

K_PLUGIN_CLASS_WITH_JSON(ZanshinRunner, "plasma-runner-zanshin.json")

Domain::TaskRepository::Ptr createTaskRepository()
{
    using namespace Akonadi;
    auto repository = new TaskRepository(StorageInterface::Ptr(new Storage),
                                         SerializerInterface::Ptr(new Serializer));
    return Domain::TaskRepository::Ptr(repository);
}

ZanshinRunner::ZanshinRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : AbstractRunner(parent, metaData, args)
    ,  m_taskRepository(createTaskRepository())
    , m_triggerWord(QStringLiteral("todo:"))
{
    setObjectName(QStringLiteral("Zanshin"));
    setTriggerWords({m_triggerWord});
}

ZanshinRunner::~ZanshinRunner()
{
}

void ZanshinRunner::match(RunnerContext &context)
{
    const QString command = context.query().trimmed();

    if (!command.startsWith(m_triggerWord)) {
        return;
    }

    const QString summary = command.mid(5).trimmed();

    if (summary.isEmpty()) {
        return;
    }

    QList<QueryMatch> matches;

    QueryMatch match(this);
    match.setData(summary);
    match.setType(QueryMatch::ExactMatch);
    match.setIcon(QIcon::fromTheme(QStringLiteral("zanshin")));
    match.setText(i18n("Add \"%1\" to your todo list", summary));
    match.setRelevance(1.0);

    matches << match;
    context.addMatches(matches);
}

void ZanshinRunner::run(const RunnerContext &context, const QueryMatch &match)
{
    Q_UNUSED(context)

    KConfig::setMainConfigName("zanshinrc");

    auto task = Domain::Task::Ptr::create();
    task->setTitle(match.data().toString());
    m_taskRepository->create(task);

    KConfig::setMainConfigName(QString());
}

#include "zanshinrunner.moc"
