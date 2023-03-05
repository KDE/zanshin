/*
 * SPDX-FileCopyrightText: 2011 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2014 Mario Bensi <mbensi@ipsquad.net>
   * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
   */


#ifndef ZANSHINRUNNER_H
#define ZANSHINRUNNER_H

#include <KRunner/AbstractRunner>

#include "domain/taskrepository.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
using namepace Plasma;
#else
using namespace KRunner;
#endif

class ZanshinRunner : public AbstractRunner
{
    Q_OBJECT

public:
    ZanshinRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~ZanshinRunner();

    void match(RunnerContext &context) override;
    void run(const RunnerContext &context, const QueryMatch &action) override;

  private:
    Domain::TaskRepository::Ptr m_taskRepository;
    const QString m_triggerWord;
};

#endif
