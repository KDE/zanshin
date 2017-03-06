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
#include <QLabel>
#include <QLineEdit>
#include <QTreeView>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QTimer>

#include <KMessageWidget>

#include "filterwidget.h"
#include "itemdelegate.h"
#include "messagebox.h"

#include <algorithm>

#include "presentation/artifactfilterproxymodel.h"
#include "presentation/metatypes.h"
#include "presentation/querytreemodelbase.h"
#include "presentation/runningtaskmodelinterface.h"

namespace Widgets {
class PageTreeView : public QTreeView
{
    Q_OBJECT
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

    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE
    {
        header()->resizeSection(0, event->size().width());
        QTreeView::resizeEvent(event);
    }
};
}

class PassivePopup : public QFrame
{
    Q_OBJECT
public:
    explicit PassivePopup(QWidget *parent = Q_NULLPTR)
        : QFrame(parent),
          m_hideTimer(new QTimer(this)),
          m_label(new QLabel(this))
    {
        setWindowFlags(Qt::Tool
                     | Qt::X11BypassWindowManagerHint
                     | Qt::WindowStaysOnTopHint
                     | Qt::FramelessWindowHint);
        setFrameStyle(QFrame::Box | QFrame::Plain);
        setLineWidth(2);
        setAttribute(Qt::WA_DeleteOnClose);

        setLayout(new QVBoxLayout);
        layout()->addWidget(m_label);

        connect(m_hideTimer, &QTimer::timeout, this, &QWidget::hide);
    }

    void setVisible(bool visible) Q_DECL_OVERRIDE
    {
        if (visible) {
            m_hideTimer->start(2000);
        }
        QFrame::setVisible(visible);
    }

    void setText(const QString &text)
    {
        m_label->setText(text);
    }

private:
    QTimer *m_hideTimer;
    QLabel *m_label;
};

using namespace Widgets;

PageView::PageView(QWidget *parent)
    : QWidget(parent),
      m_cancelAction(new QAction(this)),
      m_model(Q_NULLPTR),
      m_messageWidget(new KMessageWidget(this)),
      m_filterWidget(new FilterWidget(this)),
      m_centralView(new PageTreeView(this)),
      m_quickAddEdit(new QLineEdit(this))
{
    m_messageWidget->setObjectName(QStringLiteral("messageWidget"));
    m_messageWidget->setCloseButtonVisible(true);
    m_messageWidget->setMessageType(KMessageWidget::Error);
    m_messageWidget->setWordWrap(true);
    m_messageWidget->hide();

    m_filterWidget->setObjectName(QStringLiteral("filterWidget"));
    m_filterWidget->hide();

    m_centralView->setObjectName(QStringLiteral("centralView"));
    m_centralView->header()->hide();
    m_centralView->setAlternatingRowColors(true);
    m_centralView->setItemDelegate(new ItemDelegate(this));
    m_centralView->setDragDropMode(QTreeView::DragDrop);
    m_centralView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_centralView->setModel(m_filterWidget->proxyModel());
    m_centralView->installEventFilter(this);

    m_centralView->setItemsExpandable(false);
    m_centralView->setRootIsDecorated(false);
    connect(m_centralView->model(), &QAbstractItemModel::rowsInserted, m_centralView, &QTreeView::expandAll);
    connect(m_centralView->model(), &QAbstractItemModel::layoutChanged, m_centralView, &QTreeView::expandAll);
    connect(m_centralView->model(), &QAbstractItemModel::modelReset, m_centralView, &QTreeView::expandAll);
    m_centralView->setStyleSheet(QStringLiteral("QTreeView::branch { border-image: url(none.png); }"));

    m_quickAddEdit->setObjectName(QStringLiteral("quickAddEdit"));
    m_quickAddEdit->setPlaceholderText(tr("Type and press enter to add an item"));
    connect(m_quickAddEdit, &QLineEdit::returnPressed, this, &PageView::onReturnPressed);

    auto layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 3);
    layout->addWidget(m_messageWidget);
    layout->addWidget(m_filterWidget);
    layout->addWidget(m_centralView);
    layout->addWidget(m_quickAddEdit);
    setLayout(layout);

    m_messageBoxInterface = MessageBox::Ptr::create();

    auto addItemAction = new QAction(this);
    addItemAction->setObjectName(QStringLiteral("addItemAction"));
    addItemAction->setText(tr("New item"));
    addItemAction->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    addItemAction->setShortcut(Qt::CTRL | Qt::Key_N);
    connect(addItemAction, &QAction::triggered, this, &PageView::onAddItemRequested);

    m_cancelAction->setObjectName(QStringLiteral("cancelAddItemAction"));
    m_cancelAction->setShortcut(Qt::Key_Escape);
    addAction(m_cancelAction);
    connect(m_cancelAction, &QAction::triggered,
            m_centralView, static_cast<void(QWidget::*)()>(&QWidget::setFocus));

    auto removeItemAction = new QAction(this);
    removeItemAction->setObjectName(QStringLiteral("removeItemAction"));
    removeItemAction->setText(tr("Remove item"));
    removeItemAction->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    removeItemAction->setShortcut(Qt::Key_Delete);
    connect(removeItemAction, &QAction::triggered, this, &PageView::onRemoveItemRequested);
    addAction(removeItemAction);

    auto promoteItemAction = new QAction(this);
    promoteItemAction->setObjectName(QStringLiteral("promoteItemAction"));
    promoteItemAction->setText(tr("Promote item as project"));
    promoteItemAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_P);
    connect(promoteItemAction, &QAction::triggered, this, &PageView::onPromoteItemRequested);

    auto filterViewAction = new QAction(this);
    filterViewAction->setObjectName(QStringLiteral("filterViewAction"));
    filterViewAction->setText(tr("Filter..."));
    filterViewAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-find")));
    filterViewAction->setShortcut(Qt::CTRL | Qt::Key_F);
    filterViewAction->setCheckable(true);
    connect(filterViewAction, &QAction::triggered, this, &PageView::onFilterToggled);

    m_runTaskAction = new QAction(this);
    m_runTaskAction->setObjectName(QStringLiteral("runTaskAction"));
    m_runTaskAction->setShortcut(Qt::CTRL | Qt::Key_Space);
    m_runTaskAction->setText(tr("Start now"));
    m_runTaskAction->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
    connect(m_runTaskAction, &QAction::triggered, this, &PageView::onRunTaskTriggered);
    updateRunTaskAction();

    m_actions.insert(QStringLiteral("page_view_add"), addItemAction);
    m_actions.insert(QStringLiteral("page_view_remove"), removeItemAction);
    m_actions.insert(QStringLiteral("page_view_promote"), promoteItemAction);
    m_actions.insert(QStringLiteral("page_view_filter"), filterViewAction);
    m_actions.insert(QStringLiteral("page_run_task"), m_runTaskAction);
}

QHash<QString, QAction *> PageView::globalActions() const
{
    return m_actions;
}

QObject *PageView::model() const
{
    return m_model;
}

Presentation::RunningTaskModelInterface *PageView::runningTaskModel() const
{
    return m_runningTaskModel;
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

    setEnabled(m_model);

    updateRunTaskAction();

    if (!m_model)
        return;

    QVariant modelProperty = m_model->property("centralListModel");
    if (modelProperty.canConvert<QAbstractItemModel*>())
        m_filterWidget->proxyModel()->setSourceModel(modelProperty.value<QAbstractItemModel*>());

    connect(m_centralView->selectionModel(), &QItemSelectionModel::currentChanged, this, &PageView::onCurrentChanged);
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

void PageView::setRunningTaskModel(Presentation::RunningTaskModelInterface *model)
{
    m_runningTaskModel = model;
    connect(m_runningTaskModel, SIGNAL(runningTaskChanged(Domain::Task::Ptr)),
            this, SLOT(onRunningTaskChanged(Domain::Task::Ptr)));
}

void PageView::setMessageBoxInterface(const MessageBoxInterface::Ptr &interface)
{
    m_messageBoxInterface = interface;
}

void PageView::displayErrorMessage(const QString &message)
{
    m_messageWidget->setText(message);
    m_messageWidget->animatedShow();
}

void PageView::onReturnPressed()
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

void PageView::onAddItemRequested()
{
    if (m_quickAddEdit->hasFocus())
        return;

    const auto editTopLeft = m_quickAddEdit->geometry().topLeft();
    const auto pos = mapToGlobal(editTopLeft);
    auto popup = new PassivePopup(m_quickAddEdit);
    popup->setText(tr("Type and press enter to add an item"));
    popup->show();
    popup->move(pos - QPoint(0, popup->height()));

    m_quickAddEdit->selectAll();
    m_quickAddEdit->setFocus();
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
        const auto data = currentIndex.data(Presentation::QueryTreeModelBase::ObjectRole);
        if (data.isValid()) {
            auto task = data.value<Domain::Artifact::Ptr>().objectCast<Domain::Task>();
            if (task)
                m_runningTaskModel->taskDeleted(task);
        }
    }
}

void PageView::onPromoteItemRequested()
{
    QModelIndex currentIndex = m_centralView->currentIndex();
    if (!currentIndex.isValid())
        return;
    QMetaObject::invokeMethod(m_model, "promoteItem", Q_ARG(QModelIndex, currentIndex));
}

void PageView::onFilterToggled(bool show)
{
    m_filterWidget->setVisible(show);
    if (show)
        m_filterWidget->setFocus();
    else
        m_filterWidget->clear();
}

void PageView::updateRunTaskAction()
{
    const auto artifact = currentArtifact();
    const auto task = artifact.objectCast<Domain::Task>();
    m_runTaskAction->setEnabled(task);
}

void PageView::onRunTaskTriggered()
{
    auto task = currentArtifact().objectCast<Domain::Task>();
    Q_ASSERT(task); // the action is supposed to be disabled otherwise
    if (task->startDate().isNull())
        task->setStartDate(QDateTime::currentDateTime());
    m_runningTaskModel->setRunningTask(task);
}

void PageView::onRunningTaskChanged(const Domain::Task::Ptr &task)
{
    if (!task) {
        QWidget *toplevel = window();
        toplevel->raise();
        toplevel->activateWindow();
    }
}

void PageView::onCurrentChanged(const QModelIndex &current)
{
    updateRunTaskAction();

    auto data = current.data(Presentation::QueryTreeModelBase::ObjectRole);
    if (!data.isValid())
        return;

    auto artifact = currentArtifact();
    if (!artifact)
        return;

    emit currentArtifactChanged(artifact);
}

bool PageView::eventFilter(QObject *object, QEvent *event)
{
    Q_ASSERT(object == m_centralView);
    switch(event->type()) {
    case QEvent::FocusIn:
        m_cancelAction->setEnabled(false);
        break;
    case QEvent::FocusOut:
        m_cancelAction->setEnabled(true);
        break;
    default:
        break;
    }

    return false;
}

Domain::Artifact::Ptr PageView::currentArtifact() const
{
    const auto current = m_centralView->selectionModel()->currentIndex();
    const auto data = current.data(Presentation::QueryTreeModelBase::ObjectRole);
    if (!data.isValid())
        return Domain::Artifact::Ptr();

    return data.value<Domain::Artifact::Ptr>();
}

#include "pageview.moc"
