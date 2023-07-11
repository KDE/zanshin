/*
 * SPDX-FileCopyrightText: 2016 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "runningtaskwidget.h"
#include "runningtaskmodelinterface.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScreen>

#include <KLocalizedString>
#include <KWindowSystem>
#include <KX11Extras>

using namespace Widgets;

RunningTaskWidget::RunningTaskWidget(QWidget *parent)
    : QWidget(parent),
      m_model(nullptr),
      m_layout(new QHBoxLayout(this)),
      m_titleLabel(new QLabel(this)),
      m_stopButton(new QPushButton(this)),
      m_doneButton(new QPushButton(this)),
      m_collapsed(false)
{
    // BypassWindowManagerHint allows to prevent the window from showing up in Alt-Tab
    // This means no way to focus it with the keyboard, though, obviously.

    setWindowFlags(Qt::Window | Qt::BypassWindowManagerHint | Qt::FramelessWindowHint);
    KX11Extras::setOnAllDesktops(winId(), true);
    KWindowSystem::setState(winId(), NET::KeepAbove | NET::SkipTaskbar | NET::SkipPager);

    setWindowTitle(i18n("Zanshin Running Task Banner"));

    // Current idea for a good background color:
    // the selection color, i.e. usually blue. Arguable ;)
    QPalette pal;
    pal.setBrush(QPalette::Window, pal.brush(QPalette::Highlight));
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
    if (m_model) {
        disconnect(m_model, nullptr, this, nullptr);
    }

    m_model = model;

    if (m_model) {
        connect(m_model, &Presentation::RunningTaskModelInterface::runningTaskChanged,
                this, &RunningTaskWidget::onRunningTaskChanged);
    }
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

void RunningTaskWidget::enterEvent(QEnterEvent *)
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
    const auto screenGeometry = screen()->availableGeometry();
    const int screenWidth = screenGeometry.width();
    const int height = m_collapsed ? 5 : sizeHint().height();
    setGeometry(QRect(screenGeometry.left(), screenGeometry.top(), screenWidth, height));
}

Presentation::RunningTaskModelInterface *RunningTaskWidget::model() const
{
    return m_model;
}

QString RunningTaskWidget::currentText() const
{
    return m_titleLabel->text();
}

#include "moc_runningtaskwidget.cpp"
