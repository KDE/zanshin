/* This file is part of Zanshin Todo.

   Copyright 2008-2010 Kevin Ottens <ervin@kde.org>

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

#include "actionlisteditorpage.h"

#include <KDE/Akonadi/ItemCreateJob>
#include <KDE/Akonadi/ItemDeleteJob>
#include <KDE/Akonadi/TransactionSequence>

#include <KDE/KConfigGroup>
#include <kdescendantsproxymodel.h>
#include <kmodelindexproxymapper.h>

#include <QtCore/QTimer>
#include <QtGui/QHeaderView>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QVBoxLayout>

#include "actionlistdelegate.h"
#include "todomodel.h"
#include "todotreeview.h"
#include "todohelpers.h"

class GroupLabellingProxyModel : public QSortFilterProxyModel
{
public:
    GroupLabellingProxyModel(QObject *parent = 0)
        : QSortFilterProxyModel(parent)
    {
        setDynamicSortFilter(true);
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        if (role!=Qt::DisplayRole || index.column()!=0) {
            return QSortFilterProxyModel::data(index, role);
        } else {
            int type = index.data(TodoModel::ItemTypeRole).toInt();

            if (type!=TodoModel::ProjectTodo
             && type!=TodoModel::Category) {
                return QSortFilterProxyModel::data(index, role);

            } else {
                QString display = QSortFilterProxyModel::data(index, role).toString();

                QModelIndex currentIndex = index.parent();
                type = currentIndex.data(TodoModel::ItemTypeRole).toInt();

                while (type==TodoModel::ProjectTodo
                    || type==TodoModel::Category) {
                    display = currentIndex.data(Qt::EditRole).toString() + ": " + display;

                    currentIndex = currentIndex.parent();
                    type = currentIndex.data(TodoModel::ItemTypeRole).toInt();
                }

                return display;
            }
        }
    }
};

class GroupSortingProxyModel : public QSortFilterProxyModel
{
public:
    GroupSortingProxyModel(QObject *parent = 0)
        : QSortFilterProxyModel(parent)
    {
        setDynamicSortFilter(true);
        setFilterCaseSensitivity(Qt::CaseInsensitive);
    }

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const
    {
        int leftType = left.data(TodoModel::ItemTypeRole).toInt();
        int rightType = right.data(TodoModel::ItemTypeRole).toInt();

        return leftType==TodoModel::Inbox
            || (leftType==TodoModel::CategoryRoot && rightType!=TodoModel::Inbox)
            || (leftType==TodoModel::Collection && rightType!=TodoModel::Inbox)
            || (leftType==TodoModel::Category && rightType==TodoModel::StandardTodo)
            || (leftType==TodoModel::StandardTodo && rightType!=TodoModel::StandardTodo)
            || (leftType==TodoModel::ProjectTodo && rightType==TodoModel::Collection)
            || (leftType == rightType && QSortFilterProxyModel::lessThan(left, right));
    }
};

class TypeFilterProxyModel : public QSortFilterProxyModel
{
public:
    TypeFilterProxyModel(GroupSortingProxyModel *sorting, QObject *parent = 0)
        : QSortFilterProxyModel(parent), m_sorting(sorting)
    {
        setDynamicSortFilter(true);
        setFilterCaseSensitivity(Qt::CaseInsensitive);
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        QModelIndex sourceChild = sourceModel()->index(sourceRow, 0, sourceParent);
        int type = sourceChild.data(TodoModel::ItemTypeRole).toInt();

        QSize sizeHint = sourceChild.data(Qt::SizeHintRole).toSize();

        return type!=TodoModel::Collection
            && type!=TodoModel::CategoryRoot
            && !sizeHint.isNull(); // SelectionProxyModel uses the null size for items we shouldn't display
    }

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder)
    {
        m_sorting->sort(column, order);
    }

private:
    GroupSortingProxyModel *m_sorting;
};


class ActionListEditorView : public TodoTreeView
{
public:
    ActionListEditorView(QWidget *parent = 0)
        : TodoTreeView(parent) { }

protected:
    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
    {
        QModelIndex index = currentIndex();

        if (index.isValid() && modifiers==Qt::NoModifier) {
            QModelIndex newIndex;
            int newColumn = index.column();

            switch (cursorAction) {
            case MoveLeft:
                do {
                    newColumn--;
                } while (isColumnHidden(newColumn)
                      && newColumn>=0);
                break;

            case MoveRight:
                do {
                    newColumn++;
                } while (isColumnHidden(newColumn)
                      && newColumn<header()->count());
                break;

            default:
                return Akonadi::EntityTreeView::moveCursor(cursorAction, modifiers);
            }

            newIndex = index.sibling(index.row(), newColumn);

            if (newIndex.isValid()) {
                return newIndex;
            }
        }

        return Akonadi::EntityTreeView::moveCursor(cursorAction, modifiers);
    }
};

class ActionListEditorModel : public KDescendantsProxyModel
{
public:
    ActionListEditorModel(QObject *parent = 0)
        : KDescendantsProxyModel(parent)
    {
    }

    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
    {
        if (!sourceModel()) {
            return QAbstractProxyModel::dropMimeData(data, action, row, column, parent);
        }
        QModelIndex sourceParent = mapToSource(parent);
        return sourceModel()->dropMimeData(data, action, row, column, sourceParent);
    }
};

ActionListEditorPage::ActionListEditorPage(QAbstractItemModel *model,
                                           ModelStack *models,
                                           Zanshin::ApplicationMode mode,
                                           QWidget *parent)
    : QWidget(parent), m_mode(mode)
{
    setLayout(new QVBoxLayout(this));
    layout()->setContentsMargins(0, 0, 0, 0);

    m_treeView = new ActionListEditorView(this);

    GroupLabellingProxyModel *labelling = new GroupLabellingProxyModel(this);
    labelling->setSourceModel(model);

    GroupSortingProxyModel *sorting = new GroupSortingProxyModel(this);
    sorting->setSourceModel(labelling);

    ActionListEditorModel *descendants = new ActionListEditorModel(this);
    descendants->setSourceModel(sorting);

    TypeFilterProxyModel *filter = new TypeFilterProxyModel(sorting, this);
    filter->setSourceModel(descendants);

    m_treeView->setModel(filter);
    m_treeView->setItemDelegate(new ActionListDelegate(models, m_treeView));

    m_treeView->header()->setSortIndicatorShown(true);
    m_treeView->setSortingEnabled(true);
    m_treeView->sortByColumn(0, Qt::AscendingOrder);

    m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_treeView->setItemsExpandable(false);
    m_treeView->setRootIsDecorated(false);
    m_treeView->setEditTriggers(m_treeView->editTriggers() | QAbstractItemView::DoubleClicked);

    connect(m_treeView->model(), SIGNAL(modelReset()),
            m_treeView, SLOT(expandAll()));
    connect(m_treeView->model(), SIGNAL(layoutChanged()),
            m_treeView, SLOT(expandAll()));
    connect(m_treeView->model(), SIGNAL(rowsInserted(QModelIndex, int, int)),
            m_treeView, SLOT(expandAll()));

    layout()->addWidget(m_treeView);

    QTimer::singleShot(0, this, SLOT(onAutoHideColumns()));

    connect(m_treeView->header(), SIGNAL(sectionResized(int, int, int)),
            this, SLOT(onColumnsGeometryChanged()));
}

QItemSelectionModel *ActionListEditorPage::selectionModel() const
{
    return m_treeView->selectionModel();
}

void ActionListEditorPage::saveColumnsState(KConfigGroup &config, const QString &key) const
{
    config.writeEntry(key+"/Normal", m_normalStateCache.toBase64());
    config.writeEntry(key+"/NoCollection", m_noCollectionStateCache.toBase64());
}

void ActionListEditorPage::restoreColumnsState(const KConfigGroup &config, const QString &key)
{
    if (config.hasKey(key+"/Normal")) {
        m_normalStateCache = QByteArray::fromBase64(config.readEntry(key+"/Normal", QByteArray()));
    }

    if (config.hasKey(key+"/NoCollection")) {
        m_noCollectionStateCache = QByteArray::fromBase64(config.readEntry(key+"/NoCollection", QByteArray()));
    }

    if (!m_treeView->isColumnHidden(4)) {
        m_treeView->header()->restoreState(m_normalStateCache);
    } else {
        m_treeView->header()->restoreState(m_noCollectionStateCache);
    }
}

void ActionListEditorPage::addNewTodo(const QString &summary)
{
    if (summary.isEmpty()) return;

    QModelIndex current = m_treeView->selectionModel()->currentIndex();

    if (!current.isValid()) {
        kWarning() << "Oops, nothing selected in the list!";
        return;
    }

    int type = current.data(TodoModel::ItemTypeRole).toInt();

    while (current.isValid() && type==TodoModel::StandardTodo) {
        current = current.sibling(current.row()-1, current.column());
        type = current.data(TodoModel::ItemTypeRole).toInt();
    }

    Akonadi::Collection collection;
    QString parentUid;
    QString category;

    switch (type) {
    case TodoModel::StandardTodo:
        kFatal() << "Can't possibly happen!";
        break;

    case TodoModel::ProjectTodo:
        parentUid = current.data(TodoModel::UidRole).toString();
        collection = current.data(Akonadi::EntityTreeModel::ParentCollectionRole).value<Akonadi::Collection>();
        break;

    case TodoModel::Collection:
        collection = current.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
        break;

    case TodoModel::Category:
        category = current.data(Qt::EditRole).toString();
        // fallthrough
    case TodoModel::Inbox:
    case TodoModel::CategoryRoot:
        collection = m_defaultCollection;
        break;
    }

    KCalCore::Todo::Ptr todo(new KCalCore::Todo);
    todo->setSummary(summary);
    if (!parentUid.isEmpty()) {
        todo->setRelatedTo(parentUid);
    }
    if (!category.isEmpty()) {
        todo->setCategories(category);
    }

    Akonadi::Item item;
    item.setMimeType("application/x-vnd.akonadi.calendar.todo");
    item.setPayload<KCalCore::Todo::Ptr>(todo);

    new Akonadi::ItemCreateJob(item, collection);
}

void ActionListEditorPage::removeCurrentTodo()
{
    QModelIndex current = m_treeView->selectionModel()->currentIndex();
    int type = current.data(TodoModel::ItemTypeRole).toInt();

    if (!current.isValid() || type!=TodoModel::StandardTodo) {
        return;
    }

    if (m_mode==Zanshin::ProjectMode) {
        TodoHelpers::removeProject(this, current);
    } else {
        QStringList categories = current.data(TodoModel::CategoriesRole).toStringList();
        for (int i=current.row(); i>=0; --i) {
            QModelIndex index = m_treeView->model()->index(i, 0);
            int type = index.data(TodoModel::ItemTypeRole).toInt();
            if (type==TodoModel::Category) {
                QString category = index.data().toString();
                if (TodoHelpers::removeTodoFromCategory(current, category))
                    break;
            }
        }
    }
}

Zanshin::ApplicationMode ActionListEditorPage::mode()
{
    return m_mode;
}

void ActionListEditorPage::onAutoHideColumns()
{
    switch (m_mode) {
    case Zanshin::ProjectMode:
        hideColumn(1);
        break;
    case Zanshin::CategoriesMode:
        hideColumn(2);
        break;
    }
}

void ActionListEditorPage::hideColumn(int column)
{
    if (!m_treeView->isColumnHidden(column)) {
        m_treeView->hideColumn(column);
    }
}

void ActionListEditorPage::setCollectionColumnHidden(bool hidden)
{
    QByteArray state = hidden ? m_noCollectionStateCache : m_normalStateCache;

    if (!state.isEmpty()) {
        m_treeView->header()->restoreState(state);
    } else {
        m_treeView->setColumnHidden(4, hidden);
    }
}

void ActionListEditorPage::onColumnsGeometryChanged()
{
    if (!m_treeView->isColumnHidden(4)) {
        m_normalStateCache = m_treeView->header()->saveState();
    } else {
        m_noCollectionStateCache = m_treeView->header()->saveState();
    }
}

void ActionListEditorPage::setDefaultCollection(const Akonadi::Collection &collection)
{
    m_defaultCollection = collection;
}
