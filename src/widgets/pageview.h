/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef WIDGETS_PAGEVIEW_H
#define WIDGETS_PAGEVIEW_H

#include <QWidget>

#include <QHash>
#include <QModelIndexList>
#include <QSharedPointer>

#include "domain/task.h"
#include "messageboxinterface.h"

class QLineEdit;
class QModelIndex;
class QMessageBox;

class KMessageWidget;

namespace Presentation {
class RunningTaskModelInterface;
}

namespace Widgets {

class FilterWidget;
class PageTreeView;

class PageView : public QWidget
{
    Q_OBJECT
public:
    explicit PageView(QWidget *parent = nullptr);

    QHash<QString, QAction*> globalActions() const;

    QObject *model() const;
    Presentation::RunningTaskModelInterface *runningTaskModel() const;
    MessageBoxInterface::Ptr messageBoxInterface() const;
    QModelIndexList selectedIndexes() const;

public slots:
    void setModel(QObject *model);
    void setRunningTaskModel(Presentation::RunningTaskModelInterface *model);
    void setMessageBoxInterface(const MessageBoxInterface::Ptr &interface);
    void displayErrorMessage(const QString &message);

signals:
    void currentTaskChanged(const Domain::Task::Ptr &task);

private slots:
    void onReturnPressed();
    void onAddItemRequested();
    void onRemoveItemRequested();
    void onPromoteItemRequested();
    void onFilterToggled(bool show);
    void onCurrentChanged(const QModelIndex &current);
    void onRunTaskTriggered();
    void onRunningTaskChanged(const Domain::Task::Ptr &task);

private:
    bool eventFilter(QObject *object, QEvent *event) override;
    void updateRunTaskAction();
    Domain::Task::Ptr currentTask() const;

    QHash<QString, QAction*> m_actions;
    QAction *m_cancelAction;
    QAction *m_runTaskAction;
    QObject *m_model;
    KMessageWidget *m_messageWidget;
    FilterWidget *m_filterWidget;
    PageTreeView *m_centralView;
    QLineEdit *m_quickAddEdit;
    MessageBoxInterface::Ptr m_messageBoxInterface;
    Presentation::RunningTaskModelInterface *m_runningTaskModel;
};

}

#endif // WIDGETS_PAGEVIEW_H
