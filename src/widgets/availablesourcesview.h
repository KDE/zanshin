/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef WIDGETS_AVAILABLESOURCESVIEW_H
#define WIDGETS_AVAILABLESOURCESVIEW_H

#include <QWidget>

#include <QHash>

#include "domain/datasource.h"

class QSortFilterProxyModel;
class QTreeView;

namespace Widgets {

class AvailableSourcesView : public QWidget
{
    Q_OBJECT
public:
    explicit AvailableSourcesView(QWidget *parent = nullptr);

    QHash<QString, QAction*> globalActions() const;

    QObject *model() const;

    void setSourceModel(const QByteArray &propertyName);
public slots:
    void setModel(QObject *model);

private slots:
    void onSelectionChanged();
    void onSettingsTriggered();
    void onDefaultTriggered();

private:
    QHash<QString, QAction*> m_actions;
    QAction *m_defaultAction;
    QObject *m_model;
    QSortFilterProxyModel *m_sortProxy;
    QTreeView *m_sourcesView;
};

}

#endif // WIDGETS_AVAILABLESOURCESVIEW_H
