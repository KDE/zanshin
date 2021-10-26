/*
 * SPDX-FileCopyrightText: 2011 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "kontact_plugin.h"

#include <KontactInterface/Core>

#if KONTACTINTERFACE_VERSION < QT_VERSION_CHECK(5, 14, 42)
/**
  Exports Kontact plugin.
  @param pluginclass the class to instanciate (must derive from KontactInterface::Plugin
  @param jsonFile filename of the JSON file, generated from a .desktop file
 */
#define EXPORT_KONTACT_PLUGIN_WITH_JSON( pluginclass, jsonFile ) \
    class Instance                                           \
    {                                                        \
    public:                                                \
        static QObject *createInstance( QWidget *, QObject *parent, const QVariantList &list ) \
        { return new pluginclass( static_cast<KontactInterface::Core*>( parent ), list ); } \
    };                                                                    \
    K_PLUGIN_FACTORY_WITH_JSON( KontactPluginFactory, jsonFile, registerPlugin< pluginclass >   \
                              ( QString(), Instance::createInstance ); ) \
    K_EXPORT_PLUGIN_VERSION(KONTACT_PLUGIN_VERSION)
#endif

EXPORT_KONTACT_PLUGIN_WITH_JSON(Plugin, "zanshin_plugin.json")

Plugin::Plugin(KontactInterface::Core *core, const QVariantList&)
    : KontactInterface::Plugin(core, core, "zanshin")
{
    setComponentName(QStringLiteral("zanshin"), QStringLiteral("zanshin"));
}

#if KONTACTINTERFACE_VERSION >= QT_VERSION_CHECK(5, 14, 42)
KParts::Part *Plugin::createPart()
{
    return loadPart();
}
#else
KParts::ReadOnlyPart *Plugin::createPart()
{
    return loadPart();
}
#endif

#include "kontact_plugin.moc"
