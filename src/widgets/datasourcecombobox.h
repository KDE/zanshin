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


#ifndef WIDGETS_DATASOURCECOMBOBOX_H
#define WIDGETS_DATASOURCECOMBOBOX_H

#include <QWidget>

#include "domain/datasource.h"

class QAbstractItemModel;
class QComboBox;

namespace Widgets {

class DataSourceComboBox : public QWidget
{
    Q_OBJECT
public:
    explicit DataSourceComboBox(QWidget *parent = Q_NULLPTR);

    int count() const;
    int currentIndex() const;

    Domain::DataSource::Ptr itemSource(int index) const;

    QAbstractItemModel *model() const;
    void setModel(QAbstractItemModel *model);

    QObject *defaultSourceObject() const;
    QByteArray defaultSourceProperty() const;
    void setDefaultSourceProperty(QObject *object, const char *property);

signals:
    void sourceActivated(const Domain::DataSource::Ptr &source);

private slots:
    void onRefreshDefaultSource();
    void onActivated(int index);

private:
    QComboBox *m_combo;
    QObject *m_object;
    QByteArray m_property;
};

}

#endif // WIDGETS_DATASOURCECOMBOBOX_H
