/* This file is part of Zanshin

   Copyright 2014-2016 Kevin Ottens <ervin@kde.org>

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

#include <QApplication>
#include <QPainter>
#include <QStyleOptionViewItem>

#include <KLocalizedString>

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
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.features = QStyleOptionViewItem::HasCheckIndicator;
    opt.text += ' ' + QLocale().dateFormat(QLocale::ShortFormat).toUpper() + ' ';
    return QStyledItemDelegate::sizeHint(opt, index);
}

void ItemDelegate::paint(QPainter *painter,
                         const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    const auto data = index.data(Presentation::QueryTreeModelBase::ObjectRole);

    auto task = Domain::Task::Ptr();
    auto artifact = data.value<Domain::Artifact::Ptr>();
    if (artifact) {
        task = artifact.dynamicCast<Domain::Task>();
    } else {
        task = data.value<Domain::Task::Ptr>();
        auto note = data.value<Domain::Note::Ptr>();
        artifact = task ? task.staticCast<Domain::Artifact>()
                        : note.staticCast<Domain::Artifact>();
    }

    auto opt = QStyleOptionViewItem(option);
    initStyleOption(&opt, index);
    const auto widget = opt.widget;
    const auto style = widget ? widget->style() : QApplication::style();

    const auto isDone = task ? task->isDone() : false;
    const auto isEnabled = (opt.state & QStyle::State_Enabled);
    const auto isActive = (opt.state & QStyle::State_Active);
    const auto isSelected = (opt.state & QStyle::State_Selected);
    const auto isEditing = (opt.state & QStyle::State_Editing);

    const auto startDate = task ? task->startDate() : QDateTime();
    const auto dueDate = task ? task->dueDate() : QDateTime();

    const auto onStartDate = startDate.isValid() && startDate.date() <= QDate::currentDate();
    const auto pastDueDate = dueDate.isValid() && dueDate.date() < QDate::currentDate();
    const auto onDueDate = dueDate.isValid() && dueDate.date() == QDate::currentDate();

    const auto taskDelegate = task ? task->delegate() : Domain::Task::Delegate();

    const auto baseFont = opt.font;
    const auto summaryFont = [=] {
        auto font = baseFont;
        font.setStrikeOut(isDone);
        font.setBold(!isDone && (onStartDate || onDueDate || pastDueDate));
        font.setItalic(taskDelegate.isValid());
        return font;
    }();
    const auto summaryMetrics = QFontMetrics(summaryFont);

    const auto colorGroup = (isEnabled && !isActive) ? QPalette::Inactive
                          : isEnabled ? QPalette::Normal
                          : QPalette::Disabled;
    const auto colorRole = (isSelected && !isEditing) ? QPalette::HighlightedText : QPalette::Text;

    const auto baseColor = opt.palette.color(colorGroup, colorRole);
    const auto summaryColor = isDone ? baseColor
                            : pastDueDate ? QColor(Qt::red)
                            : onDueDate ? QColor("orange")
                            : baseColor;

    const auto summaryText = taskDelegate.isValid() ? i18n("(%1) %2", taskDelegate.display(), opt.text) : opt.text;
    const auto dueDateText = dueDate.isValid() ? QLocale().toString(dueDate.date(), QLocale::ShortFormat)
                                               : QString();

    const auto textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, 0, widget) + 1;
    const auto dueDateWidth = dueDate.isValid() ? (summaryMetrics.width(dueDateText) + 2 * textMargin) : 0;


    const auto checkRect = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, widget);
    const auto summaryRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, widget)
                             .adjusted(textMargin, 0, -dueDateWidth - textMargin, 0);
    const auto dueDateRect = opt.rect.adjusted(opt.rect.width() - dueDateWidth, 0, 0, 0);


    // Draw background
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, widget);

    // Draw the check box
    if (task) {
        auto checkOption = opt;
        checkOption.rect = checkRect;
        checkOption.state = option.state & ~QStyle::State_HasFocus;
        checkOption.state |= isDone ? QStyle::State_On : QStyle::State_Off;
        style->drawPrimitive(QStyle::PE_IndicatorViewItemCheck, &checkOption, painter, widget);
    }

    // Draw the summary
    if (!summaryText.isEmpty()) {
        painter->setPen(summaryColor);
        painter->setFont(summaryFont);
        painter->drawText(summaryRect, Qt::AlignVCenter,
                          summaryMetrics.elidedText(summaryText, Qt::ElideRight, summaryRect.width()));
    }

    // Draw the due date
    if (!dueDateText.isEmpty()) {
        painter->drawText(dueDateRect, Qt::AlignCenter, dueDateText);
    }
}
