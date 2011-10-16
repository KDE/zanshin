/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) <year>  <name of author>

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


#ifndef PIMITEMDELEGATE_H
#define PIMITEMDELEGATE_H

#include <QStyledItemDelegate>

class QTreeView;

/**
 * Background color for todos,
 * editors,
 * special design for section parents
 */
class PimItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit PimItemDelegate(QTreeView* view, QObject* parent = 0);

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
    QTreeView *m_treeView;
    
};

#endif // PIMITEMDELEGATE_H
