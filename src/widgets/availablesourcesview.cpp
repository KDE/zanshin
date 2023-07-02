/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

#include <KLocalizedString>

#include "presentation/metatypes.h"
#include "presentation/querytreemodelbase.h"

#include "widgets/datasourcedelegate.h"

using namespace Widgets;

AvailableSourcesView::AvailableSourcesView(QWidget *parent)
    : QWidget(parent),
      m_defaultAction(new QAction(this)),
      m_model(nullptr),
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
    m_defaultAction->setText(i18n("Use as Default Source"));
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
    settingsAction->setText(i18n("Configure %1...", QApplication::applicationName()));
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

    m_sortProxy->setSourceModel(nullptr);

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

#include "moc_availablesourcesview.cpp"
