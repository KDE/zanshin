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
#include <QKeyEvent>
#include <QHeaderView>
#include <QLineEdit>
#include <QTreeView>
#include <QVBoxLayout>
#include <QMessageBox>

#include "filterwidget.h"
#include "itemdelegate.h"
#include "messagebox.h"

#include <algorithm>

#include "presentation/artifactfilterproxymodel.h"
#include "presentation/metatypes.h"
#include "presentation/querytreemodelbase.h"

namespace Widgets {
class PageTreeView : public QTreeView
{
public:
    using QTreeView::QTreeView;

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE
    {
        if (event->key() == Qt::Key_Escape && state() != EditingState) {
            selectionModel()->clear();
        }

        QTreeView::keyPressEvent(event);
    }
};
}

using namespace Widgets;

PageView::PageView(QWidget *parent)
    : QWidget(parent),
      m_model(Q_NULLPTR),
      m_filterWidget(new FilterWidget(this)),
      m_centralView(new PageTreeView(this)),
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

    m_centralView->setItemsExpandable(false);
    m_centralView->setRootIsDecorated(false);
    connect(m_centralView->model(), SIGNAL(rowsInserted(QModelIndex, int, int)),
            m_centralView, SLOT(expand(QModelIndex)));
    connect(m_centralView->model(), SIGNAL(layoutChanged()),
            m_centralView, SLOT(expandAll()));
    connect(m_centralView->model(), SIGNAL(modelReset()),
            m_centralView, SLOT(expandAll()));
    m_centralView->setStyleSheet( "QTreeView::branch { border-image: url(none.png); }" );

    m_quickAddEdit->setObjectName("quickAddEdit");
    m_quickAddEdit->setPlaceholderText(tr("Type and press enter to add an item"));
    connect(m_quickAddEdit, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

    auto layout = new QVBoxLayout;
    layout->addWidget(m_filterWidget);
    layout->addWidget(m_centralView);
    layout->addWidget(m_quickAddEdit);
    setLayout(layout);

    m_messageBoxInterface = MessageBox::Ptr::create();

    auto addItemAction = new QAction(this);
    addItemAction->setObjectName("addItemAction");
    addItemAction->setText(tr("New item"));
    addItemAction->setIcon(QIcon::fromTheme("list-add"));
    addItemAction->setShortcut(Qt::CTRL | Qt::Key_N);
    connect(addItemAction, SIGNAL(triggered(bool)), m_quickAddEdit, SLOT(selectAll()));
    connect(addItemAction, SIGNAL(triggered(bool)), m_quickAddEdit, SLOT(setFocus()));

    auto cancelAddItemAction = new QAction(this);
    cancelAddItemAction->setObjectName("cancelAddItemAction");
    cancelAddItemAction->setShortcut(Qt::Key_Escape);
    cancelAddItemAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_quickAddEdit->addAction(cancelAddItemAction);
    m_filterWidget->addAction(cancelAddItemAction);
    connect(cancelAddItemAction, SIGNAL(triggered(bool)), m_centralView, SLOT(setFocus()));

    auto removeItemAction = new QAction(this);
    removeItemAction->setObjectName("removeItemAction");
    removeItemAction->setText(tr("Remove item"));
    removeItemAction->setIcon(QIcon::fromTheme("list-remove"));
    removeItemAction->setShortcut(Qt::Key_Delete);
    connect(removeItemAction, SIGNAL(triggered()), this, SLOT(onRemoveItemRequested()));
    addAction(removeItemAction);

    auto promoteItemAction = new QAction(this);
    promoteItemAction->setObjectName("promoteItemAction");
    promoteItemAction->setText(tr("Promote item as project"));
    promoteItemAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_P);
    connect(promoteItemAction, SIGNAL(triggered()), this, SLOT(onPromoteItemRequested()));

    auto filterViewAction = new QAction(this);
    filterViewAction->setObjectName("filterViewAction");
    filterViewAction->setText(tr("Filter..."));
    filterViewAction->setIcon(QIcon::fromTheme("edit-find"));
    filterViewAction->setShortcut(Qt::CTRL | Qt::Key_F);
    connect(filterViewAction, SIGNAL(triggered(bool)), m_filterWidget, SLOT(setFocus()));

    m_actions.insert("page_view_add", addItemAction);
    m_actions.insert("page_view_remove", removeItemAction);
    m_actions.insert("page_view_promote", promoteItemAction);
    m_actions.insert("page_view_filter", filterViewAction);
}

QHash<QString, QAction *> PageView::globalActions() const
{
    return m_actions;
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

QModelIndexList PageView::selectedIndexes() const
{
    using namespace std::placeholders;

    const auto selection = m_centralView->selectionModel()->selectedIndexes();

    auto sourceIndices = QModelIndexList();
    std::transform(selection.constBegin(), selection.constEnd(),
                   std::back_inserter(sourceIndices ),
                   std::bind(&QSortFilterProxyModel::mapToSource, m_filterWidget->proxyModel(), _1));

    return sourceIndices;
}

void PageView::setMessageBoxInterface(const MessageBoxInterface::Ptr &interface)
{
    m_messageBoxInterface = interface;
}

void PageView::onEditingFinished()
{
    if (m_quickAddEdit->text().isEmpty())
        return;

    auto parentIndex = QModelIndex();
    if (m_centralView->selectionModel()->selectedIndexes().size() == 1)
        parentIndex = m_centralView->selectionModel()->selectedIndexes().first();

    QMetaObject::invokeMethod(m_model, "addItem",
                              Q_ARG(QString, m_quickAddEdit->text()),
                              Q_ARG(QModelIndex, parentIndex));
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

void PageView::onPromoteItemRequested()
{
    QModelIndex currentIndex = m_centralView->currentIndex();
    if (!currentIndex.isValid())
        return;
    QMetaObject::invokeMethod(m_model, "promoteItem", Q_ARG(QModelIndex, currentIndex));
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
