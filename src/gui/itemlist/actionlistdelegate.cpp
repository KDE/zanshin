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

#include <kdescendantsproxymodel.h>
#include "utils/kdateedit.h"
#include <klocale.h>
#include <kglobal.h>
#include <kmodelindexproxymapper.h>

#include "globaldefs.h"
#include "core/modelstack.h"
#include "core/pimitem.h"
#include <core/pimitemfactory.h>

using namespace KPIM;
Q_DECLARE_METATYPE(QItemSelectionModel*)

ActionListDelegate::ActionListDelegate(ModelStack *models, QObject *parent)
    : QStyledItemDelegate(parent),
    m_models(models)
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

QString ActionListDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    switch (value.userType()) {
    case QVariant::Date:
        return KGlobal::locale()->formatDate(value.toDate(), KLocale::FancyLongDate);
    case QVariant::Time:
        return KGlobal::locale()->formatLocaleTime(value.toTime(), KLocale::TimeWithoutSeconds);
    case QVariant::DateTime:
        return KGlobal::locale()->formatDateTime(value.toDateTime());
    default:
        return QStyledItemDelegate::displayText(value, locale);
    }
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


bool ActionListDelegate::isCompleted(const QModelIndex &index) const
{
    PimItem::Ptr pimitem(PimItemFactory::getItem(index.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>()));
    if (pimitem.isNull()) {
        return false;
    }
    return pimitem->getStatus() == PimItem::Complete;
}

bool ActionListDelegate::isOverdue(const QModelIndex &index) const
{
    PimItem::Ptr pimitem(PimItemFactory::getItem(index.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>()));
    if (pimitem.isNull()) {
        return false;
    }
    return pimitem->getStatus() == PimItem::Attention;
}

QWidget *ActionListDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    if (index.data(Qt::EditRole).type()==QVariant::Date) {
        return new KDateEdit(parent);
    } else {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
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
