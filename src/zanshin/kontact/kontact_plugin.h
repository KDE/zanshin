/*
 * SPDX-FileCopyrightText: 2011 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef ZANSHIN_KONTACT_PLUGIN_H
#define ZANSHIN_KONTACT_PLUGIN_H

#include <KontactInterface/Plugin>
#include <kontactinterface_version.h>

class Plugin : public KontactInterface::Plugin
{
    Q_OBJECT

public:
    Plugin(KontactInterface::Core *core, const KPluginMetaData &data, const QVariantList &);

    int weight() const override { return 449; }

protected:
    KParts::Part *createPart() override;
};

#endif

