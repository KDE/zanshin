/* This file is part of Zanshin

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
