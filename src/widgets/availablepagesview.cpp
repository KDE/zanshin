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

#include "widgets/newpagedialog.h"
#include "widgets/messagebox.h"

#include "domain/project.h"
#include "domain/context.h"
#include "domain/tag.h"

using namespace Widgets;
using namespace Presentation;

AvailablePagesView::AvailablePagesView(QWidget *parent)
    : QWidget(parent),
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

    auto addAction = new QAction(this);
    addAction->setObjectName("addAction");
    addAction->setText(tr("New page"));
    addAction->setIcon(QIcon::fromTheme("list-add"));
    connect(addAction, SIGNAL(triggered()), this, SLOT(onAddTriggered()));
    actionBar->addAction(addAction);

    auto removeAction = new QAction(this);
    removeAction->setObjectName("removeAction");
    removeAction->setText(tr("Remove page"));
    removeAction->setIcon(QIcon::fromTheme("list-remove"));
    connect(removeAction, SIGNAL(triggered()), this, SLOT(onRemoveTriggered()));
    actionBar->addAction(removeAction);

    auto actionBarLayout = new QHBoxLayout;
    actionBarLayout->setAlignment(Qt::AlignRight);
    actionBarLayout->addWidget(actionBar);

    auto layout = new QVBoxLayout;
    layout->addWidget(m_pagesView);
    layout->addLayout(actionBarLayout);
    setLayout(layout);

    m_dialogFactory = [] (QWidget *parent) {
        return DialogPtr(new NewPageDialog(parent));
    };
    m_messageBoxInterface = MessageBox::Ptr::create();

    auto goPreviousAction = new QAction(this);
    goPreviousAction->setObjectName("goPreviousAction");
    goPreviousAction->setText(tr("Previous page"));
    goPreviousAction->setIcon(QIcon::fromTheme("go-up"));
    goPreviousAction->setShortcut(Qt::ALT | Qt::Key_Up);
    connect(goPreviousAction, SIGNAL(triggered(bool)), this, SLOT(onGoPreviousTriggered()));

    auto goNextAction = new QAction(this);
    goNextAction->setObjectName("goNextAction");
    goNextAction->setText(tr("Next page"));
    goNextAction->setIcon(QIcon::fromTheme("go-down"));
    goNextAction->setShortcut(Qt::ALT | Qt::Key_Down);
    connect(goNextAction, SIGNAL(triggered(bool)), this, SLOT(onGoNextTriggered()));

    m_actions.insert("pages_add", addAction);
    m_actions.insert("pages_remove", removeAction);
    m_actions.insert("pages_go_previous", goPreviousAction);
    m_actions.insert("pages_go_next", goNextAction);
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

AvailablePagesView::DialogFactory AvailablePagesView::dialogFactory() const
{
    return m_dialogFactory;
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

    QVariant modelProperty = m_model->property("pageListModel");
    if (modelProperty.canConvert<QAbstractItemModel*>())
        m_pagesView->setModel(modelProperty.value<QAbstractItemModel*>());

    connect(m_pagesView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(onCurrentChanged(QModelIndex)));

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

void AvailablePagesView::setDialogFactory(const AvailablePagesView::DialogFactory &factory)
{
    m_dialogFactory = factory;
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
}

void AvailablePagesView::onAddTriggered()
{
    NewPageDialogInterface::Ptr dialog = m_dialogFactory(this);
    dialog->setDataSourcesModel(m_sources);

    if (dialog->exec() == QDialog::Accepted) {
        m_defaultSource = dialog->dataSource();
        switch (dialog->pageType()) {
        case NewPageDialogInterface::Project:
            QMetaObject::invokeMethod(m_model, "addProject",
                                      Q_ARG(QString, dialog->name()),
                                      Q_ARG(Domain::DataSource::Ptr, dialog->dataSource()));
            break;
        case NewPageDialogInterface::Context:
            QMetaObject::invokeMethod(m_model, "addContext",
                                      Q_ARG(QString, dialog->name()));
            break;
        case NewPageDialogInterface::Tag:
            QMetaObject::invokeMethod(m_model, "addTag",
                                      Q_ARG(QString, dialog->name()));
            break;
        }
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

    QMessageBox::Button button = m_messageBoxInterface->askConfirmation(this, text, title);
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

void AvailablePagesView::onInitTimeout()
{
    if (m_pagesView->model()) {
        m_pagesView->setCurrentIndex(m_pagesView->model()->index(0, 0));
        m_pagesView->expandAll();
    }
}
