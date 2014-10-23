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

#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QVBoxLayout>

#include "presentation/metatypes.h"

#include "widgets/datasourcedelegate.h"

using namespace Widgets;

AvailableSourcesView::AvailableSourcesView(QWidget *parent)
    : QWidget(parent),
      m_model(0),
      m_sortProxy(new QSortFilterProxyModel(this))
{
    m_sortProxy->setDynamicSortFilter(true);
    m_sortProxy->sort(0);

    auto sourcesView = new QTreeView(this);
    sourcesView->setObjectName("sourcesView");
    sourcesView->header()->hide();
    sourcesView->setModel(m_sortProxy);

    auto delegate = new DataSourceDelegate(sourcesView);
    connect(delegate, SIGNAL(actionTriggered(Domain::DataSource::Ptr,int)),
            this, SLOT(onActionTriggered(Domain::DataSource::Ptr,int)));
    sourcesView->setItemDelegate(delegate);

    auto layout = new QVBoxLayout;
    layout->addWidget(sourcesView);
    setLayout(layout);
}

QObject *AvailableSourcesView::model() const
{
    return m_model;
}

void AvailableSourcesView::setModel(QObject *model)
{
    if (model == m_model)
        return;

    m_sortProxy->setSourceModel(0);

    m_model = model;

    QVariant modelProperty = m_model->property("sourceListModel");
    if (modelProperty.canConvert<QAbstractItemModel*>())
        m_sortProxy->setSourceModel(modelProperty.value<QAbstractItemModel*>());
}

void AvailableSourcesView::onActionTriggered(const Domain::DataSource::Ptr &source, int action)
{
    switch (action) {
    case DataSourceDelegate::AddToList:
        QMetaObject::invokeMethod(m_model, "listSource",
                                  Q_ARG(Domain::DataSource::Ptr, source));
        break;
    case DataSourceDelegate::RemoveFromList:
        QMetaObject::invokeMethod(m_model, "unlistSource",
                                  Q_ARG(Domain::DataSource::Ptr, source));
        break;
    case DataSourceDelegate::Bookmark:
        QMetaObject::invokeMethod(m_model, "bookmarkSource",
                                  Q_ARG(Domain::DataSource::Ptr, source));
        break;
    default:
        qFatal("Shouldn't happen");
        break;
    }
}
