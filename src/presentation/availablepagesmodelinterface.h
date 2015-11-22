/* This file is part of Zanshin

   Copyright 2015 Kevin Ottens <ervin@kde.org>

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


#ifndef PRESENTATION_AVAILABLEPAGESMODELINTERFACE_H
#define PRESENTATION_AVAILABLEPAGESMODELINTERFACE_H

#include <QObject>

#include "domain/datasource.h"

#include "presentation/errorhandlingmodelbase.h"

class QAbstractItemModel;
class QModelIndex;

namespace Presentation {

class AvailablePagesModelInterface : public QObject, public ErrorHandlingModelBase
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* pageListModel READ pageListModel)
    Q_PROPERTY(bool hasProjectPages READ hasProjectPages)
    Q_PROPERTY(bool hasContextPages READ hasContextPages)
    Q_PROPERTY(bool hasTagPages READ hasTagPages)
public:
    explicit AvailablePagesModelInterface(QObject *parent = Q_NULLPTR);

    virtual QAbstractItemModel *pageListModel() = 0;

    virtual bool hasProjectPages() const = 0;
    virtual bool hasContextPages() const = 0;
    virtual bool hasTagPages() const = 0;

    Q_SCRIPTABLE virtual QObject *createPageForIndex(const QModelIndex &index) = 0;

public slots:
    virtual void addProject(const QString &name, const Domain::DataSource::Ptr &source) = 0;
    virtual void addContext(const QString &name) = 0;
    virtual void addTag(const QString &name) = 0;
    virtual void removeItem(const QModelIndex &index) = 0;
};

}

#endif // PRESENTATION_AVAILABLEPAGESMODELINTERFACE_H
