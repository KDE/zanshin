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


#ifndef WIDGETS_APPLICATIONCOMPONENTS_H
#define WIDGETS_APPLICATIONCOMPONENTS_H

#include <QHash>
#include <QModelIndexList>
#include <QObject>

#include <functional>

#include "domain/artifact.h"

#include "presentation/metatypes.h"

class QAction;
class QWidget;

namespace Widgets {

class AvailablePagesView;
class AvailableSourcesView;
class EditorView;
class PageView;

class QuickSelectDialogInterface;

class ApplicationComponents : public QObject
{
    Q_OBJECT
public:
    typedef QSharedPointer<QuickSelectDialogInterface> QuickSelectDialogPtr;
    typedef std::function<QuickSelectDialogPtr(QWidget*)> QuickSelectDialogFactory;

    explicit ApplicationComponents(QWidget *parent = Q_NULLPTR);

    QHash<QString, QAction*> globalActions() const;

    QObjectPtr model() const;

    AvailableSourcesView *availableSourcesView() const;
    AvailablePagesView *availablePagesView() const;
    PageView *pageView() const;
    EditorView *editorView() const;

    QuickSelectDialogFactory quickSelectDialogFactory() const;

public slots:
    void setModel(const QObjectPtr &model);
    void setQuickSelectDialogFactory(const QuickSelectDialogFactory &factory);

private slots:
    void onCurrentPageChanged(QObject *page);
    void onCurrentArtifactChanged(const Domain::Artifact::Ptr &artifact);
    void onMoveItemsRequested();

private:
    void moveItems(const QModelIndex &destination, const QModelIndexList &droppedItems);

    QHash<QString, QAction*> m_actions;
    QObjectPtr m_model;

    QWidget *m_parent;
    AvailableSourcesView *m_availableSourcesView;
    AvailablePagesView *m_availablePagesView;
    PageView *m_pageView;
    EditorView *m_editorView;

    QuickSelectDialogFactory m_quickSelectDialogFactory;
};

}

#endif // WIDGETS_APPLICATIONCOMPONENTS_H
