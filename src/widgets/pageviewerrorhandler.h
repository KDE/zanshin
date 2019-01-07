/* This file is part of Zanshin

   Copyright 2016 Kevin Ottens <ervin@kde.org>

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
