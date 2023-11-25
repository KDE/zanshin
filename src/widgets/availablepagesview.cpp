/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#include "availablepagesview.h"

#include <QAction>
#include <QDebug>
#include <QHeaderView>
#include <QMessageBox>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

#include <KLocalizedString>

#include "presentation/metatypes.h"
#include "presentation/querytreemodelbase.h"

#include "widgets/messagebox.h"
#include "widgets/nameanddatasourcedialog.h"
#include "widgets/quickselectdialog.h"

#include "domain/project.h"
#include "domain/context.h"

using namespace Widgets;
using namespace Presentation;

AvailablePagesView::AvailablePagesView(QWidget *parent)
    : QWidget(parent),
      m_addProjectAction(new QAction(this)),
      m_addContextAction(new QAction(this)),
      m_removeAction(new QAction(this)),
      m_model(nullptr),
      m_sources(nullptr),
      m_pagesView(new QTreeView(this))
{
    m_pagesView->setObjectName(QLatin1StringView("pagesView"));
    m_pagesView->header()->hide();
    m_pagesView->setDragDropMode(QTreeView::DropOnly);

    auto actionBar = new QToolBar(this);
    actionBar->setObjectName(QLatin1StringView("actionBar"));
    actionBar->setIconSize(QSize(16, 16));

    m_addProjectAction->setObjectName(QLatin1StringView("addProjectAction"));
    m_addProjectAction->setText(i18n("New Project"));
    m_addProjectAction->setIcon(QIcon::fromTheme(QStringLiteral("view-pim-tasks")));
    connect(m_addProjectAction, &QAction::triggered, this, &AvailablePagesView::onAddProjectTriggered);
    actionBar->addAction(m_addProjectAction);

    m_addContextAction->setObjectName(QLatin1StringView("addContextAction"));
    m_addContextAction->setText(i18n("New Context"));
    m_addContextAction->setIcon(QIcon::fromTheme(QStringLiteral("view-pim-notes")));
    connect(m_addContextAction, &QAction::triggered, this, &AvailablePagesView::onAddContextTriggered);
    actionBar->addAction(m_addContextAction);

    m_removeAction->setObjectName(QLatin1StringView("removeAction"));
    m_removeAction->setText(i18n("Remove Page"));
    m_removeAction->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    connect(m_removeAction, &QAction::triggered, this, &AvailablePagesView::onRemoveTriggered);
    actionBar->addAction(m_removeAction);

    auto actionBarLayout = new QHBoxLayout;
    actionBarLayout->setContentsMargins(0, 0, 0, 0);
    actionBarLayout->setAlignment(Qt::AlignRight);
    actionBarLayout->addWidget(actionBar);

    auto layout = new QVBoxLayout;
    layout->addWidget(m_pagesView);
    layout->addLayout(actionBarLayout);
    setLayout(layout);

    auto margins = layout->contentsMargins();
    margins.setBottom(0);
    layout->setContentsMargins(margins);

    m_projectDialogFactory = [] (QWidget *parent) {
        return NameAndDataSourceDialogPtr(new NameAndDataSourceDialog(parent));
    };
    m_quickSelectDialogFactory = [] (QWidget *parent) {
        return QuickSelectDialogPtr(new QuickSelectDialog(parent));
    };
    m_messageBoxInterface = MessageBox::Ptr::create();

    auto goPreviousAction = new QAction(this);
    goPreviousAction->setObjectName(QLatin1StringView("goPreviousAction"));
    goPreviousAction->setText(i18n("Previous Page"));
    goPreviousAction->setIcon(QIcon::fromTheme(QStringLiteral("go-up")));
    goPreviousAction->setShortcut(Qt::ALT | Qt::Key_Up);
    connect(goPreviousAction, &QAction::triggered, this, &AvailablePagesView::onGoPreviousTriggered);

    auto goNextAction = new QAction(this);
    goNextAction->setObjectName(QLatin1StringView("goNextAction"));
    goNextAction->setText(i18n("Next Page"));
    goNextAction->setIcon(QIcon::fromTheme(QStringLiteral("go-down")));
    goNextAction->setShortcut(Qt::ALT | Qt::Key_Down);
    connect(goNextAction, &QAction::triggered, this, &AvailablePagesView::onGoNextTriggered);

    auto goToAction = new QAction(this);
    goToAction->setObjectName(QLatin1StringView("goToAction"));
    goToAction->setText(i18n("Go to Page..."));
    goToAction->setShortcut(Qt::Key_J);
    connect(goToAction, &QAction::triggered, this, &AvailablePagesView::onGoToTriggered);

    m_actions.insert(QStringLiteral("pages_project_add"), m_addProjectAction);
    m_actions.insert(QStringLiteral("pages_context_add"), m_addContextAction);
    m_actions.insert(QStringLiteral("pages_remove"), m_removeAction);
    m_actions.insert(QStringLiteral("pages_go_previous"), goPreviousAction);
    m_actions.insert(QStringLiteral("pages_go_next"), goNextAction);
    m_actions.insert(QStringLiteral("pages_go_to"), goToAction);
}

QHash<QString, QAction *> AvailablePagesView::globalActions() const
{
    return m_actions;
}

QObject *AvailablePagesView::model() const
{
    return m_model;
}

QAbstractItemModel *AvailablePagesView::projectSourcesModel() const
{
    return m_sources;
}

Domain::DataSource::Ptr AvailablePagesView::defaultProjectSource() const
{
    return m_defaultSource;
}

AvailablePagesView::ProjectDialogFactory AvailablePagesView::projectDialogFactory() const
{
    return m_projectDialogFactory;
}

AvailablePagesView::QuickSelectDialogFactory AvailablePagesView::quickSelectDialogFactory() const
{
    return m_quickSelectDialogFactory;
}

void AvailablePagesView::setModel(QObject *model)
{
    if (model == m_model)
        return;

    if (m_pagesView->selectionModel()) {
        disconnect(m_pagesView->selectionModel(), nullptr, this, nullptr);
    }

    if (m_pagesView->model()) {
        disconnect(m_pagesView->model(), &QAbstractItemModel::rowsInserted, m_pagesView, &QTreeView::expand);
        disconnect(m_pagesView->model(), &QAbstractItemModel::layoutChanged, m_pagesView, &QTreeView::expandAll);
        disconnect(m_pagesView->model(), &QAbstractItemModel::modelReset, m_pagesView, &QTreeView::expandAll);
    }

    m_pagesView->setModel(nullptr);

    m_model = model;

    setEnabled(m_model);

    if (!m_model)
        return;

    QVariant modelProperty = m_model->property("pageListModel");
    if (modelProperty.canConvert<QAbstractItemModel*>()) {
        m_pagesView->setModel(modelProperty.value<QAbstractItemModel*>());

        connect(m_pagesView->model(), &QAbstractItemModel::rowsInserted, m_pagesView, &QTreeView::expand);
        connect(m_pagesView->model(), &QAbstractItemModel::layoutChanged, m_pagesView, &QTreeView::expandAll);
        connect(m_pagesView->model(), &QAbstractItemModel::modelReset, m_pagesView, &QTreeView::expandAll);
    }

    connect(m_pagesView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &AvailablePagesView::onCurrentChanged);

    QMetaObject::invokeMethod(this, "onInitTimeout", Qt::QueuedConnection);
}

void AvailablePagesView::setProjectSourcesModel(QAbstractItemModel *sources)
{
    m_sources = sources;
}

void AvailablePagesView::setDefaultProjectSource(const Domain::DataSource::Ptr &source)
{
    m_defaultSource = source;
}

void AvailablePagesView::setProjectDialogFactory(const AvailablePagesView::ProjectDialogFactory &factory)
{
    m_projectDialogFactory = factory;
}

void AvailablePagesView::setQuickSelectDialogFactory(const AvailablePagesView::QuickSelectDialogFactory &factory)
{
    m_quickSelectDialogFactory = factory;
}

void AvailablePagesView::setMessageBoxInterface(const MessageBoxInterface::Ptr &interface)
{
    m_messageBoxInterface = interface;
}

void AvailablePagesView::onCurrentChanged(const QModelIndex &current)
{
    QObject *page = nullptr;
    QMetaObject::invokeMethod(m_model, "createPageForIndex",
                              Q_RETURN_ARG(QObject*, page),
                              Q_ARG(QModelIndex, current));
    emit currentPageChanged(page);

    const auto object = current.data(QueryTreeModelBase::ObjectRole).value<QObjectPtr>();
    m_removeAction->setEnabled(object.objectCast<Domain::Project>()
                            || object.objectCast<Domain::Context>());
}

void AvailablePagesView::onAddProjectTriggered()
{
    NameAndDataSourceDialogInterface::Ptr dialog = m_projectDialogFactory(this);
    dialog->setDataSourcesModel(m_sources);

    if (dialog->exec() == QDialog::Accepted) {
        m_defaultSource = dialog->dataSource();
        QMetaObject::invokeMethod(m_model, "addProject",
                                  Q_ARG(QString, dialog->name()),
                                  Q_ARG(Domain::DataSource::Ptr, dialog->dataSource()));
    }
}

void AvailablePagesView::onAddContextTriggered()
{
    NameAndDataSourceDialogInterface::Ptr dialog = m_projectDialogFactory(this);
    dialog->setWindowTitle(i18nc("@title:window", "Add a context"));
    dialog->setDataSourcesModel(m_sources);

    if (dialog->exec() == QDialog::Accepted) {
        m_defaultSource = dialog->dataSource();
        QMetaObject::invokeMethod(m_model, "addContext",
                                  Q_ARG(QString, dialog->name()),
                                  Q_ARG(Domain::DataSource::Ptr, dialog->dataSource()));
    }
}

void AvailablePagesView::onRemoveTriggered()
{
    const QModelIndex current = m_pagesView->currentIndex();
    if (!current.isValid())
        return;

    QString title;
    QString text;
    QObjectPtr object = current.data(QueryTreeModelBase::ObjectRole).value<QObjectPtr>();
    if (!object) {
        qDebug() << "Model doesn't have ObjectRole for" << current;
        return;
    }
    if (Domain::Project::Ptr project = object.objectCast<Domain::Project>()) {
        title = i18n("Delete Project");
        text = i18n("Do you really want to delete the project '%1', with all its actions?", project->name());
    } else if (Domain::Context::Ptr context = object.objectCast<Domain::Context>()) {
        title = i18n("Delete Context");
        text = i18n("Do you really want to delete the context '%1'?", context->name());
    } else {
        qFatal("Unrecognized object type");
        return;
    }

    QMessageBox::Button button = m_messageBoxInterface->askConfirmation(this, title, text);
    if (button != QMessageBox::Yes) {
        return;
    }

    QMetaObject::invokeMethod(m_model, "removeItem",
                              Q_ARG(QModelIndex, current));
}

void AvailablePagesView::onGoPreviousTriggered()
{
    auto index = m_pagesView->indexAbove(m_pagesView->currentIndex());

    while (index.isValid() && !(index.flags() & Qt::ItemIsSelectable)) {
        index = m_pagesView->indexAbove(index);
    }

    if (index.isValid())
        m_pagesView->setCurrentIndex(index);
}

void AvailablePagesView::onGoNextTriggered()
{
    auto index = m_pagesView->indexBelow(m_pagesView->currentIndex());

    while (index.isValid() && !(index.flags() & Qt::ItemIsSelectable)) {
        index = m_pagesView->indexBelow(index);
    }

    if (index.isValid())
        m_pagesView->setCurrentIndex(index);
}

void AvailablePagesView::onGoToTriggered()
{
    QuickSelectDialogInterface::Ptr dialog = m_quickSelectDialogFactory(this);
    dialog->setModel(m_pagesView->model());

    if (dialog->exec() == QDialog::Accepted
     && dialog->selectedIndex().isValid()) {
        m_pagesView->setCurrentIndex(dialog->selectedIndex());
    }
}

void AvailablePagesView::onInitTimeout()
{
    if (m_pagesView->model()) {
        m_pagesView->setCurrentIndex(m_pagesView->model()->index(0, 0));
        m_pagesView->expandAll();
    }
}

#include "moc_availablepagesview.cpp"
