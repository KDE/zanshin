/*
 * SPDX-FileCopyrightText: 2014 Mario Bensi <mbensi@ipsquad.net>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "errorhandlingmodelbase.h"

#include "presentation/errorhandler.h"

using namespace Presentation;

ErrorHandlingModelBase::ErrorHandlingModelBase()
    : m_errorHandler(nullptr)
{
}

ErrorHandler *ErrorHandlingModelBase::errorHandler() const
{
    return m_errorHandler;
}

void ErrorHandlingModelBase::setErrorHandler(ErrorHandler *errorHandler)
{
    m_errorHandler = errorHandler;
}

void ErrorHandlingModelBase::installHandler(KJob *job, const QString &message)
{
    if (!m_errorHandler)
        return;

    m_errorHandler->installHandler(job, message);
}
