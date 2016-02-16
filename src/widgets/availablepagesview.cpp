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


#include "availablepagesview.h"

#include <QAction>
#include <QDebug>
#include <QHeaderView>
#include <QMessageBox>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

#include "presentation/metatypes.h"
#include "presentation/querytreemodelbase.h"

#include "widgets/messagebox.h"
#include "widgets/newprojectdialog.h"
#include "widgets/quickselectdialog.h"

#include "domain/project.h"
#include "domain/context.h"
#include "domain/tag.h"

using namespace Widgets;
using namespace Presentation;

AvailablePagesView::AvailablePagesView(QWidget *parent)
    : QWidget(parent),
      m_addProjectAction(new QAction(this)),
      m_addContextAction(new QAction(this)),
      m_addTagAction(new QAction(this)),
      m_removeAction(new QAction(this)),
      m_model(Q_NULLPTR),
      m_sources(Q_NULLPTR),
      m_pagesView(new QTreeView(this))
{
    m_pagesView->setObjectName("pagesView");
    m_pagesView->header()->hide();
    m_pagesView->setDragDropMode(QTreeView::DropOnly);

    auto actionBar = new QToolBar(this);
    actionBar->setObjectName("actionBar");
    actionBar->setIconSize(QSize(16, 16));

    m_addProjectAction->setObjectName("addProjectAction");
    m_addProjectAction->setText(tr("New project"));
    m_addProjectAction->setIcon(QIcon::fromTheme("view-pim-tasks"));
    connect(m_addProjectAction, &QAction::triggered, this, &AvailablePagesView::onAddProjectTriggered);
    actionBar->addAction(m_addProjectAction);

    m_addContextAction->setObjectName("addContextAction");
    m_addContextAction->setText(tr("New context"));
    m_addContextAction->setIcon(QIcon::fromTheme("view-pim-notes"));
    connect(m_addContextAction, &QAction::triggered, this, &AvailablePagesView::onAddContextTriggered);
    actionBar->addAction(m_addContextAction);

    m_addTagAction->setObjectName("addTagAction");
    m_addTagAction->setText(tr("New tag"));
    m_addTagAction->setIcon(QIcon::fromTheme("view-pim-tasks"));
    connect(m_addTagAction, &QAction::triggered, this, &AvailablePagesView::onAddTagTriggered);
    actionBar->addAction(m_addTagAction);

    m_removeAction->setObjectName("removeAction");
    m_removeAction->setText(tr("Remove page"));
    m_removeAction->setIcon(QIcon::fromTheme("list-remove"));
    connect(m_removeAction, &QAction::triggered, this, &AvailablePagesView::onRemoveTriggered);
    actionBar->addAction(m_removeAction);

    auto actionBarLayout = new QHBoxLayout;
    actionBarLayout->setAlignment(Qt::AlignRight);
    actionBarLayout->addWidget(actionBar);

    auto layout = new QVBoxLayout;
    layout->addWidget(m_pagesView);
    layout->addLayout(actionBarLayout);
    setLayout(layout);

    m_projectDialogFactory = [] (QWidget *parent) {
        return NewProjectDialogPtr(new NewProjectDialog(parent));
    };
    m_quickSelectDialogFactory = [] (QWidget *parent) {
        return QuickSelectDialogPtr(new QuickSelectDialog(parent));
    };
    m_messageBoxInterface = MessageBox::Ptr::create();

    auto goPreviousAction = new QAction(this);
    goPreviousAction->setObjectName("goPreviousAction");
    goPreviousAction->setText(tr("Previous page"));
    goPreviousAction->setIcon(QIcon::fromTheme("go-up"));
    goPreviousAction->setShortcut(Qt::ALT | Qt::Key_Up);
    connect(goPreviousAction, &QAction::triggered, this, &AvailablePagesView::onGoPreviousTriggered);

    auto goNextAction = new QAction(this);
    goNextAction->setObjectName("goNextAction");
    goNextAction->setText(tr("Next page"));
    goNextAction->setIcon(QIcon::fromTheme("go-down"));
    goNextAction->setShortcut(Qt::ALT | Qt::Key_Down);
    connect(goNextAction, &QAction::triggered, this, &AvailablePagesView::onGoNextTriggered);

    auto goToAction = new QAction(this);
    goToAction->setObjectName("goToAction");
    goToAction->setText(tr("Go to page..."));
    goToAction->setShortcut(Qt::Key_J);
    connect(goToAction, &QAction::triggered, this, &AvailablePagesView::onGoToTriggered);

    m_actions.insert("pages_project_add", m_addProjectAction);
    m_actions.insert("pages_context_add", m_addContextAction);
    m_actions.insert("pages_tag_add", m_addTagAction);
    m_actions.insert("pages_remove", m_removeAction);
    m_actions.insert("pages_go_previous", goPreviousAction);
    m_actions.insert("pages_go_next", goNextAction);
    m_actions.insert("pages_go_to", goToAction);
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
        disconnect(m_pagesView->selectionModel(), Q_NULLPTR, this, Q_NULLPTR);
    }

    m_pagesView->setModel(Q_NULLPTR);

    m_model = model;

    setEnabled(m_model);

    if (!m_model)
        return;

    m_addProjectAction->setVisible(m_model->property("hasProjectPages").toBool());
    m_addContextAction->setVisible(m_model->property("hasContextPages").toBool());
    m_addTagAction->setVisible(m_model->property("hasTagPages").toBool());

    QVariant modelProperty = m_model->property("pageListModel");
    if (modelProperty.canConvert<QAbstractItemModel*>())
        m_pagesView->setModel(modelProperty.value<QAbstractItemModel*>());

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
    QObject *page = Q_NULLPTR;
    QMetaObject::invokeMethod(m_model, "createPageForIndex",
                              Q_RETURN_ARG(QObject*, page),
                              Q_ARG(QModelIndex, current));
    emit currentPageChanged(page);

    const auto object = current.data(QueryTreeModelBase::ObjectRole).value<QObjectPtr>();
    m_removeAction->setEnabled(object.objectCast<Domain::Project>()
                            || object.objectCast<Domain::Context>()
                            || object.objectCast<Domain::Tag>());
}

void AvailablePagesView::onAddProjectTriggered()
{
    NewProjectDialogInterface::Ptr dialog = m_projectDialogFactory(this);
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
    const QString name = m_messageBoxInterface->askTextInput(this, tr("Add Context"), tr("Context name"));
    if (!name.isEmpty()) {
        QMetaObject::invokeMethod(m_model, "addContext",
                                  Q_ARG(QString, name));
    }
}

void AvailablePagesView::onAddTagTriggered()
{
    const QString name = m_messageBoxInterface->askTextInput(this, tr("Add Tag"), tr("Tag name"));
    if (!name.isEmpty()) {
        QMetaObject::invokeMethod(m_model, "addTag",
                                  Q_ARG(QString, name));
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
        title = tr("Delete Project");
        text = tr("Do you really want to delete the project '%1', with all its actions?").arg(project->name());
    } else if (Domain::Context::Ptr context = object.objectCast<Domain::Context>()) {
        title = tr("Delete Context");
        text = tr("Do you really want to delete the context '%1'?").arg(context->name());
    } else if (Domain::Tag::Ptr tag = object.objectCast<Domain::Tag>()) {
        title = tr("Delete Tag");
        text = tr("Do you really want to delete the tag '%1'?").arg(tag->name());
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
