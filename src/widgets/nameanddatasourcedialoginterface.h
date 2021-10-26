/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef WIDGETS_NAMEANDDATASOURCEDIALOGINTERFACE_H
#define WIDGETS_NAMEANDDATASOURCEDIALOGINTERFACE_H

#include <QString>

#include "domain/datasource.h"

class QAbstractItemModel;

namespace Widgets {

class NameAndDataSourceDialogInterface
{
public:
    typedef QSharedPointer<NameAndDataSourceDialogInterface> Ptr;

    virtual ~NameAndDataSourceDialogInterface();

    virtual int exec() = 0;

    virtual void setWindowTitle(const QString &text) = 0;
    virtual void setDataSourcesModel(QAbstractItemModel *model) = 0;
    virtual QString name() const = 0;
    virtual Domain::DataSource::Ptr dataSource() const = 0;
};

}

#endif // WIDGETS_NAMEANDDATASOURCEDIALOGINTERFACE_H
