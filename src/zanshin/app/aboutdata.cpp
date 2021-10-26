/*
 * SPDX-FileCopyrightText: 2011-2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "aboutdata.h"
#include "../../zanshin_version.h"
#include <QObject>
#include <KLocalizedString>

KAboutData App::getAboutData()
{
    KAboutData about(QStringLiteral("zanshin"),
                     i18n("Zanshin Tasks"), QStringLiteral(ZANSHIN_VERSION_STRING),
                     i18n("A Getting Things Done application which aims at getting your mind like water"),
                     KAboutLicense::GPL_V3,
                     i18n("Copyright 2008-2016, Kevin Ottens <ervin@kde.org>"));

    about.addAuthor(i18n("Kevin Ottens"),
                    i18n("Lead Developer"),
                    QStringLiteral("ervin@kde.org"));

    about.addAuthor(i18n("Mario Bensi"),
                    i18n("Developer"),
                    QStringLiteral("nef@ipsquad.net"));

    about.addAuthor(i18n("Franck Arrecot"),
                    i18n("Developer"),
                    QStringLiteral("franck.arrecot@gmail.com"));

    return about;
}

