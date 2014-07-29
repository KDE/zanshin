/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>

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


#include "itemdelegate.h"

#include <QStyleOptionViewItemV4>

#include "domain/note.h"
#include "domain/task.h"
#include "presentation/querytreemodelbase.h"

using namespace Widgets;

ItemDelegate::ItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    // Make sure they all get the height needed for a check indicator
    QStyleOptionViewItemV4 opt = option;
    opt.features = QStyleOptionViewItemV4::HasCheckIndicator;
    QSize res = QStyledItemDelegate::sizeHint(opt, index);
    return res;
}

void ItemDelegate::paint(QPainter *painter,
                         const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    QStyleOptionViewItemV4 opt = option;

    Domain::Task::Ptr task;
    Domain::Note::Ptr note;

    QVariant data = index.data(Presentation::QueryTreeModelBase::ObjectRole);
    auto artifact = data.value<Domain::Artifact::Ptr>();
    if (artifact) {
        task = artifact.dynamicCast<Domain::Task>();
        note = artifact.dynamicCast<Domain::Note>();
    } else {
        task = data.value<Domain::Task::Ptr>();
        note = data.value<Domain::Note::Ptr>();
    }

    if (task) {
        if (task->isDone()) {
            opt.font.setStrikeOut(true);
        } else {
            if (task->startDate().isValid()
             && task->startDate().date() <= QDate::currentDate()) {
                opt.font.setBold(true);
            }

            if (task->dueDate().isValid()) {
                if (task->dueDate().date() < QDate::currentDate()) {
                    opt.font.setBold(true);
                    opt.palette.setColor(QPalette::Text, QColor(Qt::red));
                    opt.palette.setColor(QPalette::HighlightedText, QColor(Qt::red));

                } else if (task->dueDate().date() == QDate::currentDate()) {
                    opt.font.setBold(true);
                    opt.palette.setColor(QPalette::Text, QColor("orange"));
                    opt.palette.setColor(QPalette::HighlightedText, QColor("orange"));
                }
            }
        }
    }

    if (note) {
        opt.features |= QStyleOptionViewItemV4::HasDecoration;
        opt.icon = QIcon::fromTheme("text-plain");
    }

    QStyledItemDelegate::paint(painter, opt, index);
}
