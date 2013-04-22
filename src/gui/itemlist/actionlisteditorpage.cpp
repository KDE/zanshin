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

#include <KDE/KConfigGroup>
#include <kdescendantsproxymodel.h>
#include <kmodelindexproxymapper.h>

#include <QtCore/QTimer>
#include <QtGui/QHeaderView>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QVBoxLayout>

#include "core/modelstack.h"
#include "core/noteitem.h"
#include "core/settings.h"
#include "core/pimitemservices.h"
#include "actionlistcombobox.h"
#include "actionlistdelegate.h"
#include "globaldefs.h"
#include "gui/shared/todotreeview.h"
#include "todohelpers.h"
#include <KXMLGUIClient>
#include "filterproxymodel.h"
#include <QComboBox>
#include <KPassivePopup>
#include <KLineEdit>
#include <QToolBar>
#include <Akonadi/ItemDeleteJob>
#include <KGlobal>
#include <KLocalizedString>
#include "searchbar.h"

static const char *_z_defaultColumnStateCache = "AAAA/wAAAAAAAAABAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAAAAAAAvAAAAAFAQEAAQAAAAAAAAAAAAAAAGT/////AAAAgQAAAAAAAAAFAAABNgAAAAEAAAAAAAAAlAAAAAEAAAAAAAAAjQAAAAEAAAAAAAAAcgAAAAEAAAAAAAAAJwAAAAEAAAAA";

class GroupLabellingProxyModel : public QSortFilterProxyModel
{
public:
    GroupLabellingProxyModel(QObject *parent = 0)
        : QSortFilterProxyModel(parent)
    {
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        if (role!=Qt::DisplayRole || index.column()!=0) {
            return QSortFilterProxyModel::data(index, role);
        } else {
            int type = index.data(Zanshin::ItemTypeRole).toInt();

            if (type!=Zanshin::ProjectTodo
             && type!=Zanshin::Category) {
                return QSortFilterProxyModel::data(index, role);

            } else {
                QString display = QSortFilterProxyModel::data(index, role).toString();

                QModelIndex currentIndex = mapToSource(index.parent());
                type = currentIndex.data(Zanshin::ItemTypeRole).toInt();

                while (type==Zanshin::ProjectTodo
                    || type==Zanshin::Category) {
                    display = currentIndex.data().toString() + ": " + display;

                    currentIndex = currentIndex.parent();
                    type = currentIndex.data(Zanshin::ItemTypeRole).toInt();
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
    }

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const
    {
        int leftType = left.data(Zanshin::ItemTypeRole).toInt();
        int rightType = right.data(Zanshin::ItemTypeRole).toInt();

        return leftType==Zanshin::Inbox
            || (leftType==Zanshin::CategoryRoot && rightType!=Zanshin::Inbox)
            || (leftType==Zanshin::Collection && rightType!=Zanshin::Inbox)
            || (leftType==Zanshin::StandardTodo && rightType!=Zanshin::StandardTodo)
            || (leftType==Zanshin::ProjectTodo && rightType==Zanshin::Collection)
            || (leftType == rightType && QSortFilterProxyModel::lessThan(left, right));
    }
};

class TypeFilterProxyModel : public QSortFilterProxyModel
{
public:
    TypeFilterProxyModel(QObject *parent = 0)
        : QSortFilterProxyModel(parent)
    {
        setDynamicSortFilter(true);
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        QModelIndex sourceChild = sourceModel()->index(sourceRow, 0, sourceParent);
        int type = sourceChild.data(Zanshin::ItemTypeRole).toInt();

        QSize sizeHint = sourceChild.data(Qt::SizeHintRole).toSize();

        return type!=Zanshin::Collection
            && type!=Zanshin::CategoryRoot
            && type!=Zanshin::TopicRoot
            && !sizeHint.isNull(); // SelectionProxyModel uses the null size for items we shouldn't display
    }
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

class CollectionsFilterProxyModel : public QSortFilterProxyModel
{
public:
    CollectionsFilterProxyModel(const QString &mimetype, QObject *parent = 0)
        : QSortFilterProxyModel(parent),
        m_mimetype(mimetype)
    {
        setDynamicSortFilter(true);
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        QModelIndex sourceChild = sourceModel()->index(sourceRow, 0, sourceParent);
        Akonadi::Collection col = sourceChild.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();

        return col.isValid()
            && col.contentMimeTypes().contains(m_mimetype)
            && (col.rights() & (Akonadi::Collection::CanChangeItem|Akonadi::Collection::CanCreateItem));
    }

private:
    const QString m_mimetype;
};



ActionListEditorPage::ActionListEditorPage(QAbstractItemModel *model,
                                           ModelStack *models,
                                           Zanshin::ApplicationMode mode,
                                           const QList<QAction*> &contextActions,
                                           const QList<QAction*> &toolbarActions,
                                           QWidget *parent, KXMLGUIClient */*client*/)
    : QWidget(parent), 
    m_mode(mode),
    m_defaultCollectionId(-1)
{
    setLayout(new QVBoxLayout(this));
    layout()->setContentsMargins(0, 0, 0, 0);

    m_treeView = new ActionListEditorView(this);
    
    FilterProxyModel *notefilter = new FilterProxyModel(this);
    notefilter->setSourceModel(model);
    
    SearchBar *searchBar = new SearchBar(notefilter, this);
    layout()->addWidget(searchBar);

    GroupLabellingProxyModel *labelling = new GroupLabellingProxyModel(this);
    labelling->setSourceModel(notefilter);

    GroupSortingProxyModel *sorting = new GroupSortingProxyModel(this);
    sorting->setSourceModel(labelling);

    ActionListEditorModel *descendants = new ActionListEditorModel(this);
    descendants->setSourceModel(sorting);

    TypeFilterProxyModel *filter = new TypeFilterProxyModel(this);
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
    connect(m_treeView->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
            m_treeView, SLOT(expandAll()));

    layout()->addWidget(m_treeView);

    connect(m_treeView->header(), SIGNAL(sectionResized(int,int,int)),
            this, SLOT(onColumnsGeometryChanged()));

    m_treeView->setContextMenuPolicy(Qt::ActionsContextMenu);
    m_treeView->addActions(contextActions);
    
    
    QWidget *bottomBar = new QWidget(this);
    layout()->addWidget(bottomBar);
    bottomBar->setLayout(new QHBoxLayout(bottomBar));
    bottomBar->layout()->setContentsMargins(0, 0, 0, 0);

    m_addActionEdit = new KLineEdit(bottomBar);
    m_addActionEdit->installEventFilter(this);
    bottomBar->layout()->addWidget(m_addActionEdit);
    if(m_mode == Zanshin::KnowledgeMode) {
        m_addActionEdit->setClickMessage(i18n("Type and press enter to add a note"));
    } else {
        m_addActionEdit->setClickMessage(i18n("Type and press enter to add an action"));
    }
    m_addActionEdit->setClearButtonShown(true);
    connect(m_addActionEdit, SIGNAL(returnPressed()),
            this, SLOT(onAddActionRequested()));
    
    m_comboBox = new ActionListComboBox(bottomBar);
    m_comboBox->view()->setTextElideMode(Qt::ElideLeft);
    m_comboBox->setMinimumContentsLength(20);
    m_comboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

    connect(m_comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onComboBoxChanged()));

    QAbstractItemModel *sourceModel = 0;
    QString mimeTypeFilter;
    if (mode == Zanshin::KnowledgeMode) {
        sourceModel = models->knowledgeCollectionsModel();
        mimeTypeFilter = PimItem::mimeType(PimItem::Note);
        m_defaultCollectionId = Settings::instance().defaultNoteCollection().id();
    } else {
        sourceModel = models->collectionsModel();
        mimeTypeFilter = PimItem::mimeType(PimItem::Todo);
        m_defaultCollectionId = Settings::instance().defaultTodoCollection().id();
    }
    KDescendantsProxyModel *descendantProxyModel = new KDescendantsProxyModel(m_comboBox);
    descendantProxyModel->setSourceModel(sourceModel);
    descendantProxyModel->setDisplayAncestorData(true);
    m_todoColsModel = new CollectionsFilterProxyModel(mimeTypeFilter, m_comboBox);
    m_todoColsModel->setSourceModel(descendantProxyModel);
    if (m_defaultCollectionId > 0) {
        if (!selectDefaultCollection(m_todoColsModel, QModelIndex(),
                                    0, m_todoColsModel->rowCount()-1, m_defaultCollectionId)) {
            connect(m_todoColsModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                    this, SLOT(onRowInsertedInComboBox(QModelIndex,int,int)));
        }
    }
    
    m_comboBox->setModel(m_todoColsModel);
    
    bottomBar->layout()->addWidget(m_comboBox);

    QToolBar *toolBar = new QToolBar(bottomBar);
    toolBar->setIconSize(QSize(16, 16));
    bottomBar->layout()->addWidget(toolBar);
    toolBar->addActions(toolbarActions);
    onComboBoxChanged();
    
    connect(&Settings::instance(), SIGNAL(defaultNoteCollectionChanged(Akonadi::Collection)), this, SLOT(setDefaultNoteCollection(Akonadi::Collection)));
    connect(&Settings::instance(), SIGNAL(defaultTodoCollectionChanged(Akonadi::Collection)), this, SLOT(setDefaultCollection(Akonadi::Collection)));

}

void ActionListEditorPage::setCollectionSelectorVisible(bool visible)
{
    m_comboBox->setVisible(visible);
}

void ActionListEditorPage::onComboBoxChanged()
{
    QModelIndex collectionIndex = m_comboBox->model()->index( m_comboBox->currentIndex(), 0 );
    Akonadi::Collection collection = collectionIndex.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
    if (m_mode == Zanshin::KnowledgeMode) { //TODO based on content not viewtype
        Settings::instance().setDefaultNoteCollection(collection);
    } else {
        Settings::instance().setDefaultTodoCollection(collection);
    }

}

void ActionListEditorPage::selectDefaultCollection(const Akonadi::Collection& collection)
{
    selectDefaultCollection(m_todoColsModel, QModelIndex(),
                                    0, m_todoColsModel->rowCount()-1, collection.id());
}

bool ActionListEditorPage::selectDefaultCollection(QAbstractItemModel *model, const QModelIndex &parent, int begin, int end, Akonadi::Collection::Id defaultCol)
{
    for (int i = begin; i <= end; i++) {
        QModelIndex collectionIndex = model->index(i, 0, parent);
        Akonadi::Collection collection = collectionIndex.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
        if (collection.id() == defaultCol) {
            m_comboBox->setCurrentIndex(i);
            m_defaultCollectionId = -1;
            return true;
        }
    }
    return false;
}

void ActionListEditorPage::setCurrentCollection(const Akonadi::Collection& collection)
{
    m_currentCollection = collection;
}

void ActionListEditorPage::onRowInsertedInComboBox(const QModelIndex &parent, int begin, int end)
{
    QAbstractItemModel *model = static_cast<QAbstractItemModel*>(sender());
    if (selectDefaultCollection(model, parent, begin, end, m_defaultCollectionId)) {
        disconnect(this, SLOT(onRowInsertedInComboBox(QModelIndex,int,int)));
    }
}

QItemSelectionModel *ActionListEditorPage::selectionModel() const
{
    return m_treeView->selectionModel();
}

Akonadi::EntityTreeView* ActionListEditorPage::treeView() const
{
    return m_treeView;
}

void ActionListEditorPage::saveColumnsState(KConfigGroup &config, const QString &key) const
{
    config.writeEntry(key+"/Normal", m_normalStateCache.toBase64());
}

void ActionListEditorPage::restoreColumnsState(const KConfigGroup &config, const QString &key)
{
    if (config.hasKey(key+"/Normal")) {
        m_normalStateCache = QByteArray::fromBase64(config.readEntry(key+"/Normal", QByteArray()));
    } else {
        m_normalStateCache = QByteArray::fromBase64(_z_defaultColumnStateCache);
    }

    m_treeView->header()->restoreState(m_normalStateCache);

    m_treeView->setColumnHidden(PimItemModel::Date, false);
    m_treeView->setColumnHidden(PimItemModel::Collection, true);
    m_treeView->setColumnHidden(PimItemModel::Status, true);
}


void ActionListEditorPage::addNewItem(const QString& summary)
{
    if (summary.isEmpty()) return;

    QModelIndex current = m_treeView->selectionModel()->currentIndex();
    Akonadi::Collection collection;
    if (current.isValid()) {
        int type = current.data(Zanshin::ItemTypeRole).toInt();

        while (current.isValid() && type==Zanshin::StandardTodo) {
            current = current.sibling(current.row()-1, current.column());
            type = current.data(Zanshin::ItemTypeRole).toInt();
        }

        switch (type) {
        case Zanshin::StandardTodo:
            kFatal() << "Can't possibly happen!";
            break;

        case Zanshin::ProjectTodo:
            collection = current.data(Akonadi::EntityTreeModel::ParentCollectionRole).value<Akonadi::Collection>();
            break;
        case Zanshin::Collection:
            collection = current.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
            break;
        }
    } else if (m_mode == Zanshin::ProjectMode) { //This actually only happens when there is no Project in a collection which could be shown in this view (and for which we could create todos).
        collection = m_currentCollection;
        PimItemServices::create(PimNode::Project, summary, QList<PimNode>() << PimItemServices::fromIndex(current), collection);
        return;
    }
    if (m_mode == Zanshin::KnowledgeMode) {
        PimItemServices::create(PimNode::Note, summary, QList<PimNode>() << PimItemServices::fromIndex(current), collection);
    } else {
        PimItemServices::create(PimNode::Todo, summary, QList<PimNode>() << PimItemServices::fromIndex(current), collection);
    }
}

void ActionListEditorPage::dissociateTodo(const QModelIndex &current)
{
    if (!current.isValid()) {
        return;
    }
    if (m_mode == Zanshin::CategoriesMode) {
        //FIXME
//        PimItemServices::unlink(current.data(Zanshin::UriRole).toUrl(), PimItemServices::allContexts());
    } else if (m_mode == Zanshin::KnowledgeMode) {
//        PimItemServices::unlink(current.data(Zanshin::UriRole).toUrl(), PimItemServices::allTopics());
    }
}

Zanshin::ApplicationMode ActionListEditorPage::mode()
{
    return m_mode;
}

void ActionListEditorPage::onColumnsGeometryChanged()
{
    m_normalStateCache = m_treeView->header()->saveState();
}

void ActionListEditorPage::setDefaultCollection(const Akonadi::Collection &collection)
{
    //TODO select in combobox
    selectDefaultCollection(collection);
}

void ActionListEditorPage::setDefaultNoteCollection(const Akonadi::Collection& collection)
{
    selectDefaultCollection(collection);
}

bool ActionListEditorPage::selectSiblingIndex(const QModelIndex &index)
{
    QModelIndex sibling = m_treeView->indexBelow(index);
    if (!sibling.isValid()) {
        sibling = m_treeView->indexAbove(index);
    }
    if (sibling.isValid()) {
        m_treeView->selectionModel()->setCurrentIndex(sibling, QItemSelectionModel::Select|QItemSelectionModel::Rows);
        return true;
    }
    return false;
}

void ActionListEditorPage::selectFirstIndex()
{
    QTimer::singleShot(0, this, SLOT(onSelectFirstIndex()));
}

void ActionListEditorPage::onSelectFirstIndex()
{
    // Clear selection to avoid multiple selections when a widget is in edit mode.
    m_treeView->selectionModel()->clearSelection();
    QModelIndex root = m_treeView->model()->index(0, 0);
    if (root.isValid()) {
        m_treeView->selectionModel()->setCurrentIndex(root, QItemSelectionModel::Select|QItemSelectionModel::Rows);
    }
}

void ActionListEditorPage::clearActionEdit()
{
    m_addActionEdit->clear();
}

void ActionListEditorPage::focusActionEdit()
{
    QPoint pos = m_addActionEdit->geometry().topLeft();
    pos = m_addActionEdit->parentWidget()->mapToGlobal(pos);

    KPassivePopup *popup;
    if(m_mode == Zanshin::KnowledgeMode) {
        popup = KPassivePopup::message(i18n("Type and press enter to add a note"), m_addActionEdit);
    } else {
        popup = KPassivePopup::message(i18n("Type and press enter to add an action"), m_addActionEdit);
    }
    popup->move(pos-QPoint(0, popup->height()));
    m_addActionEdit->setFocus();
}

void ActionListEditorPage::setActionEditEnabled(bool enabled)
{
    m_addActionEdit->setEnabled(enabled);
}


void ActionListEditorPage::onAddActionRequested()
{
    QString summary = m_addActionEdit->text().trimmed();
    m_addActionEdit->setText(QString());

    addNewItem(summary);
}

#include "actionlisteditorpage.moc"
