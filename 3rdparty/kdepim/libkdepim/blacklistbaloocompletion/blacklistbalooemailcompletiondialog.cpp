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

#include "blacklistbalooemailcompletiondialog.h"
#include "blacklistbalooemailcompletionwidget.h"
#include <KLocalizedString>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDialogButtonBox>

using namespace KPIM;
class KPIM::BlackListBalooEmailCompletionDialogPrivate
{
public:
    BlackListBalooEmailCompletionDialogPrivate()
        : mBlackListWidget(Q_NULLPTR)
    {

    }
    BlackListBalooEmailCompletionWidget *mBlackListWidget;
};

BlackListBalooEmailCompletionDialog::BlackListBalooEmailCompletionDialog(QWidget *parent)
    : QDialog(parent),
      d(new KPIM::BlackListBalooEmailCompletionDialogPrivate)
{
    setWindowTitle(i18n("Blacklist Email Completion"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    okButton->setDefault(true);
    d->mBlackListWidget = new BlackListBalooEmailCompletionWidget(this);
    d->mBlackListWidget->load();
    d->mBlackListWidget->setObjectName(QStringLiteral("blacklistwidget"));
    mainLayout->addWidget(d->mBlackListWidget);

    mainLayout->addWidget(buttonBox);

    setModal(true);
    connect(okButton, &QAbstractButton::clicked, this, &BlackListBalooEmailCompletionDialog::slotSave);
    readConfig();
}

BlackListBalooEmailCompletionDialog::~BlackListBalooEmailCompletionDialog()
{
    writeConfig();
    delete d;
}

void BlackListBalooEmailCompletionDialog::setEmailBlackList(const QStringList &list)
{
    d->mBlackListWidget->setEmailBlackList(list);
}

void BlackListBalooEmailCompletionDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "BlackListBalooEmailCompletionDialog");
    const QSize sizeDialog = group.readEntry("Size", QSize(800, 600));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

void BlackListBalooEmailCompletionDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "BlackListBalooEmailCompletionDialog");
    group.writeEntry("Size", size());
}

void BlackListBalooEmailCompletionDialog::slotSave()
{
    d->mBlackListWidget->save();
    accept();
}

