/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>

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

#include "actionlistdelegate.h"

#include <QtGui/QCompleter>
#include <QtGui/QLineEdit>
#include <QtGui/QAbstractItemView>
#include <QtGui/QStyledItemDelegate>

#include "actionlistcheckablemodel.h"
#include "actionlistcompleterview.h"
#include "actionlistcompletermodel.h"
#include "actionlistcombobox.h"
#include "combomodel.h"
#include "globaldefs.h"
#include "kdescendantsproxymodel.h"
#include "kdateedit.h"
#include <kmodelindexproxymapper.h>
#include "modelstack.h"

using namespace KPIM;
Q_DECLARE_METATYPE(QItemSelectionModel*)

ActionListDelegate::ActionListDelegate(ModelStack *models, QObject *parent)
    : QStyledItemDelegate(parent)
    , m_models(models)
{
}

ActionListDelegate::~ActionListDelegate()
{

}

QSize ActionListDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    QSize res = QStyledItemDelegate::sizeHint(option, index);

    Zanshin::ItemType type = (Zanshin::ItemType)index.data(Zanshin::ItemTypeRole).toInt();

    if (type!=Zanshin::StandardTodo) {
        res.setHeight(24);
    }

    return res;
}

void ActionListDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    Zanshin::ItemType type = (Zanshin::ItemType)index.data(Zanshin::ItemTypeRole).toInt();

    QStyleOptionViewItemV4 opt = option;

    if (type!=Zanshin::StandardTodo) {
        opt.decorationSize = QSize(22, 22);
        opt.font.setWeight(QFont::Bold);

    } else {
        if (index.row()%2==0) {
            opt.features|= QStyleOptionViewItemV4::Alternate;
        }

        if (index.column()==0) {
            opt.rect.setLeft(opt.rect.left()+32);
        }
    }

    if (isCompleted(index)) {
        opt.font.setStrikeOut(true);
    } else if (isOverdue(index)) {
        opt.palette.setColor(QPalette::Text, QColor(Qt::red));
        opt.palette.setColor(QPalette::HighlightedText, QColor(Qt::red));
    }

    QStyledItemDelegate::paint(painter, opt, index);
}

KCalCore::Todo::Ptr ActionListDelegate::todoFromIndex(const QModelIndex &index) const
{
    Zanshin::ItemType type = (Zanshin::ItemType)index.data(Zanshin::ItemTypeRole).toInt();

    if (type!=Zanshin::StandardTodo) {
        return KCalCore::Todo::Ptr();
    }

    Akonadi::Item item = index.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    if (!item.isValid() || !item.hasPayload<KCalCore::Todo::Ptr>()) {
        return KCalCore::Todo::Ptr();
    }

    return item.payload<KCalCore::Todo::Ptr>();
}

bool ActionListDelegate::isCompleted(const QModelIndex &index) const
{
    KCalCore::Todo::Ptr todo = todoFromIndex(index);

    if (todo) {
        return todo->isCompleted();
    } else {
        return false;
    }
}

bool ActionListDelegate::isOverdue(const QModelIndex &index) const
{
    KCalCore::Todo::Ptr todo = todoFromIndex(index);

    if (todo) {
        return todo->isOverdue();
    } else {
        return false;
    }
}

QWidget *ActionListDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    if (index.data(Qt::EditRole).type()==QVariant::Date) {
        return new KDateEdit(parent);
    } else if (index.data(Zanshin::DataTypeRole).toInt() == Zanshin::CategoryType) {
        return createComboBox(m_models->categoriesComboModel(), parent, index, true);
    } else if (index.data(Zanshin::DataTypeRole).toInt() == Zanshin::ProjectType) {
        return createComboBox(m_models->treeComboModel(), parent, index, false);
    } else {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

QWidget *ActionListDelegate::createComboBox(QAbstractItemModel *model, QWidget *parent, const QModelIndex &selectedIndex, bool isCategory) const
{
    ActionListComboBox *comboBox = new ActionListComboBox(parent);
    comboBox->setEditable(true);
    comboBox->view()->setTextElideMode(Qt::ElideNone);

    QCompleter *completer = new QCompleter(comboBox);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);

    if (isCategory) {
        comboBox->setAutoHidePopupEnabled(true);
        QItemSelectionModel *checkModel = new QItemSelectionModel(model, comboBox);
        ActionListCheckableModel *checkable = new ActionListCheckableModel(comboBox);
        QStringList ancestorsCategories = selectedIndex.data(Zanshin::AncestorsCategoriesRole).value<QStringList>();
        checkable->setDisabledCategories(ancestorsCategories);
        checkable->setSourceModel(model);
        checkable->setSelectionModel(checkModel);

        QStringList categories = selectedIndex.data(Zanshin::CategoriesRole).value<QStringList>();
        Q_ASSERT(checkable->rowCount() == model->rowCount());
        for (int i = 0; i < checkable->rowCount(); ++i) {
            QModelIndex checkIndex = checkable->index(i, 0);
            QModelIndex index = model->index(i, 0);
            foreach (QString item, categories) {
                if (index.data(Zanshin::CategoryPathRole).toString() == item && checkIndex.flags() & Qt::ItemIsEnabled) {
                    checkModel->select(index, QItemSelectionModel::Toggle);
                }
            }
        }
        comboBox->setModel(checkable);
        ActionListCompleterModel *completerModel = new ActionListCompleterModel(checkModel, completer);
        completerModel->setSourceModel(checkable);
        completer->setModel(completerModel);
        ActionListCompleterView *listView = new ActionListCompleterView(comboBox);
        completer->setPopup(listView);
    } else {
        comboBox->setModel(model);
        completer->setModel(model);
        comboBox->setEditText(selectedIndex.data().toString());
    }
    connect(completer, SIGNAL(activated(QModelIndex)), this, SLOT(onCompleterActivated(QModelIndex)));
    comboBox->setCompleter(completer);

    return comboBox;
}

void ActionListDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    KDateEdit *dateEdit = qobject_cast<KDateEdit*>(editor);

    if (dateEdit) {
        dateEdit->setDate(index.data(Qt::EditRole).toDate());
        if (dateEdit->lineEdit()->text().isEmpty()) {
            dateEdit->setDate(QDate::currentDate());
        }
        dateEdit->lineEdit()->selectAll();

    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void ActionListDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                      const QModelIndex &index) const
{
    if (index.data(Qt::EditRole).type()==QVariant::Date) {
        KDateEdit *dateEdit = static_cast<KDateEdit*>(editor);
        model->setData(index, dateEdit->date());
    } else if (index.data(Zanshin::DataTypeRole).toInt() == Zanshin::CategoryType) {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        QStringList currentCategories = comboBox->currentText().split(", ");
        model->setData(index, currentCategories);

    } else if (index.data(Zanshin::DataTypeRole).toInt() == Zanshin::ProjectType) {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        if (comboBox->currentIndex() == -1) {
            return;
        }
        QModelIndex idx = comboBox->model()->index(comboBox->currentIndex(), 0);
        if (idx.isValid()) {
            model->setData(index, idx.data(Zanshin::UidRole));
        }
    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}

void ActionListDelegate::updateEditorGeometry(QWidget *editor,
                                              const QStyleOptionViewItem &option,
                                              const QModelIndex &index) const
{
    QStyleOptionViewItemV4 opt = option;

    if (index.column()==0) {
        Zanshin::ItemType type = (Zanshin::ItemType)index.data(Zanshin::ItemTypeRole).toInt();

        if (type == Zanshin::StandardTodo && index.column()==0) {
            opt.rect.setLeft(opt.rect.left()+32);
        }
    }

    QStyledItemDelegate::updateEditorGeometry(editor, opt, index);
}

bool ActionListDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                     const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QStyleOptionViewItemV4 opt = option;

    if (index.column()==0) {
        Zanshin::ItemType type = (Zanshin::ItemType)index.data(Zanshin::ItemTypeRole).toInt();

        if (type == Zanshin::StandardTodo && index.column()==0) {
            opt.rect.setLeft(opt.rect.left()+32);
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, opt, index);
}

void ActionListDelegate::onCompleterActivated(const QModelIndex &index)
{
    QCompleter *completer = static_cast<QCompleter*>(sender());
    QComboBox *comboBox = static_cast<QComboBox*>(completer->widget());

    KModelIndexProxyMapper *mapper = new KModelIndexProxyMapper(comboBox->model(), index.model(), this);
    QModelIndex mapperIndex = mapper->mapRightToLeft(index);

    comboBox->setCurrentIndex(mapperIndex.row());
    QVariant value = mapperIndex.data(Qt::CheckStateRole);
    if (!value.isValid()) {
        return;
    }
    Qt::CheckState state = (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked
                            ? Qt::Unchecked : Qt::Checked);
    comboBox->model()->setData(mapperIndex, state, Qt::CheckStateRole);
}
