/*
 * SPDX-FileCopyrightText: 2014-2016 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "itemdelegate.h"

#include <QApplication>
#include <QPainter>
#include <QStyleOptionViewItem>

#include <KLocalizedString>

#include "domain/task.h"
#include "presentation/querytreemodelbase.h"
#include "utils/datetime.h"

using namespace Widgets;

ItemDelegate::ItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    // Make sure they all get the height needed for a check indicator
    opt.features = QStyleOptionViewItem::HasCheckIndicator;
    // and for a date on the right
    opt.text += ' ' + QLocale().dateFormat(QLocale::ShortFormat).toUpper() + ' ';
    QSize sz = QStyledItemDelegate::sizeHint(opt, index);
    const auto projectInfo = index.data(Presentation::QueryTreeModelBase::ProjectRole);
    const auto dataSourceInfo = index.data(Presentation::QueryTreeModelBase::DataSourceRole);
    const auto contextListInfo = index.data(Presentation::QueryTreeModelBase::ContextListRole);
    const auto hasAdditionalInfo = projectInfo.isValid() || dataSourceInfo.isValid() || contextListInfo.isValid();
    if (hasAdditionalInfo)
        sz.rheight() += opt.fontMetrics.height();
    return sz;
}

void ItemDelegate::paint(QPainter *painter,
                         const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    const auto data = index.data(Presentation::QueryTreeModelBase::ObjectRole);
    auto task = data.value<Domain::Task::Ptr>();

    auto opt = QStyleOptionViewItem(option);
    initStyleOption(&opt, index);
    const auto widget = opt.widget;
    const auto style = widget ? widget->style() : QApplication::style();

    const auto isDone = task ? task->isDone() : false;
    const auto isEnabled = (opt.state & QStyle::State_Enabled);
    const auto isActive = (opt.state & QStyle::State_Active);
    const auto isSelected = (opt.state & QStyle::State_Selected);
    const auto isEditing = (opt.state & QStyle::State_Editing);

    const auto startDate = task ? task->startDate() : QDate();
    const auto dueDate = task ? task->dueDate() : QDate();
    const auto projectInfo = index.data(Presentation::QueryTreeModelBase::ProjectRole);
    const auto dataSourceInfo = index.data(Presentation::QueryTreeModelBase::DataSourceRole);
    const auto contextListInfo = index.data(Presentation::QueryTreeModelBase::ContextListRole);
    const auto hasAdditionalInfo = projectInfo.isValid() || dataSourceInfo.isValid() || contextListInfo.isValid();

    const auto currentDate = Utils::DateTime::currentDate();
    const auto onStartDate = startDate.isValid() && startDate <= currentDate;
    const auto pastDueDate = dueDate.isValid() && dueDate < currentDate;
    const auto onDueDate = dueDate.isValid() && dueDate == currentDate;

    const auto baseFont = opt.font;
    const auto summaryFont = [=] {
        auto font = baseFont;
        font.setStrikeOut(isDone);
        font.setBold(!isDone && (onStartDate || onDueDate || pastDueDate));
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

    const auto summaryText = opt.text;
    const auto dueDateText = dueDate.isValid() ? QLocale().toString(dueDate, QLocale::ShortFormat)
                                               : QString();

    const auto textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, widget) + 1;
    const auto dueDateWidth = dueDate.isValid() ? (summaryMetrics.horizontalAdvance(dueDateText) + 2 * textMargin) : 0;


    const auto checkRect = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, widget);
    const auto textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, widget)
                             .adjusted(textMargin, 0, - textMargin, 0);
    auto summaryRect = textRect.adjusted(0, 0, -dueDateWidth, 0);
    if (hasAdditionalInfo)
        summaryRect.setHeight(summaryRect.height() - opt.fontMetrics.height());
    auto dueDateRect = textRect.adjusted(textRect.width() - dueDateWidth, 0, 0, 0);
    dueDateRect.setHeight(summaryRect.height());

    const auto additionalInfoRect = QRect(textRect.x(), summaryRect.bottom(), textRect.width(), textRect.height() - summaryRect.height());

    // Draw background
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, widget);

    // Draw the check box
    if (task) {
        auto checkOption = opt;
        checkOption.rect = checkRect;
        checkOption.state = option.state & ~QStyle::State_HasFocus;
        checkOption.state |= isDone ? QStyle::State_On : QStyle::State_Off;
        style->drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &checkOption, painter, widget);
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

    // Draw the second line
    if (hasAdditionalInfo) {
        const auto additionalInfo = projectInfo.isValid() && !projectInfo.toString().isEmpty() ? i18n("Project: %1", projectInfo.toString())
                                  : dataSourceInfo.isValid() ? dataSourceInfo.toString()
                                  : i18n("Inbox");

        QFont additionalInfoFont = baseFont;
        additionalInfoFont.setItalic(true);
        additionalInfoFont.setPointSize(additionalInfoFont.pointSize() - 1);
        painter->setFont(additionalInfoFont);
        painter->drawText(additionalInfoRect, Qt::AlignLeft, additionalInfo);
    }
}

QWidget *ItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    m_editingState = EditingState::JustCreatedEditor;
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void ItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    m_editingState = EditingState::NotEditing;
    QStyledItemDelegate::setModelData(editor, model, index);
}

void ItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    // The first call gets through (initial value)
    // And later call is rejected, the user's own value takes precedence
    if (m_editingState == EditingState::JustCreatedEditor) {
        m_editingState = EditingState::Editing;
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

#include "moc_itemdelegate.cpp"
