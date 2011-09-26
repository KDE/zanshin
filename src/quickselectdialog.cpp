/* This file is part of Zanshin Todo.

   Copyright 2009 Kevin Ottens <ervin@kde.org>

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

#include "quickselectdialog.h"

#include <KDE/KDebug>
#include <KDE/KLocale>
#include <krecursivefilterproxymodel.h>

#include <QtGui/QKeyEvent>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QTreeView>

#include "globaldefs.h"
#include "todocategoriesmodel.h"
#include "todotreemodel.h"

QuickSelectDialog::QuickSelectDialog(QWidget *parent, QAbstractItemModel *model, Zanshin::ApplicationMode mode, ActionType action)
    : KDialog(parent),
      m_label(0),
      m_tree(0),
      m_filter(new KRecursiveFilterProxyModel(this)),
      m_model(model),
      m_mode(mode)
{
    QString caption;

    if (mode==Zanshin::CategoriesMode) {
        switch (action) {
        case MoveAction:
            caption = i18n("Move Actions to Context");
            break;
        case CopyAction:
            caption = i18n("Copy Actions to Context");
            break;
        case JumpAction:
            caption = i18n("Jump to Context");
            break;
        }
    } else if (mode==Zanshin::ProjectMode) {
        switch (action) {
        case MoveAction:
            caption = i18n("Move Actions to Project");
            break;
        case CopyAction:
            caption = i18n("Copy Actions to Project");
            break;
        case JumpAction:
            caption = i18n("Jump to Project");
            break;
        }
    } else {
        kError() << "Shouldn't happen";
    }

    setCaption(caption);
    setButtons(Ok|Cancel);

    QWidget *page = mainWidget();
    page->setLayout(new QVBoxLayout(page));

    m_label = new QLabel(page);
    page->layout()->addWidget(m_label);

    m_tree = new QTreeView(page);
    m_tree->setSortingEnabled(true);
    m_tree->sortByColumn(0, Qt::AscendingOrder);
    page->layout()->addWidget(m_tree);

    m_filter->setDynamicSortFilter(true);
    m_filter->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_filter->setSourceModel(m_model);

    m_tree->setModel(m_filter);
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tree->setCurrentIndex(m_filter->index(0, 0));
    m_tree->expandAll();
    m_tree->setFocus(Qt::OtherFocusReason);

    m_tree->installEventFilter(this);
    applyPattern(QString());
}

QString QuickSelectDialog::selectedId() const
{
    if (m_mode==Zanshin::ProjectMode) {
        return projectSelectedId();
    } else {
        return categorySelectedId();
    }
}

Zanshin::ItemType QuickSelectDialog::selectedType() const
{
    QModelIndex index = m_tree->selectionModel()->currentIndex();
    return (Zanshin::ItemType)index.data(Zanshin::ItemTypeRole).toInt();
}

QString QuickSelectDialog::categorySelectedId() const
{
    QModelIndex index = m_tree->selectionModel()->currentIndex();
    return index.data(Zanshin::CategoryPathRole).toString();
}

QString QuickSelectDialog::projectSelectedId() const
{
    QModelIndex index = m_tree->selectionModel()->currentIndex();
    return index.data(Zanshin::UidRole).toString();
}

Akonadi::Collection QuickSelectDialog::collection() const
{
    QModelIndex index = m_tree->selectionModel()->currentIndex();
    Akonadi::Collection collection;
    Zanshin::ItemType type = (Zanshin::ItemType)index.data(Zanshin::ItemTypeRole).toInt();
    if (type == Zanshin::Collection) {
        collection = index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
    } else {
        const Akonadi::Item item = index.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
        collection = item.parentCollection();
    }
    return collection;
}

QModelIndex QuickSelectDialog::selectedIndex() const
{
    return m_tree->selectionModel()->currentIndex();
}

QString QuickSelectDialog::pattern() const
{
    return m_filter->filterRegExp().pattern();
}

void QuickSelectDialog::applyPattern(const QString &pattern)
{
    if (pattern.isEmpty()) {
        QString type = i18n("projects");
        if (m_mode==Zanshin::CategoriesMode) {
            type = i18n("contexts");
        }

        m_label->setText(i18n("You can start typing to filter the list of %1.", type));
    } else {
        m_label->setText(i18n("Path: %1", pattern));
    }

    m_filter->setFilterFixedString(pattern);
    m_tree->expandAll();
}

bool QuickSelectDialog::eventFilter(QObject *, QEvent *ev)
{
    if (ev->type() == QEvent::KeyPress) {
        const QKeyEvent * const event = static_cast<QKeyEvent*>(ev);
        QString p = pattern();

        switch (event->key()) {
        case Qt::Key_Backspace:
            p.chop(1);
            break;
        case Qt::Key_Delete:
            p = QString();
            break;
        default:
            if (event->text().contains("^(\\w| )+$")) {
                p+= event->text();
            }
            break;
        }

        applyPattern(p);
    }
    return false;
}
