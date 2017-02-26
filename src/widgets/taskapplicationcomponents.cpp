/* This file is part of Zanshin

   Copyright 2016-2017 David Faure <faure@kde.org>

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

#include "taskapplicationcomponents.h"

#include "pageview.h"
#include "runningtaskwidget.h"
#include "runningtaskmodelinterface.h"

using namespace Widgets;
using namespace Presentation;

TaskApplicationComponents::TaskApplicationComponents(QWidget *parent)
    : ApplicationComponents(parent),
      m_runningTaskWidget(new RunningTaskWidget(parentWidget()))
{
}

TaskApplicationComponents::~TaskApplicationComponents()
{
}

void TaskApplicationComponents::setModel(const QObjectPtr &model)
{
    ApplicationComponents::setModel(model);
    RunningTaskModelInterface *runningTaskModel = model ? model->property("runningTaskModel").value<RunningTaskModelInterface*>()
                                      : nullptr;
    m_runningTaskWidget->setModel(runningTaskModel);
    if (m_pageView) {
        m_pageView->setRunningTaskModel(runningTaskModel);
    }
}

PageView *TaskApplicationComponents::pageView() const
{
    auto pageView = ApplicationComponents::pageView();
    pageView->setRunningTaskModel(model() ? model()->property("runningTaskModel").value<RunningTaskModelInterface*>()
                                          : nullptr);
    return pageView;
}

// Only used by the unittest
RunningTaskWidget *TaskApplicationComponents::runningTaskWidget() const
{
    return m_runningTaskWidget;
}
