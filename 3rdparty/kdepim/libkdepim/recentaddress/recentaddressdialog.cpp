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

#include "recentaddressdialog.h"
#include "recentaddresswidget.h"
#include "recentaddresses.h"

#include <KConfigGroup>
#include <KSharedConfig>
#include <QCoreApplication>
#include <QLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <KLocalizedString>
using namespace KPIM;

RecentAddressDialog::RecentAddressDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Edit Recent Addresses"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->setObjectName(QStringLiteral("buttonbox"));
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mRecentAddressWidget = new RecentAddressWidget(this);
    mRecentAddressWidget->setObjectName(QStringLiteral("recentaddresswidget"));

    mainLayout->addWidget(mRecentAddressWidget);
    mainLayout->addWidget(buttonBox);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &RecentAddressDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &RecentAddressDialog::reject);
    okButton->setDefault(true);
    setModal(true);
    readConfig();
}

RecentAddressDialog::~RecentAddressDialog()
{
    writeConfig();
}

void RecentAddressDialog::setAddresses(const QStringList &addrs)
{
    mRecentAddressWidget->setAddresses(addrs);
}

void RecentAddressDialog::storeAddresses(KConfig *config)
{
    mRecentAddressWidget->storeAddresses(config);
}

bool RecentAddressDialog::wasChanged() const
{
    return mRecentAddressWidget->wasChanged();
}

void RecentAddressDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "RecentAddressDialog");
    const QSize size = group.readEntry("Size", QSize(600, 400));
    if (size.isValid()) {
        resize(size);
    }
}

void RecentAddressDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "RecentAddressDialog");
    group.writeEntry("Size", size());
    group.sync();
}
