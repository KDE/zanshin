/* This file is part of Zanshin Todo.

   Copyright 2011 Christian Mollekopf <chrigi_1@fastmail.fm>

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

#ifndef ITEMEDITOR_H
#define ITEMEDITOR_H

#include "ui_itemview.h"
#include "attachmentsviewer.h"
#include <QFrame>
#include <KXMLGUIClient>

#include <Akonadi/Item>

#include "core/pimitem.h"
#include "core/pimitemmonitor.h"

namespace Nepomuk2 {
class Resource;
}

class ItemContext;
class QFocusEvent;

class KXMLGUIClient;

class QTimer;

namespace Ui {
    class tags;
    class properties;
}

/**
 * The editor part for editing notes/todos/events
 */
class ItemEditor : public QFrame, private Ui_itemView, public KXMLGUIClient
{
    Q_OBJECT
public:
    explicit ItemEditor(QWidget* parent, KXMLGUIClient *parentClient);
    virtual ~ItemEditor();

public slots:
    /**
     * Set new Akonadi::Item in the viewer, this function also stores the (changed) content to the old item
     *
     */
    void setItem(const Akonadi::Item &);

    /**
     * This will fetch the given item first, before setting it on the editor
     */
    void setItem(const KUrl &);
    
private slots:
    ///add tag from tag edit
    void itemsReceived(const Akonadi::Item::List&);
    void saveItem();
    void updateContent(PimItemMonitor::ChangedParts parts = PimItemMonitor::AllParts);
    void itemRemoved();
    void clearView();

    void setDueDate(KDateTime, bool);

    void setEventDate(KDateTime);

    void autosave();
    
    void setFullscreenEditor();
    void restoreState();

signals:
    void itemChanged();
private:

    PimItemMonitor *m_itemMonitor;
    PimItem::Ptr m_currentItem;
    QTimer *m_autosaveTimer;
    int m_autosaveTimeout;
    Ui::properties *ui_properties;
    AttachmentsViewer *m_attachmentsViewer;

};

#endif // ITEMVIEWER_H
