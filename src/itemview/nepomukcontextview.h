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


#ifndef NEPOMUKCONTEXTVIEW_H
#define NEPOMUKCONTEXTVIEW_H

#include <QTreeView>

#include <QList>
#include <Nepomuk/Resource>

class QContextMenuEvent;
class KXMLGUIClient;

class NepomukContextView : public QTreeView
{
    Q_OBJECT
public:
    explicit NepomukContextView(KXMLGUIClient *client, QWidget *parent = 0);
    QList <Nepomuk::Resource> selectedResources();
protected:
    virtual void contextMenuEvent(QContextMenuEvent* );

private:
    KXMLGUIClient *m_guiClient;
};

#endif // NEPOMUKCONTEXTVIEW_H
