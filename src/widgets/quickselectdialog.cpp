/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
   SPDX-FileCopyrightText: 2015 Franck Arrecot <franck.arrecot@gmail.com>
   * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

#include "moc_quickselectdialog.cpp"
