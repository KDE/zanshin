/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "recentaddresswidget.h"
#include "recentaddresses.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLineEdit>
#include <QPushButton>
#include <KMessageBox>
#include <KLocalizedString>

#include <QCoreApplication>
#include <QLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QKeyEvent>

using namespace KPIM;
RecentAddressWidget::RecentAddressWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    mLineEdit = new KLineEdit(this);
    mLineEdit->setObjectName(QStringLiteral("line_edit"));
    layout->addWidget(mLineEdit);

    mLineEdit->setTrapReturnKey(true);
    mLineEdit->installEventFilter(this);

    connect(mLineEdit, &KLineEdit::textChanged, this, &RecentAddressWidget::slotTypedSomething);
    connect(mLineEdit, &KLineEdit::returnPressed, this, &RecentAddressWidget::slotAddItem);

    QHBoxLayout *hboxLayout = new QHBoxLayout;

    QVBoxLayout *btnsLayout = new QVBoxLayout;
    btnsLayout->addStretch();

    mNewButton = new QPushButton(QIcon::fromTheme(QStringLiteral("list-add")), i18n("&Add"), this);
    mNewButton->setObjectName(QStringLiteral("new_button"));
    connect(mNewButton, &QPushButton::clicked, this, &RecentAddressWidget::slotAddItem);
    btnsLayout->insertWidget(0, mNewButton);

    mRemoveButton = new QPushButton(QIcon::fromTheme(QStringLiteral("list-remove")), i18n("&Remove"), this);
    mRemoveButton->setObjectName(QStringLiteral("remove_button"));
    mRemoveButton->setEnabled(false);
    connect(mRemoveButton, &QPushButton::clicked, this, &RecentAddressWidget::slotRemoveItem);
    btnsLayout->insertWidget(1, mRemoveButton);

    mListView = new QListWidget(this);
    mListView->setObjectName(QStringLiteral("list_view"));
    mListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mListView->setSortingEnabled(true);
    hboxLayout->addWidget(mListView);
    hboxLayout->addLayout(btnsLayout);
    layout->addLayout(hboxLayout);
    connect(mListView, &QListWidget::itemSelectionChanged, this, &RecentAddressWidget::slotSelectionChanged);
    // maybe supplied lineedit has some text already
    slotTypedSomething(mLineEdit->text());
}

RecentAddressWidget::~RecentAddressWidget()
{

}

void RecentAddressWidget::slotTypedSomething(const QString &text)
{
    if (mListView->currentItem()) {
        if (mListView->currentItem()->text() != mLineEdit->text() && !mLineEdit->text().isEmpty()) {
            // IMHO changeItem() shouldn't do anything with the value
            // of currentItem() ... like changing it or emitting signals ...
            // but TT disagree with me on this one (it's been that way since ages ... grrr)
            bool block = mListView->signalsBlocked();
            mListView->blockSignals(true);
            QListWidgetItem *currentIndex = mListView->currentItem();
            if (currentIndex) {
                currentIndex->setText(text);
                mDirty = true;
            }
            mListView->blockSignals(block);
        }
    }
}

void RecentAddressWidget::slotAddItem()
{
    if (mListView->count() > 0) {
        const QString text = mListView->item(0)->text();
        if (text.isEmpty()) {
            return;
        }
    }
    mListView->blockSignals(true);
    mListView->insertItem(0, QString());
    mListView->blockSignals(false);
    mListView->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);
    mLineEdit->setFocus();
    mDirty = true;
    updateButtonState();
}

void RecentAddressWidget::slotRemoveItem()
{
    QList<QListWidgetItem *> selectedItems = mListView->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }
    if (KMessageBox::Yes == KMessageBox::questionYesNo(this, i18np("Do you want to remove this email address?", "Do you want to remove %1 email addresses?", selectedItems.count()), i18n("Remove"))) {
        Q_FOREACH (QListWidgetItem *item, selectedItems) {
            delete mListView->takeItem(mListView->row(item));
        }
        mDirty = true;
        updateButtonState();
    }
}

void RecentAddressWidget::updateButtonState()
{
    QList<QListWidgetItem *> selectedItems = mListView->selectedItems();
    const int numberOfElementSelected(selectedItems.count());
    mRemoveButton->setEnabled(numberOfElementSelected);
    mNewButton->setEnabled(numberOfElementSelected <= 1);
    mLineEdit->setEnabled(numberOfElementSelected <= 1);

    if (numberOfElementSelected == 1) {
        const QString text = mListView->currentItem()->text();
        if (text != mLineEdit->text()) {
            mLineEdit->setText(text);
        }
    } else {
        mLineEdit->clear();
    }
}

void RecentAddressWidget::slotSelectionChanged()
{
    updateButtonState();
}

void RecentAddressWidget::setAddresses(const QStringList &addrs)
{
    mListView->clear();
    mListView->addItems(addrs);
}

bool RecentAddressWidget::eventFilter(QObject *o, QEvent *e)
{
    if (o == mLineEdit && e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = (QKeyEvent *)e;
        if (keyEvent->key() == Qt::Key_Down ||
                keyEvent->key() == Qt::Key_Up) {
            return ((QObject *)mListView)->event(e);
        }
    }

    return false;
}

void RecentAddressWidget::storeAddresses(KConfig *config)
{
    const int numberOfItem(mListView->count());
    for (int i = 0; i < numberOfItem; ++i) {
        KPIM::RecentAddresses::self(config)->add(mListView->item(i)->text());
    }
}

bool RecentAddressWidget::wasChanged() const
{
    return mDirty;
}
