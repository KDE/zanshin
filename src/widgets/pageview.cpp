/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageWidget>
#include <KSharedConfig>

#include "filterwidget.h"
#include "itemdelegate.h"
#include "messagebox.h"

#include <algorithm>

#include "presentation/metatypes.h"
#include "presentation/querytreemodelbase.h"
#include "presentation/runningtaskmodelinterface.h"
#include "presentation/taskfilterproxymodel.h"
#include "utils/datetime.h"
#include <QProxyStyle>

namespace Widgets {

class TreeProxyStyle : public QProxyStyle
{
    Q_OBJECT
public:
    using QProxyStyle::QProxyStyle;
    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w) const override {
        if (pe != PE_IndicatorBranch) {
            QProxyStyle::drawPrimitive(pe, opt, p, w);
        }
    }
};

class PageTreeView : public QTreeView
{
    Q_OBJECT
public:
    using QTreeView::QTreeView;

protected:
    void keyPressEvent(QKeyEvent *event) override
    {
        if (event->key() == Qt::Key_Escape && state() != EditingState) {
            selectionModel()->clear();
        }

        QTreeView::keyPressEvent(event);
    }

    void resizeEvent(QResizeEvent *event) override
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
    explicit PassivePopup(QWidget *parent = nullptr)
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

    void setVisible(bool visible) override
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
      m_model(nullptr),
      m_messageWidget(new KMessageWidget(this)),
      m_filterWidget(new FilterWidget(this)),
      m_centralView(new PageTreeView(this)),
      m_quickAddEdit(new QLineEdit(this)),
      m_runningTaskModel(nullptr)
{
    m_messageWidget->setObjectName(QLatin1StringView("messageWidget"));
    m_messageWidget->setCloseButtonVisible(true);
    m_messageWidget->setMessageType(KMessageWidget::Error);
    m_messageWidget->setWordWrap(true);
    m_messageWidget->setPosition(KMessageWidget::Header);
    m_messageWidget->hide();

    m_filterWidget->setObjectName(QLatin1StringView("filterWidget"));
    m_filterWidget->hide();

    m_centralView->setObjectName(QLatin1StringView("centralView"));
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
    m_centralView->setStyle(new TreeProxyStyle);

    m_quickAddEdit->setObjectName(QLatin1StringView("quickAddEdit"));
    m_quickAddEdit->setPlaceholderText(i18nc("@info:placeholder", "Type and press enter to add a task"));
    m_quickAddEdit->setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::TopEdge}));
    connect(m_quickAddEdit, &QLineEdit::returnPressed, this, &PageView::onReturnPressed);

    auto layout = new QVBoxLayout;
    layout->setContentsMargins({});
    layout->setSpacing({});
    layout->addWidget(m_messageWidget);
    layout->addWidget(m_filterWidget);
    layout->addWidget(m_centralView);
    layout->addWidget(m_quickAddEdit);
    setLayout(layout);

    m_messageBoxInterface = MessageBox::Ptr::create();

    auto addItemAction = new QAction(this);
    addItemAction->setObjectName(QLatin1StringView("addItemAction"));
    addItemAction->setText(i18n("New Task"));
    addItemAction->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    addItemAction->setShortcut(Qt::CTRL | Qt::Key_N);
    connect(addItemAction, &QAction::triggered, this, &PageView::onAddItemRequested);

    m_cancelAction->setObjectName(QLatin1StringView("cancelAddItemAction"));
    m_cancelAction->setShortcut(Qt::Key_Escape);
    addAction(m_cancelAction);
    connect(m_cancelAction, &QAction::triggered,
            m_centralView, static_cast<void(QWidget::*)()>(&QWidget::setFocus));

    auto removeItemAction = new QAction(this);
    removeItemAction->setObjectName(QLatin1StringView("removeItemAction"));
    removeItemAction->setText(i18n("Remove Task"));
    removeItemAction->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    removeItemAction->setShortcut(Qt::Key_Delete);
    connect(removeItemAction, &QAction::triggered, this, &PageView::onRemoveItemRequested);
    addAction(removeItemAction);

    auto promoteItemAction = new QAction(this);
    promoteItemAction->setObjectName(QLatin1StringView("promoteItemAction"));
    promoteItemAction->setText(i18n("Promote Task as Project"));
    promoteItemAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_P);
    connect(promoteItemAction, &QAction::triggered, this, &PageView::onPromoteItemRequested);

    auto filterViewAction = new QAction(this);
    filterViewAction->setObjectName(QLatin1StringView("filterViewAction"));
    filterViewAction->setText(i18n("Filter..."));
    filterViewAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-find")));
    filterViewAction->setShortcut(Qt::CTRL | Qt::Key_F);
    filterViewAction->setCheckable(true);
    connect(filterViewAction, &QAction::triggered, this, &PageView::onFilterToggled);

    auto doneViewAction = new QAction(this);
    doneViewAction->setObjectName(QLatin1StringView("doneViewAction"));
    doneViewAction->setText(i18n("Show done tasks"));
    doneViewAction->setIcon(QIcon::fromTheme(QStringLiteral("view-pim-tasks")));
    doneViewAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_D);
    doneViewAction->setCheckable(true);
    connect(doneViewAction, &QAction::triggered, m_filterWidget, &FilterWidget::setShowDoneTasks);

    auto futureViewAction = new QAction(this);
    futureViewAction->setObjectName(QLatin1StringView("futureViewAction"));
    futureViewAction->setText(i18n("Show future tasks"));
    futureViewAction->setIcon(QIcon::fromTheme(QStringLiteral("view-calendar-whatsnext")));
    futureViewAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_F);
    futureViewAction->setCheckable(true);
    connect(futureViewAction, &QAction::triggered, m_filterWidget, &FilterWidget::setShowFutureTasks);

    auto configGroup = KConfigGroup(KSharedConfig::openConfig(), "General");
    if (configGroup.readEntry("ShowDone", true))
        doneViewAction->trigger();

    connect(doneViewAction, &QAction::triggered,
            doneViewAction, [configGroup] (bool checked) mutable {
                configGroup.writeEntry("ShowDone", checked);
            });

    if (configGroup.readEntry("ShowFuture", true))
        futureViewAction->trigger();

    connect(futureViewAction, &QAction::triggered,
            futureViewAction, [configGroup] (bool checked) mutable {
                configGroup.writeEntry("ShowFuture", checked);
            });

    m_runTaskAction = new QAction(this);
    m_runTaskAction->setObjectName(QLatin1StringView("runTaskAction"));
    m_runTaskAction->setShortcut(Qt::CTRL | Qt::Key_Space);
    m_runTaskAction->setText(i18n("Start Now"));
    m_runTaskAction->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
    connect(m_runTaskAction, &QAction::triggered, this, &PageView::onRunTaskTriggered);
    updateRunTaskAction();

    m_actions.insert(QStringLiteral("page_view_add"), addItemAction);
    m_actions.insert(QStringLiteral("page_view_remove"), removeItemAction);
    m_actions.insert(QStringLiteral("page_view_promote"), promoteItemAction);
    m_actions.insert(QStringLiteral("page_view_filter"), filterViewAction);
    m_actions.insert(QStringLiteral("page_view_done"), doneViewAction);
    m_actions.insert(QStringLiteral("page_view_future"), futureViewAction);
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
        disconnect(m_centralView->selectionModel(), nullptr, this, nullptr);
    }

    m_filterWidget->proxyModel()->setSourceModel(nullptr);

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
    if (m_runningTaskModel) {
        connect(m_runningTaskModel, SIGNAL(runningTaskChanged(Domain::Task::Ptr)),
                this, SLOT(onRunningTaskChanged(Domain::Task::Ptr)));
    }
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
    popup->setText(i18n("Type and press enter to add an item"));
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

    auto indexHasChildren = [](QModelIndex index) {
        if (const QAbstractProxyModel *proxy = qobject_cast<const QAbstractProxyModel *>(index.model())) {
            index = proxy->mapToSource(index);
        }
        return index.model()->rowCount(index) > 0;
    };

    QString text;
    if (currentIndexes.size() > 1) {
        const bool hasDescendants = std::any_of(currentIndexes.constBegin(), currentIndexes.constEnd(), [&](const QModelIndex &currentIndex) {
            return currentIndex.isValid() && indexHasChildren(currentIndex); });
        if (hasDescendants)
            text = i18n("Do you really want to delete the selected items and their children?");
        else
            text = i18n("Do you really want to delete the selected items?");

    } else {
        const QModelIndex &currentIndex = currentIndexes.first();
        if (!currentIndex.isValid())
            return;

        if (indexHasChildren(currentIndex))
            text = i18n("Do you really want to delete the selected task and all its children?");
    }

    if (!text.isEmpty()) {
        QMessageBox::Button button = m_messageBoxInterface->askConfirmation(this, i18n("Delete Tasks"), text);
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
            auto task = data.value<Domain::Task::Ptr>();
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
    const auto task = currentTask();
    m_runTaskAction->setEnabled(!task.isNull());
}

void PageView::onRunTaskTriggered()
{
    auto task = currentTask();
    Q_ASSERT(task); // the action is supposed to be disabled otherwise
    if (task->startDate().isNull())
        task->setStartDate(Utils::DateTime::currentDate());
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

    auto task = currentTask();
    if (!task)
        return;

    emit currentTaskChanged(task);
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

Domain::Task::Ptr PageView::currentTask() const
{
    const auto current = m_centralView->selectionModel()->currentIndex();
    const auto data = current.data(Presentation::QueryTreeModelBase::ObjectRole);
    if (!data.isValid())
        return Domain::Task::Ptr();

    return data.value<Domain::Task::Ptr>();
}

#include "pageview.moc"

#include "moc_pageview.cpp"
