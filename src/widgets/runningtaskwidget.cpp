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

#include "runningtaskwidget.h"
#include "runningtaskmodelinterface.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include <KLocalizedString>
#include <KWindowSystem>

using namespace Widgets;

RunningTaskWidget::RunningTaskWidget(QWidget *parent)
    : QWidget(parent),
      m_layout(new QHBoxLayout(this)),
      m_titleLabel(new QLabel(this)),
      m_stopButton(new QPushButton(this)),
      m_doneButton(new QPushButton(this)),
      m_collapsed(false)
{
    // BypassWindowManagerHint allows to prevent the window from showing up in Alt-Tab
    // This means no way to focus it with the keyboard, though, obviously.

    setWindowFlags(Qt::Window | Qt::BypassWindowManagerHint | Qt::FramelessWindowHint);
    KWindowSystem::setOnAllDesktops(winId(), true);
    KWindowSystem::setState(winId(), NET::KeepAbove | NET::SkipTaskbar | NET::SkipPager);

    setWindowTitle(i18n("Zanshin Running Task Banner"));

    // Current idea for a good background color:
    // the selection color, i.e. usually blue. Arguable ;)
    QPalette pal;
    pal.setBrush(QPalette::Background, pal.brush(QPalette::Highlight));
    setPalette(pal);
    setAutoFillBackground(true);

    m_stopButton->setObjectName(QStringLiteral("stopButton"));
    m_stopButton->setText(i18n("Stop"));
    connect(m_stopButton, &QAbstractButton::clicked, this, &RunningTaskWidget::onTaskRunStopped);

    m_doneButton->setObjectName(QStringLiteral("doneButton"));
    m_doneButton->setText(i18n("Done"));
    connect(m_doneButton, &QAbstractButton::clicked, this, &RunningTaskWidget::onTaskRunDone);

    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_stopButton);
    m_layout->addWidget(m_titleLabel, 1, Qt::AlignCenter);
    m_layout->addWidget(m_doneButton);

    setCollapsed(true);
}

void RunningTaskWidget::setModel(Presentation::RunningTaskModelInterface *model)
{
    Q_ASSERT(model);
    m_model = model;
    connect(m_model, &Presentation::RunningTaskModelInterface::runningTaskChanged,
            this, &RunningTaskWidget::onRunningTaskChanged);
}

void RunningTaskWidget::setCollapsed(bool b)
{
    if (m_collapsed == b) {
        return;
    }
    m_collapsed = b;
    m_stopButton->setVisible(!b);
    m_titleLabel->setVisible(!b);
    m_doneButton->setVisible(!b);
    m_layout->activate();
    resize();
}

void RunningTaskWidget::enterEvent(QEvent *)
{
    setCollapsed(false);
}

void RunningTaskWidget::leaveEvent(QEvent *)
{
    setCollapsed(true);
}

void RunningTaskWidget::onRunningTaskChanged(const Domain::Task::Ptr &task)
{
    if (task) {
        m_titleLabel->setText(task->title());
        resize();
        show();
    } else {
        hide();
    }
}

void RunningTaskWidget::onTaskRunStopped()
{
    m_model->stopTask();
}

void RunningTaskWidget::onTaskRunDone()
{
    m_model->doneTask();
}

void RunningTaskWidget::resize()
{
    const auto screenGeometry = qApp->desktop()->availableGeometry(this);
    const int screenWidth = screenGeometry.width();
    const int height = m_collapsed ? 5 : sizeHint().height();
    setGeometry(QRect(screenGeometry.left(), screenGeometry.top(), screenWidth, height));
}

Presentation::RunningTaskModelInterface *RunningTaskWidget::model() const
{
    return m_model;
}
