/* This file is part of Zanshin Todo.

   Copyright 2011 Kevin Ottens <ervin@kde.org>

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

#include <KDE/Akonadi/CollectionFetchJob>
#include <KDE/Akonadi/CollectionFetchScope>
#include <KDE/Akonadi/Item>
#include <KDE/Akonadi/ItemCreateJob>

#include <KDE/KCalCore/Todo>

#include <KDE/KDebug>
#include <KDE/KIcon>
#include <KDE/KLocale>

ZanshinRunner::ZanshinRunner(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    setObjectName(QLatin1String("Zanshin"));
    setIgnoredTypes(Plasma::RunnerContext::Directory | Plasma::RunnerContext::File |
                    Plasma::RunnerContext::NetworkLocation | Plasma::RunnerContext::Help);

    connect(this, SIGNAL(prepare()), this, SLOT(prep()));
    connect(this, SIGNAL(teardown()), this, SLOT(down()));
}

ZanshinRunner::~ZanshinRunner()
{
}

void ZanshinRunner::match(Plasma::RunnerContext &context)
{
    const QString command = context.query().trimmed();

    if (!command.startsWith("todo:", Qt::CaseInsensitive)) {
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
    match.setIcon(KIcon("office-calendar"));
    match.setText(i18n("Add \"%1\" to your todo list", summary));
    match.setRelevance(1.0);

    matches << match;
    context.addMatches(context.query(), matches);
}

void ZanshinRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)

    Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(),
                                                                       Akonadi::CollectionFetchJob::Recursive);
    job->fetchScope().setContentMimeTypes(QStringList() << "application/x-vnd.akonadi.calendar.todo");
    job->exec();

    Akonadi::Collection::List cols = job->collections();

    if (cols.isEmpty()) {
        return;
    }

    Akonadi::Collection collection;

    KConfig zanshin("zanshinrc");
    KConfigGroup config(&zanshin, "General");

    qint64 defaultCollectionId = config.readEntry("defaultCollection", -1);

    if (defaultCollectionId > 0) {
        foreach (Akonadi::Collection col, cols) {
            if (col.id() == defaultCollectionId) {
                collection = col;
                break;
            }
        }
    }

    if (!collection.isValid()) {
        collection = cols.first();
    }

    KCalCore::Todo::Ptr todo(new KCalCore::Todo);
    todo->setSummary(match.data().toString());

    Akonadi::Item item;
    item.setMimeType("application/x-vnd.akonadi.calendar.todo");
    item.setPayload<KCalCore::Todo::Ptr>(todo);

    new Akonadi::ItemCreateJob(item, collection);
}

#include "zanshinrunner.moc"
