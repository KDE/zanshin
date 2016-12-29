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


#include "filterwidget.h"

#include <QBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QToolButton>

#include "presentation/artifactfilterproxymodel.h"

#include "ui_filterwidget.h"

using namespace Widgets;

FilterWidget::FilterWidget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::FilterWidget),
      m_model(new Presentation::ArtifactFilterProxyModel(this))
{
    ui->setupUi(this);
    ui->extension->hide();
    ui->sortTypeCombo->addItem(tr("Sort by title"), Presentation::ArtifactFilterProxyModel::TitleSort);
    ui->sortTypeCombo->addItem(tr("Sort by date"), Presentation::ArtifactFilterProxyModel::DateSort);
    setFocusProxy(ui->filterEdit);

    connect(ui->filterEdit, &QLineEdit::textChanged, this, &FilterWidget::onTextChanged);
    connect(ui->sortTypeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &FilterWidget::onSortTypeChanged);
    connect(ui->ascendingButton, &QToolButton::clicked, this, &FilterWidget::onAscendingClicked);
    connect(ui->descendingButton, &QToolButton::clicked, this, &FilterWidget::onDescendingClicked);
    connect(ui->showFutureCheck, &QCheckBox::toggled, this, &FilterWidget::onShowFutureChanged);
}

FilterWidget::~FilterWidget()
{
    delete ui;
}

Presentation::ArtifactFilterProxyModel *FilterWidget::proxyModel() const
{
    return m_model;
}

void FilterWidget::clear()
{
    ui->filterEdit->clear();
}

void FilterWidget::onTextChanged(const QString &text)
{
    m_model->setFilterFixedString(text);
}

void FilterWidget::onSortTypeChanged(int index)
{
    const int data = ui->sortTypeCombo->itemData(index).toInt();
    m_model->setSortType(Presentation::ArtifactFilterProxyModel::SortType(data));
}

void FilterWidget::onAscendingClicked()
{
    m_model->setSortOrder(Qt::AscendingOrder);
}

void FilterWidget::onDescendingClicked()
{
    m_model->setSortOrder(Qt::DescendingOrder);
}

void FilterWidget::onShowFutureChanged(bool show)
{
    m_model->setShowFutureTasks(show);
}
