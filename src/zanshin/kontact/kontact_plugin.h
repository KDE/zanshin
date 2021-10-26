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
    Plugin(KontactInterface::Core *core, const QVariantList &);

    int weight() const override { return 449; }

protected:
#if KONTACTINTERFACE_VERSION >= QT_VERSION_CHECK(5, 14, 42)
    KParts::Part *createPart() override;
#else
    KParts::ReadOnlyPart *createPart() override;
#endif
};

#endif

