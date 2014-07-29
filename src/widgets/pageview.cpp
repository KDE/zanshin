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


#include "pageview.h"

#include <QHeaderView>
#include <QTreeView>
#include <QVBoxLayout>

using namespace Widgets;

Q_DECLARE_METATYPE(QAbstractItemModel*)

PageView::PageView(QWidget *parent)
    : QWidget(parent),
      m_centralView(new QTreeView(this))
{
    m_centralView->setObjectName("centralView");
    m_centralView->header()->hide();

    auto layout = new QVBoxLayout;
    layout->addWidget(m_centralView);
    setLayout(layout);
}

QObject *PageView::model() const
{
    return m_model;
}

void PageView::setModel(QObject *model)
{
    if (model == m_model)
        return;

    m_centralView->setModel(0);

    m_model = model;

    QVariant modelProperty = m_model->property("centralListModel");
    if (modelProperty.canConvert<QAbstractItemModel*>())
        m_centralView->setModel(modelProperty.value<QAbstractItemModel*>());
}

