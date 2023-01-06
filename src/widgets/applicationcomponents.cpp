/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "applicationcomponents.h"

#include <memory>

#include <QBoxLayout>
#include <QLabel>
#include <QMimeData>
#include <QVariant>
#include <QWidget>
#include <QWidgetAction>

#include <KLocalizedString>
#include <KWindowSystem>

#include "availablepagesview.h"
#include "availablesourcesview.h"
#include "editorview.h"
#include "pageview.h"
#include "pageviewerrorhandler.h"
#include "quickselectdialog.h"
#include "runningtaskwidget.h"

#include "presentation/runningtaskmodelinterface.h"

using namespace Widgets;

ApplicationComponents::ApplicationComponents(QWidget *parent)
    : QObject(parent),
      m_parent(parent),
      m_availableSourcesView(nullptr),
      m_availablePagesView(nullptr),
      m_pageView(nullptr),
      m_editorView(nullptr),
      m_errorHandler(new PageViewErrorHandler)
{
    m_quickSelectDialogFactory = [] (QWidget *parent) {
        return QuickSelectDialogPtr(new QuickSelectDialog(parent));
    };

    auto moveItemAction = new QAction(this);
    moveItemAction->setObjectName(QStringLiteral("moveItemAction"));
    moveItemAction->setText(i18n("Move Task"));
    moveItemAction->setShortcut(Qt::Key_M);
    connect(moveItemAction, &QAction::triggered, this, &ApplicationComponents::onMoveItemsRequested);

    m_actions.insert(QStringLiteral("page_view_move"), moveItemAction);
}

ApplicationComponents::~ApplicationComponents()
{
    setModel({});
}

QHash<QString, QAction*> ApplicationComponents::globalActions() const
{
    auto actions = QHash<QString, QAction*>();
    actions.insert(availableSourcesView()->globalActions());
    actions.insert(availablePagesView()->globalActions());
    actions.insert(pageView()->globalActions());
    actions.insert(m_actions);

    return actions;
}

QObjectPtr ApplicationComponents::model() const
{
    return m_model;
}

AvailableSourcesView *ApplicationComponents::availableSourcesView() const
{
    if (!m_availableSourcesView) {
        auto view = new AvailableSourcesView(m_parent);
        if (m_model) {
            view->setModel(m_model->property("availableSources").value<QObject*>());
        }

        ApplicationComponents *self = const_cast<ApplicationComponents*>(this);
        self->m_availableSourcesView = view;
    }

    return m_availableSourcesView;
}

AvailablePagesView *ApplicationComponents::availablePagesView() const
{
    if (!m_availablePagesView) {
        auto availablePagesView = new AvailablePagesView(m_parent);
        if (m_model) {
            availablePagesView->setModel(m_model->property("availablePages").value<QObject*>());
            auto availableSources = m_model->property("availableSources").value<QObject*>();
            if (availableSources)
                availablePagesView->setProjectSourcesModel(availableSources->property("sourceListModel").value<QAbstractItemModel*>());
        }

        ApplicationComponents *self = const_cast<ApplicationComponents*>(this);
        self->m_availablePagesView = availablePagesView;

        connect(self->m_availablePagesView, &AvailablePagesView::currentPageChanged, self, &ApplicationComponents::onCurrentPageChanged);
    }

    return m_availablePagesView;
}

PageView *ApplicationComponents::pageView() const
{
    if (!m_pageView) {
        auto pageView = new PageView(m_parent);
        if (m_model) {
            pageView->setModel(m_model->property("currentPage").value<QObject*>());
            pageView->setRunningTaskModel(m_model->property("runningTaskModel").value<Presentation::RunningTaskModelInterface*>());
            connect(m_model.data(), SIGNAL(currentPageChanged(QObject*)),
                    pageView, SLOT(setModel(QObject*)));
        }

        ApplicationComponents *self = const_cast<ApplicationComponents*>(this);
        self->m_pageView = pageView;
        self->m_errorHandler->setPageView(pageView);

        connect(self->m_pageView, &PageView::currentTaskChanged, self, &ApplicationComponents::onCurrentTaskChanged);
    }

    return m_pageView;
}

EditorView *ApplicationComponents::editorView() const
{
    if (!m_editorView) {
        auto editorView = new EditorView(m_parent);
        if (m_model) {
            editorView->setModel(m_model->property("editor").value<QObject*>());
        }

        auto self = const_cast<ApplicationComponents*>(this);
        self->m_editorView = editorView;
    }

    return m_editorView;
}

RunningTaskWidget *ApplicationComponents::runningTaskView() const
{
    if (!m_runningTaskView) {
        auto runningTaskView = new RunningTaskWidget(m_parent);
        if (m_model) {
            runningTaskView->setModel(m_model->property("runningTaskModel").value<Presentation::RunningTaskModelInterface*>());
        }

        auto self = const_cast<ApplicationComponents*>(this);
        self->m_runningTaskView = runningTaskView;
    }

    return m_runningTaskView;
}

ApplicationComponents::QuickSelectDialogFactory ApplicationComponents::quickSelectDialogFactory() const
{
    return m_quickSelectDialogFactory;
}

void ApplicationComponents::setModel(const QObjectPtr &model)
{
    if (m_model == model)
        return;

    if (m_model) {
        if (m_pageView)
            disconnect(m_model.data(), 0, m_pageView, 0);
        m_model->setProperty("errorHandler", 0);
    }

    // Delay deletion of the old model until we're out of scope
    auto tmp = m_model;
    Q_UNUSED(tmp);

    m_model = model;

    if (m_model) {
        m_model->setProperty("errorHandler", QVariant::fromValue(errorHandler()));
    }

    if (m_availableSourcesView) {
        m_availableSourcesView->setModel(m_model ? m_model->property("availableSources").value<QObject*>()
                                                 : nullptr);
    }

    if (m_availablePagesView) {
        m_availablePagesView->setModel(m_model ? m_model->property("availablePages").value<QObject*>()
                                               : nullptr);
        m_availablePagesView->setProjectSourcesModel(m_model ? m_model->property("dataSourcesModel").value<QAbstractItemModel*>()
                                                             : nullptr);
    }

    if (m_pageView) {
        m_pageView->setModel(m_model ? m_model->property("currentPage").value<QObject*>()
                                     : nullptr);
        m_pageView->setRunningTaskModel(m_model ? m_model->property("runningTaskModel").value<Presentation::RunningTaskModelInterface*>()
                                                : nullptr);

        if (m_model) {
            connect(m_model.data(), SIGNAL(currentPageChanged(QObject*)),
                    m_pageView, SLOT(setModel(QObject*)));
        }
    }

    if (m_editorView) {
        m_editorView->setModel(m_model ? m_model->property("editor").value<QObject*>()
                                       : nullptr);
    }

    if (m_runningTaskView) {
        m_runningTaskView->setModel(m_model ? m_model->property("runningTaskModel").value<Presentation::RunningTaskModelInterface*>()
                                            : nullptr);
    } else if (m_model) {
        if (!KWindowSystem::isPlatformWayland()) {
            runningTaskView(); // We got a model so make sure this view exists now
        }
    }
}

void ApplicationComponents::setQuickSelectDialogFactory(const QuickSelectDialogFactory &factory)
{
    m_quickSelectDialogFactory = factory;
}

void ApplicationComponents::onCurrentPageChanged(QObject *page)
{
    if (!m_model)
        return;

    m_model->setProperty("currentPage", QVariant::fromValue(page));

    QObject *editorModel = m_model->property("editor").value<QObject*>();
    if (editorModel)
        editorModel->setProperty("task", QVariant::fromValue(Domain::Task::Ptr()));
}

void ApplicationComponents::onCurrentTaskChanged(const Domain::Task::Ptr &task)
{
    if (!m_model)
        return;

    auto editorModel = m_model->property("editor").value<QObject*>();
    if (editorModel)
        editorModel->setProperty("task", QVariant::fromValue(task));
}

void ApplicationComponents::onMoveItemsRequested()
{
    if (!m_model)
        return;

    if (m_pageView->selectedIndexes().size() == 0)
        return;

    auto pageListModel = m_availablePagesView->model()->property("pageListModel").value<QAbstractItemModel*>();
    Q_ASSERT(pageListModel);

    QuickSelectDialogInterface::Ptr dlg = m_quickSelectDialogFactory(m_pageView);
    dlg->setModel(pageListModel);
    if (dlg->exec() == QDialog::Accepted)
        moveItems(dlg->selectedIndex(), m_pageView->selectedIndexes());
}

Presentation::ErrorHandler *ApplicationComponents::errorHandler() const
{
    return m_errorHandler.data();
}

void ApplicationComponents::moveItems(const QModelIndex &destination, const QModelIndexList &droppedItems)
{
    Q_ASSERT(destination.isValid());
    Q_ASSERT(!droppedItems.isEmpty());

    auto centralListModel = droppedItems.first().model();
    auto availablePagesModel = const_cast<QAbstractItemModel*>(destination.model());

    // drag
    const auto data = std::unique_ptr<QMimeData>(centralListModel->mimeData(droppedItems));

    // drop
    availablePagesModel->dropMimeData(data.get(), Qt::MoveAction, -1, -1, destination);
}


#include "moc_applicationcomponents.cpp"
