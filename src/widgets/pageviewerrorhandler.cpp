/*
 * SPDX-FileCopyrightText: 2016 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "pageviewerrorhandler.h"

#include "widgets/pageview.h"

using namespace Widgets;

PageViewErrorHandler::PageViewErrorHandler()
    : m_pageView(nullptr)
{
}

Widgets::PageView *PageViewErrorHandler::pageView() const
{
    return m_pageView;
}

void PageViewErrorHandler::setPageView(Widgets::PageView *pageView)
{
    m_pageView = pageView;
}

void Widgets::PageViewErrorHandler::doDisplayMessage(const QString &message)
{
    if (m_pageView)
        m_pageView->displayErrorMessage(message);
}
