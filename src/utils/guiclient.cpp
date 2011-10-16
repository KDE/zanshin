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


#include "guiclient.h"

#include <kxmlguibuilder.h>
#include <kxmlguifactory.h>

#include <QWidget>


GuiClient::GuiClient(const QString &xmlfile, QObject *parent)
: QObject(parent),
KXMLGUIClient(),
m_builder(0),
m_factory(0)
{
    setXMLFile(xmlfile);
}

GuiClient::~GuiClient()
{
    if (m_factory) {
        m_factory->removeClient(this);
        m_factory->deleteLater();
    }
    if (m_builder) {
        delete m_builder;
    }
}


void GuiClient::setupActions(QWidget* widget)
{
    m_builder = new KXMLGUIBuilder(widget);
    m_factory = new KXMLGUIFactory(m_builder);
    m_factory->addClient(this);
}
