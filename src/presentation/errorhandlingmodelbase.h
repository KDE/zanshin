/*
 * SPDX-FileCopyrightText: 2014 Mario Bensi <mbensi@ipsquad.net>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef PRESENTATION_ERRORHANDLINGMODELBASE_H
#define PRESENTATION_ERRORHANDLINGMODELBASE_H

#include <QString>

class KJob;

namespace Presentation {

class ErrorHandler;

class ErrorHandlingModelBase
{
public:
    ErrorHandlingModelBase();

    ErrorHandler *errorHandler() const;
    void setErrorHandler(ErrorHandler *errorHandler);

protected:
    void installHandler(KJob *job, const QString &message);

private:
    ErrorHandler *m_errorHandler;
};

}

#endif // PRESENTATION_ERRORHANDLINGMODELBASE_H
