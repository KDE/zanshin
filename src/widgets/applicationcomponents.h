/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef WIDGETS_APPLICATIONCOMPONENTS_H
#define WIDGETS_APPLICATIONCOMPONENTS_H

#include <QHash>
#include <QModelIndexList>
#include <QObject>
#include <QPointer>

#include <functional>

#include "domain/task.h"

#include "presentation/metatypes.h"

class QAction;
class QWidget;

namespace Presentation {
class ErrorHandler;
}

namespace Widgets {

class AvailablePagesView;
class AvailableSourcesView;
class EditorView;
class PageView;
class PageViewErrorHandler;
class RunningTaskWidget;

class QuickSelectDialogInterface;

class ApplicationComponents : public QObject
{
    Q_OBJECT
public:
    typedef QSharedPointer<QuickSelectDialogInterface> QuickSelectDialogPtr;
    typedef std::function<QuickSelectDialogPtr(QWidget*)> QuickSelectDialogFactory;

    explicit ApplicationComponents(QWidget *parent = nullptr);
    ~ApplicationComponents();

    QHash<QString, QAction*> globalActions() const;

    QObjectPtr model() const;

    AvailableSourcesView *availableSourcesView() const;
    AvailablePagesView *availablePagesView() const;
    virtual PageView *pageView() const;
    EditorView *editorView() const;
    RunningTaskWidget *runningTaskView() const;

    QuickSelectDialogFactory quickSelectDialogFactory() const;

public slots:
    virtual void setModel(const QObjectPtr &model);
    void setQuickSelectDialogFactory(const QuickSelectDialogFactory &factory);

private slots:
    void onCurrentPageChanged(QObject *page);
    void onCurrentTaskChanged(const Domain::Task::Ptr &task);
    void onMoveItemsRequested();

private:
    Presentation::ErrorHandler *errorHandler() const;
    void moveItems(const QModelIndex &destination, const QModelIndexList &droppedItems);

    QHash<QString, QAction*> m_actions;
    QObjectPtr m_model;

    QWidget *m_parent;
    QPointer<AvailableSourcesView> m_availableSourcesView;
    QPointer<AvailablePagesView> m_availablePagesView;
    QPointer<PageView> m_pageView;
    QPointer<EditorView> m_editorView;
    QPointer<RunningTaskWidget> m_runningTaskView;

    QScopedPointer<PageViewErrorHandler> m_errorHandler;
    QuickSelectDialogFactory m_quickSelectDialogFactory;
};

}

#endif // WIDGETS_APPLICATIONCOMPONENTS_H
