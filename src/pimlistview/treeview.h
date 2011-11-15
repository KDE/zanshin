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


#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <Akonadi/EntityTreeView>
#include <QPoint>
#include <akonadi/item.h>

#include <QHeaderView>

class QMouseEvent;
class KXMLGUIClient;

class TreeView : public Akonadi::EntityTreeView
{
    Q_OBJECT
public:
    TreeView(KXMLGUIClient *guiClient, QWidget *parent);
    TreeView(QWidget *parent);
    virtual ~TreeView();

    Akonadi::Item currentItem();
    QList<Akonadi::Item> selectedItems();
    virtual void setModel(QAbstractItemModel* model);

protected:
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent( QMouseEvent* event );
    virtual void mouseDoubleClickEvent ( QMouseEvent* event );
    virtual void contextMenuEvent(QContextMenuEvent* event);

    virtual void rowsInserted(const QModelIndex& parent, int start, int end);

    virtual void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const;
    virtual void drawRow(QPainter* painter, const QStyleOptionViewItem& options, const QModelIndex& index) const;

    //virtual void resizeEvent(QResizeEvent* event);

private slots:
    void expandToplevel();
    void sectionWasClicked(int);

private:
    void recursiveExpand(const QModelIndex &parent = QModelIndex());
    QPoint m_dragStartPosition;
    KXMLGUIClient *m_guiClient;
};

class TreeHeaderView: public QHeaderView
{
    Q_OBJECT
public:
    explicit TreeHeaderView(Qt::Orientation orientation, QWidget* parent = 0);

    virtual void setModel(QAbstractItemModel* model);

protected slots:
    virtual void resizeEvent(QResizeEvent* event);
    void headerDataChanged(Qt::Orientation,int,int);

private:
    void resizeSections(int width);
};

#endif // TREEVIEW_H
