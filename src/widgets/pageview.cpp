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


#include "pageview.h"

#include <QAction>
#include <QHeaderView>
#include <QLineEdit>
#include <QTreeView>
#include <QVBoxLayout>
#include <QMessageBox>

#include "filterwidget.h"
#include "itemdelegate.h"
#include "messagebox.h"

#include "presentation/artifactfilterproxymodel.h"
#include "presentation/metatypes.h"
#include "presentation/querytreemodelbase.h"

using namespace Widgets;

PageView::PageView(QWidget *parent)
    : QWidget(parent),
      m_model(Q_NULLPTR),
      m_filterWidget(new FilterWidget(this)),
      m_centralView(new QTreeView(this)),
      m_quickAddEdit(new QLineEdit(this))
{
    m_filterWidget->setObjectName("filterWidget");

    m_centralView->setObjectName("centralView");
    m_centralView->header()->hide();
    m_centralView->setAlternatingRowColors(true);
    m_centralView->setItemDelegate(new ItemDelegate(this));
    m_centralView->setDragDropMode(QTreeView::DragDrop);
    m_centralView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_centralView->setModel(m_filterWidget->proxyModel());

    m_quickAddEdit->setObjectName("quickAddEdit");
    m_quickAddEdit->setPlaceholderText(tr("Type and press enter to add an action"));
    connect(m_quickAddEdit, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

    auto layout = new QVBoxLayout;
    layout->addWidget(m_filterWidget);
    layout->addWidget(m_centralView);
    layout->addWidget(m_quickAddEdit);
    setLayout(layout);

    auto removeItemAction = new QAction(this);
    removeItemAction->setShortcut(Qt::Key_Delete);
    connect(removeItemAction, SIGNAL(triggered()), this, SLOT(onRemoveItemRequested()));
    addAction(removeItemAction);

    m_messageBoxInterface = MessageBox::Ptr::create();
}

QObject *PageView::model() const
{
    return m_model;
}

void PageView::setModel(QObject *model)
{
    if (model == m_model)
        return;

    if (m_centralView->selectionModel()) {
        disconnect(m_centralView->selectionModel(), Q_NULLPTR, this, Q_NULLPTR);
    }

    m_filterWidget->proxyModel()->setSourceModel(Q_NULLPTR);

    m_model = model;

    if (!m_model)
        return;

    QVariant modelProperty = m_model->property("centralListModel");
    if (modelProperty.canConvert<QAbstractItemModel*>())
        m_filterWidget->proxyModel()->setSourceModel(modelProperty.value<QAbstractItemModel*>());

    connect(m_centralView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(onCurrentChanged(QModelIndex)));
}

MessageBoxInterface::Ptr PageView::messageBoxInterface() const
{
    return m_messageBoxInterface;
}

void PageView::setMessageBoxInterface(const MessageBoxInterface::Ptr &interface)
{
    m_messageBoxInterface = interface;
}

void PageView::onEditingFinished()
{
    if (m_quickAddEdit->text().isEmpty())
        return;

    QMetaObject::invokeMethod(m_model, "addTask", Q_ARG(QString, m_quickAddEdit->text()));
    m_quickAddEdit->clear();
}

void PageView::onRemoveItemRequested()
{
    const QModelIndexList &currentIndexes = m_centralView->selectionModel()->selectedIndexes();
    if (currentIndexes.isEmpty())
        return;

    QString text;
    if (currentIndexes.size() > 1) {
        bool hasDescendants = false;
        foreach (const QModelIndex &currentIndex, currentIndexes) {
            if (!currentIndex.isValid())
                continue;

            if (currentIndex.model()->rowCount(currentIndex) > 0) {
                hasDescendants = true;
                break;
            }
        }

        if (hasDescendants)
            text = tr("Do you really want to delete the selected items and their children?");
        else
            text = tr("Do you really want to delete the selected items?");

    } else {
        const QModelIndex &currentIndex = currentIndexes.first();
        if (!currentIndex.isValid())
            return;

        if (currentIndex.model()->rowCount(currentIndex) > 0)
            text = tr("Do you really want to delete the selected task and all its children?");
    }

    if (!text.isEmpty()) {
        QMessageBox::Button button = m_messageBoxInterface->askConfirmation(this, tr("Delete Tasks"), text);
        bool canRemove = (button == QMessageBox::Yes);

        if (!canRemove)
            return;
    }

    foreach (const QModelIndex &currentIndex, currentIndexes) {
        if (!currentIndex.isValid())
            continue;

        QMetaObject::invokeMethod(m_model, "removeItem", Q_ARG(QModelIndex, currentIndex));
    }
}

void PageView::onCurrentChanged(const QModelIndex &current)
{
    auto data = current.data(Presentation::QueryTreeModelBase::ObjectRole);
    if (!data.isValid())
        return;

    auto artifact = data.value<Domain::Artifact::Ptr>();
    if (!artifact)
        return;

    emit currentArtifactChanged(artifact);
}
