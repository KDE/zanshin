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

#include "actionlistcombobox.h"
#include "combomodel.h"
#include "kdateedit.h"
#include "modelstack.h"
#include "todomodel.h"

using namespace KPIM;

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
    ActionListComboBox *comboBox = new ActionListComboBox(isCategory, parent);
    comboBox->setEditable(true);
    comboBox->view()->setTextElideMode(Qt::ElideNone);
    ComboModel *comboModel = static_cast<ComboModel*>(model);
    if (isCategory) {
        comboModel->setSelectedItems(selectedIndex.data(TodoModel::CategoriesRole).value<QStringList>());
    } else {
        comboModel->setSelectedItems(QStringList(selectedIndex.data(Qt::DisplayRole).value<QString>()));
    }
    comboBox->setModel(model);
    QCompleter *completer = new QCompleter(comboBox);
    completer->setModel(model);
    completer->setWidget(comboBox);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    comboBox->setCompleter(completer);
    if (isCategory) {
        ActionListComboBox *completerBox = new ActionListComboBox(isCategory, parent);
        completer->setPopup(completerBox->view());
        completer->setCompletionRole(ComboModel::LastPathPartRole);
        comboBox->setEditText(selectedIndex.data(TodoModel::CategoriesRole).value<QStringList>().join(", "));
        connect(comboBox, SIGNAL(activated(int)), model, SLOT(checkItem(int)));
        connect(comboBox, SIGNAL(activated(int)), comboBox, SLOT(showItem()));
        connect(completer->popup(), SIGNAL(activated(const QModelIndex&)), model, SLOT(checkItem(const QModelIndex&)));
        connect(completer->popup(), SIGNAL(clicked(const QModelIndex&)), comboBox, SLOT(showItem()));
    }

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
    KDateEdit *dateEdit = qobject_cast<KDateEdit*>(editor);

    if (dateEdit) {
        model->setData(index, dateEdit->date());

    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}
