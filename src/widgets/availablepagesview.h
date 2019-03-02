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
