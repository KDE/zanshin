/*
 * SPDX-FileCopyrightText: 2014 Mario Bensi <mbensi@ipsquad.net>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef PRESENTATION_ERRORHANDLER_H
#define PRESENTATION_ERRORHANDLER_H

#include <QMetaType>
#include <QString>

class KJob;

namespace Presentation {

class ErrorHandler
{
public:
    virtual ~ErrorHandler();

    void installHandler(KJob *job, const QString &message);

private:
    void displayMessage(KJob *job, const QString &message);

    virtual void doDisplayMessage(const QString &message) = 0;
};

}

Q_DECLARE_METATYPE(Presentation::ErrorHandler*)

#endif // PRESENTATION_ERRORHANDLER_H
