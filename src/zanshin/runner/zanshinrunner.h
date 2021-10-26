/*
 * SPDX-FileCopyrightText: 2011 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2014 Mario Bensi <mbensi@ipsquad.net>
   * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
   */


#ifndef ZANSHINRUNNER_H
#define ZANSHINRUNNER_H

#include <KRunner/AbstractRunner>

#include "domain/taskrepository.h"

class ZanshinRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    ZanshinRunner(QObject *parent, const QVariantList &args);
    ~ZanshinRunner();

    void match(Plasma::RunnerContext &context) override;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &action) override;
private:
    Domain::TaskRepository::Ptr m_taskRepository;
};


#endif
