/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "applicationmodel.h"

#include "presentation/availablepagesmodel.h"
#include "presentation/availablesourcesmodel.h"
#include "presentation/editormodel.h"
#include "presentation/pagemodel.h"
#include "presentation/runningtaskmodel.h"

#include "utils/dependencymanager.h"
#include "utils/jobhandler.h"

using namespace Presentation;

ApplicationModel::ApplicationModel(QObject *parent)
    : QObject(parent),
      m_errorHandler(nullptr)
{
    MetaTypes::registerAll();
}

ApplicationModel::~ApplicationModel()
{
    Utils::JobHandler::clear();
}

QObject *ApplicationModel::availableSources()
{
    if (!m_availableSources) {
        auto model = Utils::DependencyManager::globalInstance().create<AvailableSourcesModel>();
        model->setErrorHandler(errorHandler());
        m_availableSources = model;
    }
    return m_availableSources.data();
}

QObject *ApplicationModel::availablePages()
{
    if (!m_availablePages) {
        auto model = Utils::DependencyManager::globalInstance().create<AvailablePagesModel>();
        model->setErrorHandler(errorHandler());
        m_availablePages = model;
    }
    return m_availablePages.data();
}

QObject *ApplicationModel::currentPage()
{
    return m_currentPage.data();
}

QObject *ApplicationModel::editor()
{
    if (!m_editor) {
        auto model = Utils::DependencyManager::globalInstance().create<EditorModel>();
        model->setErrorHandler(errorHandler());
        m_editor = model;
    }

    return m_editor.data();
}

RunningTaskModelInterface *ApplicationModel::runningTaskModel()
{
    if (!m_runningTaskModel) {
        auto model = Utils::DependencyManager::globalInstance().create<RunningTaskModel>();
        m_runningTaskModel = model;
        m_runningTaskModel->setErrorHandler(errorHandler());
    }
    return m_runningTaskModel.data();
}

ErrorHandler *ApplicationModel::errorHandler() const
{
    return m_errorHandler;
}

void ApplicationModel::setCurrentPage(QObject *page)
{
    if (page == m_currentPage)
        return;

    m_currentPage = QObjectPtr(page);

    if (m_currentPage) {
        m_currentPage->setParent(nullptr);

        auto pageModel = m_currentPage.staticCast<PageModel>();
        Q_ASSERT(pageModel);
        pageModel->setErrorHandler(errorHandler());
    }

    emit currentPageChanged(page);
}

void ApplicationModel::setErrorHandler(ErrorHandler *errorHandler)
{
    m_errorHandler = errorHandler;
    if (m_availableSources)
        m_availableSources.staticCast<AvailableSourcesModel>()->setErrorHandler(errorHandler);
    if (m_availablePages)
        m_availablePages.staticCast<AvailablePagesModel>()->setErrorHandler(errorHandler);
    if (m_editor)
        m_editor.staticCast<EditorModel>()->setErrorHandler(errorHandler);
    if (m_runningTaskModel)
        m_runningTaskModel.staticCast<RunningTaskModel>()->setErrorHandler(errorHandler);
    if (m_currentPage)
        m_currentPage.staticCast<PageModel>()->setErrorHandler(errorHandler);
}

#include "moc_applicationmodel.cpp"
