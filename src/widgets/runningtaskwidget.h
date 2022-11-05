/*
 * SPDX-FileCopyrightText: 2016 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef RUNNINGTASKWIDGET_H
#define RUNNINGTASKWIDGET_H

#include <QWidget>
#include "domain/task.h"

class QLabel;
class QHBoxLayout;
class QPushButton;

namespace Presentation {
class RunningTaskModelInterface;
}

namespace Widgets {

class RunningTaskWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RunningTaskWidget(QWidget *parent = nullptr);

    void setModel(Presentation::RunningTaskModelInterface *model);

    Presentation::RunningTaskModelInterface *model() const;

    QString currentText() const; // for the unittest

private slots:
    // connected to the model
    void onRunningTaskChanged(const Domain::Task::Ptr &task);
    // connected to the push buttons
    void onTaskRunStopped();
    void onTaskRunDone();

    void setCollapsed(bool b);

protected:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEvent *ev) override;
#else
    void enterEvent(QEnterEvent *ev) override;
#endif
    void leaveEvent(QEvent *ev) override;

private:
    void resize();

    Presentation::RunningTaskModelInterface *m_model;
    QHBoxLayout *m_layout;
    QLabel *m_titleLabel;
    QPushButton *m_stopButton;
    QPushButton *m_doneButton;
    bool m_collapsed;
};

}

#endif // RUNNINGTASKWIDGET_H
