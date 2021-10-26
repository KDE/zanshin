/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "filterwidget.h"

#include <QBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QToolButton>

#include <KLocalizedString>

#include "presentation/taskfilterproxymodel.h"

#include "ui_filterwidget.h"

using namespace Widgets;

FilterWidget::FilterWidget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::FilterWidget),
      m_model(new Presentation::TaskFilterProxyModel(this))
{
    ui->setupUi(this);
    ui->extension->hide();
    ui->sortTypeCombo->addItem(i18n("Sort by title"), Presentation::TaskFilterProxyModel::TitleSort);
    ui->sortTypeCombo->addItem(i18n("Sort by date"), Presentation::TaskFilterProxyModel::DateSort);
    setFocusProxy(ui->filterEdit);

    connect(ui->filterEdit, &QLineEdit::textChanged, this, &FilterWidget::onTextChanged);
    connect(ui->sortTypeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &FilterWidget::onSortTypeChanged);
    connect(ui->ascendingButton, &QToolButton::clicked, this, &FilterWidget::onAscendingClicked);
    connect(ui->descendingButton, &QToolButton::clicked, this, &FilterWidget::onDescendingClicked);
}

FilterWidget::~FilterWidget()
{
    delete ui;
}

Presentation::TaskFilterProxyModel *FilterWidget::proxyModel() const
{
    return m_model;
}

void FilterWidget::clear()
{
    ui->filterEdit->clear();
}

void FilterWidget::setShowDoneTasks(bool show)
{
    m_model->setShowDoneTasks(show);
}

void FilterWidget::setShowFutureTasks(bool show)
{
    m_model->setShowFutureTasks(show);
}

void FilterWidget::onTextChanged(const QString &text)
{
    m_model->setFilterRegularExpression(QRegularExpression::escape(text));
}

void FilterWidget::onSortTypeChanged(int index)
{
    const int data = ui->sortTypeCombo->itemData(index).toInt();
    m_model->setSortType(Presentation::TaskFilterProxyModel::SortType(data));
}

void FilterWidget::onAscendingClicked()
{
    m_model->setSortOrder(Qt::AscendingOrder);
}

void FilterWidget::onDescendingClicked()
{
    m_model->setSortOrder(Qt::DescendingOrder);
}
