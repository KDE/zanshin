/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>

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


#include "applicationmodel.h"

#include "presentation/availablepagesmodelinterface.h"
#include "presentation/availablesourcesmodel.h"
#include "presentation/editormodel.h"
#include "presentation/errorhandler.h"
#include "presentation/pagemodel.h"

#include "utils/dependencymanager.h"
#include "utils/jobhandler.h"

using namespace Presentation;

ApplicationModel::ApplicationModel(QObject *parent)
    : QObject(parent),
      m_errorHandler(Q_NULLPTR)
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
        auto model = Utils::DependencyManager::globalInstance().create<AvailablePagesModelInterface>();
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
        m_currentPage->setParent(Q_NULLPTR);

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
        m_availablePages.staticCast<AvailablePagesModelInterface>()->setErrorHandler(errorHandler);
    if (m_editor)
        m_editor.staticCast<EditorModel>()->setErrorHandler(errorHandler);
    if (m_currentPage)
        m_currentPage.staticCast<PageModel>()->setErrorHandler(errorHandler);
}
