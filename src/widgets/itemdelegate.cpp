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

namespace {
    // For the round rect around contexts
    constexpr int s_xMargin = 5;
    constexpr int s_yMargin = 2;
    constexpr int s_xSpacingAfterInfoText = 10;
    constexpr int s_xSpacingBetweenRoundRects = 5;
    constexpr int s_radius = 5;
}

ItemDelegate::ItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

// Shared data between sizeHint() and paint()
struct ItemDelegate::Layout {
    QVariant projectInfo;
    QVariant dataSourceInfo;
    QStringList contexts;
    int additionalInfoHeight = 0;
    QFont additionalInfoFont = {};

    bool showContexts() const { return !contexts.isEmpty(); }
    bool hasAdditionalInfo() const { return projectInfo.isValid() || dataSourceInfo.isValid() || showContexts(); }
    QString additionalInfoText() const {
        if (projectInfo.isValid() && !projectInfo.toString().isEmpty())
            return i18n("Project: %1", projectInfo.toString());
        if (dataSourceInfo.isValid())
            return dataSourceInfo.toString();
        return {};
    }
};

// Shared code between sizeHint() and paint()
ItemDelegate::Layout ItemDelegate::doLayout(const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
    Layout layout {
        .projectInfo = index.data(Presentation::QueryTreeModelBase::ProjectRole),
        .dataSourceInfo = index.data(Presentation::QueryTreeModelBase::DataSourceRole),
        .contexts = index.data(Presentation::QueryTreeModelBase::ContextListRole).toStringList()
    };
    if (layout.hasAdditionalInfo()) {
        layout.additionalInfoFont = option.font;
        layout.additionalInfoFont.setItalic(true);
        layout.additionalInfoFont.setPointSize(layout.additionalInfoFont.pointSize() - 1);
        layout.additionalInfoHeight = QFontMetrics(layout.additionalInfoFont).height();
        if (layout.showContexts()) {
            layout.additionalInfoHeight += s_yMargin * 2;
        }
    }
    return layout;
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
    Layout layout = doLayout(option, index);
    sz.rheight() += layout.additionalInfoHeight;
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
    Layout layout = doLayout(option, index);
    const auto widget = opt.widget;
    const auto style = widget ? widget->style() : QApplication::style();

    const auto isDone = task ? task->isDone() : false;
    const auto isEnabled = (opt.state & QStyle::State_Enabled);
    const auto isActive = (opt.state & QStyle::State_Active);
    const auto isSelected = (opt.state & QStyle::State_Selected);
    const auto isEditing = (opt.state & QStyle::State_Editing);

    const auto startDate = task ? task->startDate() : QDate();
    const auto dueDate = task ? task->dueDate() : QDate();

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
    const auto summaryRect = textRect.adjusted(0, 0, -dueDateWidth, -layout.additionalInfoHeight);
    const auto dueDateRect = textRect.adjusted(textRect.width() - dueDateWidth, 0, 0, -layout.additionalInfoHeight);
    const auto additionalInfoRect = QRect(textRect.x(), summaryRect.bottom(), textRect.width(), layout.additionalInfoHeight);

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
    if (layout.hasAdditionalInfo()) {
        const auto additionalInfoText = layout.additionalInfoText();

        int x = additionalInfoRect.left();
        if (!additionalInfoText.isEmpty()) {
            painter->setFont(layout.additionalInfoFont);
            QRect textBoundingRect;
            painter->drawText(additionalInfoRect, Qt::AlignLeft, additionalInfoText, &textBoundingRect);
            x = textBoundingRect.right() + s_xSpacingAfterInfoText;
        }   

        x += s_xMargin;
        QFont contextFont = baseFont;
        contextFont.setPointSize(contextFont.pointSize() - 1);
        QFontMetrics contextFontMetrics(contextFont);
        painter->setFont(contextFont);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setOpacity(0.8);
        QRect contextRect = additionalInfoRect;
        for (const QString &context : layout.contexts) {
            contextRect.setLeft(x);
            const auto boundingRect = contextFontMetrics.boundingRect(contextRect, Qt::AlignLeft | Qt::TextSingleLine, context);
            const auto roundRect = boundingRect.adjusted(-s_xMargin, -s_yMargin, s_xMargin, s_yMargin);
            painter->drawRoundedRect(roundRect, s_radius, s_radius);
            painter->drawText(boundingRect, Qt::AlignCenter, context);
            x += roundRect.width() + s_xSpacingBetweenRoundRects;
        }
        painter->setOpacity(1);
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
