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
#include "actionlistcompletermodel.h"
#include "actionlistcombobox.h"
#include "combomodel.h"
#include "kdescendantsproxymodel.h"
#include "kdateedit.h"
#include "modelstack.h"
#include "todomodel.h"

using namespace KPIM;
Q_DECLARE_METATYPE(QItemSelectionModel*);

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

    TodoModel::ItemType type = (TodoModel::ItemType)index.data(TodoModel::ItemTypeRole).toInt();

    if (!isInFocus(index)) {
        res.setHeight(32);
    } else if (type!=TodoModel::StandardTodo) {
        res.setHeight(24);
    }

    return res;
}

void ActionListDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    TodoModel::ItemType type = (TodoModel::ItemType)index.data(TodoModel::ItemTypeRole).toInt();

    QStyleOptionViewItemV4 opt = option;

    if (!isInFocus(index)) {
        opt.decorationSize = QSize(1, 1);
        opt.displayAlignment = Qt::AlignHCenter|Qt::AlignBottom;
        opt.font.setItalic(true);
        opt.font.setPointSizeF(opt.font.pointSizeF()*0.75);

    } else if (type!=TodoModel::StandardTodo) {
        opt.decorationSize = QSize(22, 22);
        opt.font.setWeight(QFont::Bold);

    } else if (index.parent().isValid()) {
        if (index.row()%2==0) {
            opt.features|= QStyleOptionViewItemV4::Alternate;
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
    TodoModel::ItemType type = (TodoModel::ItemType)index.data(TodoModel::ItemTypeRole).toInt();

    if (type!=TodoModel::StandardTodo) {
        return KCalCore::Todo::Ptr();
    }

    Akonadi::Item item = index.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    if (!item.isValid() || !item.hasPayload<KCalCore::Todo::Ptr>()) {
        return KCalCore::Todo::Ptr();
    }

    return item.payload<KCalCore::Todo::Ptr>();
}

bool ActionListDelegate::isInFocus(const QModelIndex &/*index*/) const
{
    return true;
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
    } else if (index.data(TodoModel::DataTypeRole).toInt() == TodoModel::CategoryType) {
        return createComboBox(m_models->categoriesComboModel(), parent, index, true);
    } else if (index.data(TodoModel::DataTypeRole).toInt() == TodoModel::ProjectType) {
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
    completer->setWidget(comboBox);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);

    if (isCategory) {
        comboBox->setAutoHidePopupEnabled(true);
        QItemSelectionModel *checkModel = new QItemSelectionModel(model, comboBox);
        ActionListCheckableModel *checkable = new ActionListCheckableModel(comboBox);
        checkable->setSourceModel(model);
        checkable->setSelectionModel(checkModel);

        QStringList categories = selectedIndex.data(TodoModel::CategoriesRole).value<QStringList>();
        for (int i = 0; i < model->rowCount(); ++i) {
            QModelIndex index = model->index(i, 0);
            foreach (QString item, categories) {
                if (index.data(Qt::DisplayRole).toString().contains(item)) {
                    checkModel->select(index, QItemSelectionModel::Toggle);
                }
            }
        }
        QVariant var;
        var.setValue(checkModel);
        comboBox->setProperty("selectionModel", var);
        comboBox->setModel(checkable);

        ActionListCompleterModel *completerModel = new ActionListCompleterModel(checkModel, completer);
        completerModel->setSourceModel(checkable);
        completer->setModel(completerModel);
        ActionListComboBox *completerBox = new ActionListComboBox(parent);
        completerBox->setAutoHidePopupEnabled(true);
        completer->setPopup(completerBox->view());
    } else {
        comboBox->setModel(model);
        completer->setModel(model);
    }
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
    } else if (index.data(TodoModel::DataTypeRole).toInt() == TodoModel::CategoryType) {
        QStyledItemDelegate::setModelData(editor, model, index);
    } else if (index.data(TodoModel::DataTypeRole).toInt() == TodoModel::ProjectType) {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        kDebug() << comboBox->currentText();
        QStyledItemDelegate::setModelData(editor, model, index);
    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}
