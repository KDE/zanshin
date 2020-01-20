/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>
   Copyright 2015 Franck Arrecot <franck.arrecot@gmail.com>

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

#include <QDialogButtonBox>
#include <QEvent>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QVBoxLayout>

#include <KLocalizedString>

using namespace Widgets;

QuickSelectDialog::QuickSelectDialog(QWidget *parent)
    : QDialog(parent),
      m_model(nullptr),
      m_filterProxyModel(new QSortFilterProxyModel(this)),
      m_label(new QLabel(this)),
      m_tree(new QTreeView(this))
{
    setWindowTitle(i18n("Quick Select Dialog"));

    m_label->setText(i18n("You can start typing to filter the list of available pages"));
    m_filterProxyModel->setRecursiveFilteringEnabled(true);

    m_tree->setModel(m_filterProxyModel);
    m_tree->setObjectName(QStringLiteral("pagesView"));
    m_tree->header()->hide();
    m_tree->expandAll();
    m_tree->setFocus();
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tree->setSortingEnabled(false);
    m_tree->installEventFilter(this);

    auto buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_label);
    mainLayout->addWidget(m_tree);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QuickSelectDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QuickSelectDialog::reject);
}

int QuickSelectDialog::exec()
{
    return QDialog::exec();
}

QPersistentModelIndex QuickSelectDialog::selectedIndex() const
{
    QModelIndex selected = m_tree->currentIndex();
    return m_filterProxyModel->mapToSource(selected);
}

void QuickSelectDialog::applyFilterChanged(const QString &textFilter)
{
    if (textFilter.isEmpty())
        m_label->setText(i18n("You can start typing to filter the list of available pages"));
    else
        m_label->setText(i18n("Path: %1", textFilter));

    m_filterProxyModel->setFilterRegularExpression(QRegularExpression(textFilter, QRegularExpression::CaseInsensitiveOption));
    m_tree->expandAll();
}

bool QuickSelectDialog::eventFilter(QObject *, QEvent *ev)
{
    if (ev->type() == QEvent::KeyPress) {
        auto event = static_cast<QKeyEvent*>(ev);
        auto filter = m_filterProxyModel->filterRegularExpression().pattern();

        switch (event->key()) {
        case Qt::Key_Backspace:
            filter.chop(1);
            break;
        case Qt::Key_Delete:
            filter = QString();
            break;
        default:
            if (event->text().contains(QRegularExpression(QStringLiteral("^(\\w| )+$")))) {
                filter += event->text();
            }
            break;
        }

        applyFilterChanged(filter);
    }
    return false;
}

void QuickSelectDialog::setModel(QAbstractItemModel *model)
{
    if (model == m_model)
        return;

    m_model = model;
    m_filterProxyModel->setSourceModel(m_model);
    m_tree->expandAll();
}
