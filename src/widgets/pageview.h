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


#ifndef WIDGETS_PAGEVIEW_H
#define WIDGETS_PAGEVIEW_H

#include <QWidget>

#include <QHash>
#include <QModelIndexList>
#include <QSharedPointer>

#include <functional>

#include "domain/artifact.h"
#include "messageboxinterface.h"

class QLineEdit;
class QModelIndex;
class QMessageBox;

namespace Widgets {

class FilterWidget;
class PageTreeView;

class PageView : public QWidget
{
    Q_OBJECT
public:
    explicit PageView(QWidget *parent = Q_NULLPTR);

    QHash<QString, QAction*> globalActions() const;

    QObject *model() const;
    MessageBoxInterface::Ptr messageBoxInterface() const;
    QModelIndexList selectedIndexes() const;

public slots:
    void setModel(QObject *model);
    void setMessageBoxInterface(const MessageBoxInterface::Ptr &interface);

signals:
    void currentArtifactChanged(const Domain::Artifact::Ptr &artifact);

private slots:
    void onReturnPressed();
    void onAddItemRequested();
    void onRemoveItemRequested();
    void onPromoteItemRequested();
    void onFilterToggled(bool show);
    void onCurrentChanged(const QModelIndex &current);

private:
    bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;

    QHash<QString, QAction*> m_actions;
    QAction *m_cancelAction;
    QObject *m_model;
    FilterWidget *m_filterWidget;
    PageTreeView *m_centralView;
    QLineEdit *m_quickAddEdit;
    MessageBoxInterface::Ptr m_messageBoxInterface;
};

}

#endif // WIDGETS_PAGEVIEW_H
