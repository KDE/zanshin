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


#include "availablesourcesview.h"

#include <algorithm>

#include <QAction>
#include <QApplication>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

#include <KLineEdit>

#include "presentation/metatypes.h"
#include "presentation/querytreemodelbase.h"

#include "widgets/datasourcedelegate.h"

using namespace Widgets;

AvailableSourcesView::AvailableSourcesView(QWidget *parent)
    : QWidget(parent),
      m_defaultAction(new QAction(this)),
      m_model(Q_NULLPTR),
      m_sortProxy(new QSortFilterProxyModel(this)),
      m_sourcesView(new QTreeView(this))
{
    m_sortProxy->setDynamicSortFilter(true);
    m_sortProxy->sort(0);

    m_sourcesView->setObjectName(QStringLiteral("sourcesView"));
    m_sourcesView->header()->hide();
    m_sourcesView->setModel(m_sortProxy);
    connect(m_sourcesView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &AvailableSourcesView::onSelectionChanged);
    connect(m_sourcesView->model(), &QAbstractItemModel::rowsInserted, m_sourcesView, &QTreeView::expand);
    connect(m_sourcesView->model(), &QAbstractItemModel::layoutChanged, m_sourcesView, &QTreeView::expandAll);
    connect(m_sourcesView->model(), &QAbstractItemModel::modelReset, m_sourcesView, &QTreeView::expandAll);

    auto delegate = new DataSourceDelegate(m_sourcesView);
    m_sourcesView->setItemDelegate(delegate);

    auto actionBar = new QToolBar(this);
    actionBar->setObjectName(QStringLiteral("actionBar"));
    actionBar->setIconSize(QSize(16, 16));

    m_defaultAction->setObjectName(QStringLiteral("defaultAction"));
    m_defaultAction->setText(tr("Use as Default Source"));
    m_defaultAction->setIcon(QIcon::fromTheme(QStringLiteral("folder-favorites")));
    connect(m_defaultAction, &QAction::triggered, this, &AvailableSourcesView::onDefaultTriggered);
    actionBar->addAction(m_defaultAction);

    auto layout = new QVBoxLayout;
    layout->addWidget(m_sourcesView);

    auto actionBarLayout = new QHBoxLayout;
    actionBarLayout->setContentsMargins(0, 0, 0, 0);
    actionBarLayout->setAlignment(Qt::AlignRight);
    actionBarLayout->addWidget(actionBar);
    layout->addLayout(actionBarLayout);
    setLayout(layout);

    auto margins = layout->contentsMargins();
    margins.setBottom(0);
    layout->setContentsMargins(margins);

    auto settingsAction = new QAction(this);
    settingsAction->setObjectName(QStringLiteral("settingsAction"));
    settingsAction->setText(tr("Configure %1...").arg(QApplication::applicationName()));
    settingsAction->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
    connect(settingsAction, &QAction::triggered, this, &AvailableSourcesView::onSettingsTriggered);
    m_actions.insert(QStringLiteral("options_configure"), settingsAction);

    onSelectionChanged();
}

QHash<QString, QAction *> AvailableSourcesView::globalActions() const
{
    return m_actions;
}

QObject *AvailableSourcesView::model() const
{
    return m_model;
}

void AvailableSourcesView::setModel(QObject *model)
{
    if (model == m_model)
        return;

    m_sortProxy->setSourceModel(Q_NULLPTR);

    m_model = model;

    setEnabled(m_model);

    if (!m_model)
        return;

    setSourceModel("sourceListModel");
}

void AvailableSourcesView::onSelectionChanged()
{
    const auto selectedIndexes = m_sourcesView->selectionModel()->selectedIndexes();
    auto selectedSources = Domain::DataSource::List();
    std::transform(selectedIndexes.constBegin(), selectedIndexes.constEnd(),
                   std::back_inserter(selectedSources),
                   [] (const QModelIndex &index) {
                       return index.data(Presentation::QueryTreeModelBase::ObjectRole)
                                   .value<Domain::DataSource::Ptr>();
                   });

    m_defaultAction->setEnabled(selectedSources.size() == 1
                             && selectedSources.first()->contentTypes() != Domain::DataSource::NoContent);
}

void AvailableSourcesView::onSettingsTriggered()
{
    QMetaObject::invokeMethod(m_model, "showConfigDialog");
}

void AvailableSourcesView::onDefaultTriggered()
{
    const auto currentIndex = m_sourcesView->currentIndex();
    const auto index = m_sortProxy->mapToSource(currentIndex);
    if (index.isValid())
        QMetaObject::invokeMethod(m_model, "setDefaultItem",
                                  Q_ARG(QModelIndex, index));
}

void AvailableSourcesView::setSourceModel(const QByteArray &propertyName)
{
    QVariant modelProperty = m_model->property(propertyName);
    if (modelProperty.canConvert<QAbstractItemModel*>())
        m_sortProxy->setSourceModel(modelProperty.value<QAbstractItemModel*>());
}
