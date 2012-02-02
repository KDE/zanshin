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


#include "treeview.h"

#include <QMouseEvent>
#include <QApplication>
#include <QHeaderView>

#include <Akonadi/EntityTreeModel>
#include <KIconLoader>
#include <KConfigGroup>
#include <KXMLGUIClient>
#include <KXMLGUIFactory>

#include <QMenu>

#include "pimitem.h"
#include <KActionCollection>
#include <KAction>

#include "pimitemdelegate.h"
#include <qvarlengtharray.h>
#include <QTimer>

TreeHeaderView::TreeHeaderView(Qt::Orientation orientation, QWidget* parent): QHeaderView(orientation, parent)
{
    setResizeMode(QHeaderView::Fixed);
    setStretchLastSection(false);
    //setMinimumSectionSize(200);
    setClickable(true);
}


void TreeHeaderView::resizeEvent(QResizeEvent* event)
{
    resizeSections(event->size().width());
    QAbstractItemView::resizeEvent(event);
}

void TreeHeaderView::setModel( QAbstractItemModel* model )
{
  QHeaderView::setModel( model );
  connect(model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)), this, SLOT(headerDataChanged(Qt::Orientation,int,int)));
}

void TreeHeaderView::headerDataChanged( Qt::Orientation , int first , int last )
{
    resizeSections(size().width());
}


void TreeHeaderView::resizeSections(int width)
{
    //TODO probably also not the nicest implementation for resizing the columns
    /*Unfortuantely I couldn't figure out how to resize only the first col, and keep the last two static,
    * so the title gets the maximum available space using resize to content or providing a proper size hint.
    * Therefore I do this manually now.
    * The sizehint is implemented in notetakermodel
    */
    const int minimumSize = 200;
    int size = width;
    for (int i = 0; i < count(); i++) {
        const int logicalIndex = visualIndex(i);
        if (logicalIndex < 0) {
            continue;
        }
        const int s = sectionSizeHint(logicalIndex);
        size -= s;
        resizeSection(logicalIndex, s);
    }
    if (size < minimumSize) {
        size = minimumSize;
    }
    resizeSection(0, size);

}



TreeView::TreeView(KXMLGUIClient *guiClient, QWidget *parent)
: Akonadi::EntityTreeView(guiClient, parent),
    m_guiClient(guiClient)
{
    setDragEnabled(true);
    setItemsExpandable(true);

    PimItemDelegate *delegate = new PimItemDelegate(this);
    setItemDelegate(delegate);

    QHeaderView *headerView = new TreeHeaderView(Qt::Horizontal, this);
    setHeader(headerView);
    connect(headerView, SIGNAL(sectionClicked(int)), this, SLOT(sectionWasClicked(int)));
    //header()->setCascadingSectionResizes(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setRootIsDecorated(false);
    setExpandsOnDoubleClick(true);    
}

TreeView::~TreeView()
{

}



void TreeView::rowsInserted(const QModelIndex& parent, int start, int end)
{
    QTreeView::rowsInserted(parent, start, end);
    if (!model()) {
        return;
    }
    if (model()->index(start, 0).data(Akonadi::EntityTreeModel::ItemRole).canConvert<Akonadi::Item>()) { //not a section
        return;
    }

    //expand sections
    if (!parent.isValid() && (start == end)) {
        expand(model()->index(start, 0));
    }
    //expand all new items
    /*for (int i = start; i < end; i++) {
        const QModelIndex &index = model()->index(i, 0, parent);
        expand(index);
    }*/
}

void TreeView::recursiveExpand(const QModelIndex &parent)
{
    for (int i = 0; i < model()->rowCount(parent); i++) {
        //kDebug() << "get index";
        const QModelIndex &index = model()->index(i, 0, parent);
        if (!index.data(Akonadi::EntityTreeModel::ItemRole).canConvert<Akonadi::Item>()) { //section
            //kDebug() << "expand : " << index << model()->rowCount(index);
            expand(index);
            Q_ASSERT(isExpanded(index));
            recursiveExpand(index);
        }
    }
}


void TreeView::expandToplevel()
{
    recursiveExpand();
    //expand toplevel items (sections)
    /*for (int i = 0; i < model()->rowCount(); i++) {
        //kDebug() << "get index";
        const QModelIndex &index = model()->index(i, 0);
        if (!index.data(Akonadi::EntityTreeModel::ItemRole).canConvert<Akonadi::Item>()) { //section
            //kDebug() << "expand : " << index << model()->rowCount(index);
            expand(index);
            Q_ASSERT(isExpanded(index));
        }
    }*/
}

void TreeView::sectionWasClicked(int index)
{
    //Setting the sort indicator, triggers a resort
    //kDebug() << "section clicked" << index;
    if (!header()->isSortIndicatorShown()) {
        //kDebug() << "show indicator";
        header()->setSortIndicatorShown(true);
        header()->setSortIndicator(index, Qt::AscendingOrder);
    } else {
        if (header()->sortIndicatorOrder() == Qt::DescendingOrder) { //FIXME although we should check for ascending, descending does the correct thing
            //kDebug() << "changed to descend";
            header()->setSortIndicator(index, Qt::DescendingOrder);
        } else {
            //kDebug() << "hide indicator";
            header()->setSortIndicator(-1, Qt::AscendingOrder); //Trigger the custom sorting, This works only with the NoteSortFilterProxyModel in the modelstack
            header()->setSortIndicatorShown(false);
        }
    }
}

void TreeView::setModel(QAbstractItemModel* model)
{
    disconnect(model, 0, this, 0);
    Akonadi::EntityTreeView::setModel(model);
    //kDebug() << model->rowCount();
    //expandToplevel(); //FIXME does not work when disabling/enabling the sectionmodel?
    connect(model, SIGNAL(modelReset()), this, SLOT(expandToplevel()));
    connect(model, SIGNAL(layoutChanged()), this, SLOT(expandToplevel()));
    QTimer::singleShot(0, this, SLOT(expandToplevel())); //For some reason a direct call does not work....
    sortByColumn(-1, Qt::AscendingOrder);
    header()->setSortIndicatorShown(false);
}


void TreeView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStartPosition = event->pos();
    }

    if (event->button() == Qt::RightButton) {
        return;
    }

    const QModelIndex &idx = indexAt(event->pos());
    //TODO multi selection with shift
    //FIXME doubleclick to expand items does not work
    if (event->modifiers() == Qt::CTRL) {
        selectionModel()->select(idx, QItemSelectionModel::Rows|QItemSelectionModel::Toggle);
    } else {
        selectionModel()->select(idx, QItemSelectionModel::Rows|QItemSelectionModel::ClearAndSelect); //Select row so standardactino work, but don't change item for d&d with currently activated item
    }
    //EntityTreeView::mousePressEvent(event);
}

void TreeView::mouseReleaseEvent( QMouseEvent* event )
{
    //By selecting in the release event, we ensure d&d works without activating another item
    if ((event->button() == Qt::LeftButton) && (event->modifiers() == Qt::NoModifier) ) {
        const QModelIndex &idx = indexAt(event->pos());
        selectionModel()->setCurrentIndex(idx, QItemSelectionModel::Rows|QItemSelectionModel::NoUpdate);
    }
    //EntityTreeView::mouseReleaseEvent(event);
}

void TreeView::mouseDoubleClickEvent ( QMouseEvent* event )
{
    const QModelIndex &idx = indexAt(event->pos());
    if (!idx.data(Akonadi::EntityTreeModel::ItemRole).canConvert<Akonadi::Item>()) { //section
            //kDebug() << "expand : " << index << model()->rowCount(index);
            setExpanded(idx, !isExpanded(idx));
        }
    QTreeView::mouseDoubleClickEvent ( event );
}


Akonadi::Item TreeView::currentItem()
{
    QModelIndex index =  currentIndex();
    const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
    if (!item.isValid()) {
        kWarning() << "invalid item";
    }
    return item;
}

QList<Akonadi::Item> TreeView::selectedItems()
{
    QList <Akonadi::Item> list;
    foreach (const QModelIndex &index, selectedIndexes()) {
        if (index.column() != 0) { //were only interested in the first column
            continue;
        }
        const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
        if (item.isValid()) {
            list << item;
        }
    }
    return list;
}


void TreeView::contextMenuEvent(QContextMenuEvent* event)
{

    if ( !m_guiClient || !model())
        return;

    const QModelIndex index = indexAt( event->pos() );
    QMenu *popup = 0;
        // check if the index under the cursor is a collection or item
        const Akonadi::Item item = model()->data( index,  Akonadi::EntityTreeModel::ItemRole ).value< Akonadi::Item>();
        if (!index.isValid() || item.isValid() ) {
            popup = static_cast<QMenu*>( m_guiClient->factory()->container(
                QLatin1String( "akonadi_itemview_contextmenu" ), m_guiClient ) );

            KActionCollection *actionCollection = m_guiClient->actionCollection();
            QAction *action = actionCollection->action("new_subTodo");
            if (item.isValid() && (AbstractPimItem::itemType(item) & AbstractPimItem::Todo)) {
                action->setEnabled(true);
            } else {
                action->setEnabled(false);
            }

            //TODO hide standard delete action when not in trash
            /*action = actionCollection->action("delete_Note");
            if (item.isValid()) {
                action->setEnabled(true);
            } else {
                action->setEnabled(false);
            }*/
        } else {
            popup = static_cast<QMenu*>( m_guiClient->factory()->container(
                QLatin1String( "akonadi_collectionview_contextmenu" ), m_guiClient ) );
        }

    if ( popup ) {
        popup->exec( event->globalPos() );
    }

    //Akonadi::EntityTreeView::contextMenuEvent(event);
}


void TreeView::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton)) {
        return;
    }
    if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
        return;
    }

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    AbstractPimItem *pimItem = 0;
    QList <QUrl> list;

    QModelIndex index = indexAt(m_dragStartPosition);


    //We're dragging a selection
    if (selectedIndexes().size() > 1 && selectedIndexes().contains(index)) {

        foreach (const QModelIndex &index, selectedIndexes()) {
            if (index.column() != 0) { //not interested in having all items once for each col
                continue;
            }
            const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
            if (item.isValid()) {
                if (!pimItem) {
                    pimItem = PimItemUtils::getItem(item);
                }
                if (!list.contains(item.url())) { //probably one item in selection for each column
                    list << item.url();
                } else {
                    kWarning() << "same item multiple times in selection";
                }
            }
        }
    } else { //We're dragging a single item
        const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
        if (!item.isValid()) {
            kDebug() << "invalid item";
        }
        list << item.url();
        pimItem = PimItemUtils::getItem(item);
    }

    if (!pimItem) {
        return;
    }
    kDebug() << "start drag: " << list;
    mimeData->setUrls(list);
    //mimeData->setData(pimItem->mimeType(),

    drag->setMimeData(mimeData);
    drag->setPixmap(SmallIcon(pimItem->getIconName()));

    Qt::DropAction dropAction = drag->exec(Qt::CopyAction| Qt::MoveAction);

}
/*

void TreeView::resizeEvent(QResizeEvent* event)
{
    header()->resizeSection();
    QAbstractItemView::resizeEvent(event);
}
*/

void TreeView::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
    QTreeView::drawBranches(painter, rect, index);
}

void TreeView::drawRow(QPainter* painter, const QStyleOptionViewItem& options, const QModelIndex& index) const
{
    QTreeView::drawRow(painter, options, index);
}
