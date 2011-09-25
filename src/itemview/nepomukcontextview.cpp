/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2011  Christian Mollekopf <chrigi_1@fastmail.fm>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "nepomukcontextview.h"

#include <Nepomuk/Utils/ResourceModel>
#include <Nepomuk/Resource>
#include <Nepomuk/Variant>

#include <QContextMenuEvent>
#include <QMenu>

#include <kxmlguiclient.h>
#include <kxmlguifactory.h>
#include <kactioncollection.h>

NepomukContextView::NepomukContextView(KXMLGUIClient *client, QWidget* parent)
:   QTreeView(parent),
    m_guiClient(client)
{

}


void NepomukContextView::contextMenuEvent(QContextMenuEvent *event)
{
    QAbstractScrollArea::contextMenuEvent(event);
    if ( !m_guiClient || !model())
        return;

    const QModelIndex index = indexAt( event->pos() );

    const Nepomuk::Resource res = Nepomuk::Variant(model()->data( index,  Nepomuk::Utils::ResourceModel::ResourceRole)).toResource();

    if (!res.isValid()) {
        return;
    }

    QMenu *popup = 0;


    /*
     * -remove Relation
     * -openFile/activateItem
     * -open item in ext. editor
     * -add File
     */
    popup = static_cast<QMenu*>( m_guiClient->factory()->container(
        QLatin1String( "nepomuk_contextview_contextmenu" ), m_guiClient ) );

    /*KActionCollection *actionCollection = m_guiClient->actionCollection();
    QAction *action = actionCollection->action("fileOpen");
    if (res.isFile()) {
        action->setEnabled(true);
    } else {
        action->setEnabled(false);
    }
    action = actionCollection->action("delete_Note");
    if (item.isValid()) {
        action->setEnabled(true);
    } else {
        action->setEnabled(false);
    }*/

    if ( popup ) {
        popup->exec( event->globalPos() );
    }
}

QList< Nepomuk::Resource > NepomukContextView::selectedResources()
{
    QList< Nepomuk::Resource > resourceList;
    foreach (const QModelIndex &i ,selectionModel()->selectedIndexes()) {
        if (i.column() != 0) {
            continue;
        }
        resourceList.append(Nepomuk::Variant(model()->data(i,  Nepomuk::Utils::ResourceModel::ResourceRole)).toResource());
    }
    return resourceList;
}


