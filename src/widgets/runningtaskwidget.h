/* This file is part of Zanshin

   Copyright 2016 David Faure <faure@kde.org>

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
    explicit RunningTaskWidget(QWidget *parent = Q_NULLPTR);

    void setModel(Presentation::RunningTaskModelInterface *model);

    Presentation::RunningTaskModelInterface *model() const;

private slots:
    // connected to the model
    void onRunningTaskChanged(const Domain::Task::Ptr &task);
    // connected to the push buttons
    void onTaskRunStopped();
    void onTaskRunDone();

    void setCollapsed(bool b);

protected:
    void enterEvent(QEvent *ev) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *ev) Q_DECL_OVERRIDE;

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
