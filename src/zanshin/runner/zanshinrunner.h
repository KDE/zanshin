/*
 * SPDX-FileCopyrightText: 2011 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2014 Mario Bensi <mbensi@ipsquad.net>
   * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
   */

#ifndef ZANSHINRUNNER_H
#define ZANSHINRUNNER_H

#include <KRunner/AbstractRunner>

#include "domain/taskrepository.h"

using namespace KRunner;

class ZanshinRunner : public AbstractRunner
{
    Q_OBJECT

public:
    ZanshinRunner(QObject *parent, const KPluginMetaData &metaData);

private:
    void match(RunnerContext &context) override;
    void run(const RunnerContext &context, const QueryMatch &action) override;

    Domain::TaskRepository::Ptr m_taskRepository;
    const QString m_triggerWord;
};

#endif
