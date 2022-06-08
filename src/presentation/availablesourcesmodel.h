/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef PRESENTATION_AVAILABLESOURCESMODEL_H
#define PRESENTATION_AVAILABLESOURCESMODEL_H

#include <QObject>

#include "domain/datasourcequeries.h"
#include "domain/datasourcerepository.h"

#include "presentation/metatypes.h"
#include "presentation/errorhandlingmodelbase.h"

class QModelIndex;

namespace Presentation {

class AvailableSourcesModel : public QObject, public ErrorHandlingModelBase
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* sourceListModel READ sourceListModel)
public:
    explicit AvailableSourcesModel(const Domain::DataSourceQueries::Ptr &dataSourceQueries,
                                   const Domain::DataSourceRepository::Ptr &dataSourceRepository,
                                   QObject *parent = nullptr);

    QAbstractItemModel *sourceListModel();

public Q_SLOTS:
    void setDefaultItem(const QModelIndex &index);

    void showConfigDialog();

private Q_SLOTS:
    void onDefaultSourceChanged();

private:
    void emitDefaultSourceChanged(const QModelIndex &root);

    QAbstractItemModel *createSourceListModel();

    QAbstractItemModel *m_sourceListModel;

    Domain::DataSourceQueries::Ptr m_dataSourceQueries;
    Domain::DataSourceRepository::Ptr m_dataSourceRepository;
};

}

#endif // PRESENTATION_AVAILABLESOURCESMODEL_H
