/*
 * SPDX-FileCopyrightText: 2011 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "kontact_plugin.h"

#include <KontactInterface/Core>

EXPORT_KONTACT_PLUGIN_WITH_JSON(Plugin, "zanshin_plugin.json")

Plugin::Plugin(KontactInterface::Core *core, const KPluginMetaData &data, const QVariantList&)
    : KontactInterface::Plugin(core, core, data, "zanshin")
{
    setComponentName(QStringLiteral("zanshin"), QStringLiteral("zanshin"));
}

KParts::Part *Plugin::createPart()
{
    return loadPart();
}

#include "kontact_plugin.moc"

#include "moc_kontact_plugin.cpp"
