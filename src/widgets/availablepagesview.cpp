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
#include <QHeaderView>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

#include "presentation/metatypes.h"
#include "presentation/querytreemodelbase.h"

#include "widgets/newpagedialog.h"

using namespace Widgets;

AvailablePagesView::AvailablePagesView(QWidget *parent)
    : QWidget(parent),
      m_model(0),
      m_sources(0),
      m_pagesView(new QTreeView(this)),
      m_actionBar(new QToolBar(this))
{
    m_pagesView->setObjectName("pagesView");
    m_pagesView->header()->hide();
    m_pagesView->setDragDropMode(QTreeView::DropOnly);

    m_actionBar->setObjectName("actionBar");
    m_actionBar->setIconSize(QSize(16, 16));

    QAction *addAction = new QAction(this);
    addAction->setObjectName("addAction");
    addAction->setText(tr("New page"));
    addAction->setIcon(QIcon::fromTheme("list-add"));
    connect(addAction, SIGNAL(triggered()), this, SLOT(onAddTriggered()));
    m_actionBar->addAction(addAction);

    QAction *removeAction = new QAction(this);
    removeAction->setObjectName("removeAction");
    removeAction->setText(tr("Remove page"));
    removeAction->setIcon(QIcon::fromTheme("list-remove"));
    connect(removeAction, SIGNAL(triggered()), this, SLOT(onRemoveTriggered()));
    m_actionBar->addAction(removeAction);

    QHBoxLayout *actionBarLayout = new QHBoxLayout;
    actionBarLayout->setAlignment(Qt::AlignRight);
    actionBarLayout->addWidget(m_actionBar);

    auto layout = new QVBoxLayout;
    layout->addWidget(m_pagesView);
    layout->addLayout(actionBarLayout);
    setLayout(layout);

    m_dialogFactory = [] (QWidget *parent) {
        return DialogPtr(new NewPageDialog(parent));
    };
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
        disconnect(m_pagesView->selectionModel(), 0, this, 0);
    }

    m_pagesView->setModel(0);

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

void AvailablePagesView::onCurrentChanged(const QModelIndex &current)
{
    QObject *page = 0;
    QMetaObject::invokeMethod(m_model, "createPageForIndex",
                              Q_RETURN_ARG(QObject*, page),
                              Q_ARG(QModelIndex, current));
    emit currentPageChanged(page);
}

void AvailablePagesView::onAddTriggered()
{
    NewPageDialogInterface::Ptr dialog = m_dialogFactory(this);
    dialog->setDataSourcesModel(m_sources);
    dialog->setDefaultSource(m_defaultSource);

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
    const auto &current = m_pagesView->currentIndex();
    if (!current.isValid())
        return;

    QMetaObject::invokeMethod(m_model, "removeItem",
                              Q_ARG(QModelIndex, current));
}

void AvailablePagesView::onInitTimeout()
{
    if (m_pagesView->model()) {
        m_pagesView->setCurrentIndex(m_pagesView->model()->index(0, 0));
        m_pagesView->expandAll();
    }
}
