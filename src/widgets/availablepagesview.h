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

#include <QSharedPointer>

#include <functional>

#include "domain/datasource.h"
#include "messageboxinterface.h"

class QAbstractItemModel;
class QModelIndex;
class QToolBar;
class QTreeView;

namespace Widgets {

class NewPageDialogInterface;

class AvailablePagesView : public QWidget
{
    Q_OBJECT
public:
    typedef QSharedPointer<NewPageDialogInterface> DialogPtr;
    typedef std::function<DialogPtr(QWidget*)> DialogFactory;

    explicit AvailablePagesView(QWidget *parent = 0);

    QObject *model() const;
    QAbstractItemModel *projectSourcesModel() const;
    Domain::DataSource::Ptr defaultProjectSource() const;
    DialogFactory dialogFactory() const;

public slots:
    void setModel(QObject *model);
    void setProjectSourcesModel(QAbstractItemModel *sources);
    void setDefaultProjectSource(const Domain::DataSource::Ptr &source);
    void setDialogFactory(const DialogFactory &factory);
    void setMessageBoxInterface(const MessageBoxInterface::Ptr &interface);

signals:
    void currentPageChanged(QObject *page);

private slots:
    void onCurrentChanged(const QModelIndex &current);
    void onAddTriggered();
    void onRemoveTriggered();
    void onInitTimeout();

private:
    QObject *m_model;
    QAbstractItemModel *m_sources;
    Domain::DataSource::Ptr m_defaultSource;
    QTreeView *m_pagesView;
    QToolBar *m_actionBar;
    DialogFactory m_dialogFactory;
    MessageBoxInterface::Ptr m_messageBoxInterface;
};

}

#endif // WIDGETS_AVAILABLEPAGESVIEW_H
