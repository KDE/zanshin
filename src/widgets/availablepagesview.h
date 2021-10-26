/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef WIDGETS_AVAILABLEPAGESVIEW_H
#define WIDGETS_AVAILABLEPAGESVIEW_H

#include <QWidget>

#include <QHash>
#include <QSharedPointer>

#include <functional>

#include "domain/datasource.h"
#include "messageboxinterface.h"

class QAbstractItemModel;
class QModelIndex;
class QTreeView;

namespace Widgets {

class NameAndDataSourceDialogInterface;
class QuickSelectDialogInterface;

class AvailablePagesView : public QWidget
{
    Q_OBJECT
public:
    typedef QSharedPointer<NameAndDataSourceDialogInterface> NameAndDataSourceDialogPtr;
    typedef std::function<NameAndDataSourceDialogPtr(QWidget*)> ProjectDialogFactory;
    typedef QSharedPointer<QuickSelectDialogInterface> QuickSelectDialogPtr;
    typedef std::function<QuickSelectDialogPtr(QWidget*)> QuickSelectDialogFactory;

    explicit AvailablePagesView(QWidget *parent = nullptr);

    QHash<QString, QAction*> globalActions() const;

    QObject *model() const;
    QAbstractItemModel *projectSourcesModel() const;
    Domain::DataSource::Ptr defaultProjectSource() const;
    ProjectDialogFactory projectDialogFactory() const;
    QuickSelectDialogFactory quickSelectDialogFactory() const;

public slots:
    void setModel(QObject *model);
    void setProjectSourcesModel(QAbstractItemModel *sources);
    void setDefaultProjectSource(const Domain::DataSource::Ptr &source);
    void setProjectDialogFactory(const ProjectDialogFactory &factory);
    void setQuickSelectDialogFactory(const QuickSelectDialogFactory &factory);
    void setMessageBoxInterface(const MessageBoxInterface::Ptr &interface);

signals:
    void currentPageChanged(QObject *page);

private slots:
    void onCurrentChanged(const QModelIndex &current);
    void onAddProjectTriggered();
    void onAddContextTriggered();
    void onRemoveTriggered();
    void onGoPreviousTriggered();
    void onGoNextTriggered();
    void onGoToTriggered();
    void onInitTimeout();

private:
    QAction *m_addProjectAction;
    QAction *m_addContextAction;
    QAction *m_removeAction;
    QHash<QString, QAction*> m_actions;

    QObject *m_model;
    QAbstractItemModel *m_sources;
    Domain::DataSource::Ptr m_defaultSource;
    QTreeView *m_pagesView;
    ProjectDialogFactory m_projectDialogFactory;
    QuickSelectDialogFactory m_quickSelectDialogFactory;
    MessageBoxInterface::Ptr m_messageBoxInterface;
};

}

#endif // WIDGETS_AVAILABLEPAGESVIEW_H
