/*
 * SPDX-FileCopyrightText: 2016 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef WIDGETS_PAGEVIEWERRORHANDLER_H
#define WIDGETS_PAGEVIEWERRORHANDLER_H

#include "presentation/errorhandler.h"

namespace Widgets {

class PageView;

class PageViewErrorHandler : public Presentation::ErrorHandler
{
public:
    PageViewErrorHandler();

    Widgets::PageView *pageView() const;
    void setPageView(Widgets::PageView *pageView);

private:
    void doDisplayMessage(const QString &message) override;

    Widgets::PageView *m_pageView;
};

}

#endif // WIDGETS_PAGEVIEWERRORHANDLER_H
