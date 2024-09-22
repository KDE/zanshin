/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef PRESENTATION_APPLICATIONMODEL_H
#define PRESENTATION_APPLICATIONMODEL_H

#include <QObject>

#include "presentation/metatypes.h"
#include "presentation/errorhandler.h"
#include "presentation/runningtaskmodelinterface.h"

namespace Presentation {

class ApplicationModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* availableSources READ availableSources)
    Q_PROPERTY(QObject* availablePages READ availablePages)
    Q_PROPERTY(QObject* currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged)
    Q_PROPERTY(QObject* editor READ editor)
    Q_PROPERTY(RunningTaskModelInterface* runningTaskModel READ runningTaskModel)
    Q_PROPERTY(Presentation::ErrorHandler* errorHandler READ errorHandler WRITE setErrorHandler)
public:
    typedef QSharedPointer<ApplicationModel> Ptr;

    explicit ApplicationModel(QObject *parent = nullptr);
    ~ApplicationModel();

    QObject *availableSources();
    QObject *availablePages();
    QObject *currentPage();
    QObject *editor();
    Presentation::RunningTaskModelInterface *runningTaskModel();

    ErrorHandler *errorHandler() const;

public slots:
    void setCurrentPage(QObject *page);
    void setErrorHandler(ErrorHandler *errorHandler);

signals:
    void currentPageChanged(QObject *page);

private:
    QObjectPtr m_availableSources;
    QObjectPtr m_availablePages;
    QObjectPtr m_currentPage;
    QObjectPtr m_editor;
    RunningTaskModelInterface::Ptr m_runningTaskModel;

    ErrorHandler *m_errorHandler;
};

}

#endif // PRESENTATION_APPLICATIONMODEL_H
