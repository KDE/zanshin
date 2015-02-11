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


#include "datasourcecombobox.h"

#include <QComboBox>
#include <QVBoxLayout>

#include "presentation/querytreemodelbase.h"

using namespace Widgets;

DataSourceComboBox::DataSourceComboBox(QWidget *parent)
    : QWidget(parent),
      m_combo(new QComboBox(this)),
      m_object(Q_NULLPTR)
{
    setFocusProxy(m_combo);

    auto layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_combo);
    setLayout(layout);

    connect(m_combo, SIGNAL(activated(int)), this, SLOT(onActivated(int)));
}

int DataSourceComboBox::count() const
{
    return m_combo->count();
}

int DataSourceComboBox::currentIndex() const
{
    return m_combo->currentIndex();
}

Domain::DataSource::Ptr DataSourceComboBox::itemSource(int index) const
{
    if (!m_combo->model())
        return Domain::DataSource::Ptr();

    const QModelIndex modelIndex = m_combo->model()->index(index, 0);
    if (!modelIndex.isValid())
        return Domain::DataSource::Ptr();

    const QVariant data = modelIndex.data(Presentation::QueryTreeModelBase::ObjectRole);
    const auto source = data.value<Domain::DataSource::Ptr>();
    return source;
}

QAbstractItemModel *DataSourceComboBox::model() const
{
    return m_combo->model();
}

void DataSourceComboBox::setModel(QAbstractItemModel *model)
{
    if (model == m_combo->model())
        return;

    disconnect(m_combo->model(), Q_NULLPTR, this, Q_NULLPTR);

    m_combo->setModel(model);

    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(onRefreshDefaultSource()));
    connect(model, SIGNAL(layoutChanged()), this, SLOT(onRefreshDefaultSource()));
    connect(model, SIGNAL(modelReset()), this, SLOT(onRefreshDefaultSource()));
    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(onRefreshDefaultSource()));
    connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(onRefreshDefaultSource()));

    onRefreshDefaultSource();
}

QObject *DataSourceComboBox::defaultSourceObject() const
{
    return m_object;
}

QByteArray DataSourceComboBox::defaultSourceProperty() const
{
    return m_property;
}

void DataSourceComboBox::setDefaultSourceProperty(QObject *object, const char *property)
{
    m_object = object;
    m_property = property;
    onRefreshDefaultSource();
}

void DataSourceComboBox::onRefreshDefaultSource()
{
    if (!m_object)
        return;

    const QVariant data = m_object->property(m_property);
    const auto defaultSource = data.value<Domain::DataSource::Ptr>();

    if (!defaultSource)
        return;

    for (int index = 0; index < count(); index++) {
        if (itemSource(index) == defaultSource) {
            m_combo->setCurrentIndex(index);
            return;
        }
    }
}

void DataSourceComboBox::onActivated(int index)
{
    Domain::DataSource::Ptr source = itemSource(index);

    if (source)
        emit sourceActivated(source);
}

