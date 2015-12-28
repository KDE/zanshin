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

#include "blacklistbalooemailcompletionwidget.h"
#include "blacklistbalooemaillist.h"
#include "blacklistbalooemailsearchjob.h"
#include "blacklistbalooemailutil.h"
#include "blacklistbalooemailwarning.h"

#include <KLocalizedString>
#include <KLineEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QLabel>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KListWidgetSearchLine>
#include "libkdepim_debug.h"

using namespace KPIM;
BlackListBalooEmailCompletionWidget::BlackListBalooEmailCompletionWidget(QWidget *parent)
    : QWidget(parent),
      mLimit(500)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QHBoxLayout *searchLayout = new QHBoxLayout;
    mainLayout->addLayout(searchLayout);

    QLabel *lab = new QLabel(i18n("Search email:"));
    lab->setObjectName(QStringLiteral("search_label"));
    searchLayout->addWidget(lab);

    mSearchLineEdit = new KLineEdit;
    mSearchLineEdit->setPlaceholderText(i18n("Research is done from 3 characters"));
    mSearchLineEdit->setFocus();
    mSearchLineEdit->setClearButtonShown(true);
    mSearchLineEdit->setTrapReturnKey(true);
    mSearchLineEdit->setObjectName(QStringLiteral("search_lineedit"));
    connect(mSearchLineEdit, &KLineEdit::returnPressed, this, &BlackListBalooEmailCompletionWidget::slotCheckIfUpdateBlackListIsNeeded);
    searchLayout->addWidget(mSearchLineEdit);

    mSearchButton = new QPushButton(QIcon::fromTheme(QStringLiteral("edit-find")), i18n("Search"));
    mSearchButton->setObjectName(QStringLiteral("search_button"));
    connect(mSearchButton, &QAbstractButton::clicked, this, &BlackListBalooEmailCompletionWidget::slotCheckIfUpdateBlackListIsNeeded);
    mSearchButton->setEnabled(false);
    searchLayout->addWidget(mSearchButton);

    mShowAllBlackListedEmails = new QPushButton(i18n("Show Blacklisted Emails"));
    mShowAllBlackListedEmails->setObjectName(QStringLiteral("show_blacklisted_email_button"));
    connect(mShowAllBlackListedEmails, &QAbstractButton::clicked, this, &BlackListBalooEmailCompletionWidget::slotShowAllBlacklistedEmail);
    searchLayout->addWidget(mShowAllBlackListedEmails);

    mEmailList = new BlackListBalooEmailList;
    mEmailList->setObjectName(QStringLiteral("email_list"));
    mainLayout->addWidget(mEmailList);

    QHBoxLayout *searchLineLayout = new QHBoxLayout;
    mainLayout->addLayout(searchLineLayout);
    mSearchInResultLineEdit = new KListWidgetSearchLine(this, mEmailList);
    mSearchInResultLineEdit->setObjectName(QStringLiteral("searchinresultlineedit"));
    mSearchInResultLineEdit->setClearButtonEnabled(true);
    mSearchInResultLineEdit->setPlaceholderText(i18n("Search in result..."));

    searchLineLayout->addStretch(0);
    mNumberOfEmailsFound = new QLabel;
    mNumberOfEmailsFound->setObjectName(QStringLiteral("numberofemailsfound"));

    searchLineLayout->addWidget(mNumberOfEmailsFound);
    searchLineLayout->addWidget(mSearchInResultLineEdit);

    QHBoxLayout *selectElementLayout = new QHBoxLayout;
    mainLayout->addLayout(selectElementLayout);
    mSelectButton = new QPushButton(i18n("&Select"), this);
    mSelectButton->setObjectName(QStringLiteral("select_email"));
    connect(mSelectButton, &QAbstractButton::clicked, this, &BlackListBalooEmailCompletionWidget::slotSelectEmails);
    selectElementLayout->addWidget(mSelectButton);

    mUnselectButton = new QPushButton(i18n("&Unselect"), this);
    mUnselectButton->setObjectName(QStringLiteral("unselect_email"));
    connect(mUnselectButton, &QAbstractButton::clicked, this, &BlackListBalooEmailCompletionWidget::slotUnselectEmails);
    selectElementLayout->addWidget(mUnselectButton);

    mMoreResult = new QLabel(i18n("<qt><a href=\"more_result\">More result...</a></qt>"), this);
    mMoreResult->setObjectName(QStringLiteral("moreresultlabel"));
    selectElementLayout->addWidget(mMoreResult);

    mMoreResult->setContextMenuPolicy(Qt::NoContextMenu);
    connect(mMoreResult, &QLabel::linkActivated, this, &BlackListBalooEmailCompletionWidget::slotLinkClicked);
    mMoreResult->setVisible(false);
    selectElementLayout->addStretch(1);

    connect(mSearchLineEdit, &QLineEdit::textChanged, this, &BlackListBalooEmailCompletionWidget::slotSearchLineEditChanged);

    QHBoxLayout *excludeDomainLayout = new QHBoxLayout;
    excludeDomainLayout->setMargin(0);
    mainLayout->addLayout(excludeDomainLayout);

    QLabel *excludeDomainLabel = new QLabel(i18n("Exclude domain names:"));
    excludeDomainLabel->setObjectName(QStringLiteral("domain_label"));
    excludeDomainLayout->addWidget(excludeDomainLabel);

    mExcludeDomainLineEdit = new KLineEdit;
    excludeDomainLayout->addWidget(mExcludeDomainLineEdit);
    mExcludeDomainLineEdit->setObjectName(QStringLiteral("domain_lineedit"));
    mExcludeDomainLineEdit->setClearButtonShown(true);
    mExcludeDomainLineEdit->setTrapReturnKey(true);
    mExcludeDomainLineEdit->setPlaceholderText(i18n("Separate domain with \'%1\'", QLatin1Char(',')));

    mBlackListWarning = new BlackListBalooEmailWarning(this);
    mBlackListWarning->setObjectName(QStringLiteral("backlistwarning"));
    mainLayout->addWidget(mBlackListWarning);
    connect(mBlackListWarning, &BlackListBalooEmailWarning::newSearch, this, &BlackListBalooEmailCompletionWidget::slotSearch);
    connect(mBlackListWarning, &BlackListBalooEmailWarning::saveChanges, this, &BlackListBalooEmailCompletionWidget::slotSaveChanges);

    connect(mEmailList, &QListWidget::itemSelectionChanged, this, &BlackListBalooEmailCompletionWidget::slotSelectionChanged);
    slotSelectionChanged();
}

BlackListBalooEmailCompletionWidget::~BlackListBalooEmailCompletionWidget()
{

}

void BlackListBalooEmailCompletionWidget::slotSelectionChanged()
{
    mSelectButton->setEnabled(!mEmailList->selectedItems().isEmpty());
    mUnselectButton->setEnabled(!mEmailList->selectedItems().isEmpty());
}

void BlackListBalooEmailCompletionWidget::load()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("kpimbalooblacklist"));
    KConfigGroup group(config, "AddressLineEdit");
    const QStringList lst = group.readEntry("ExcludeDomain", QStringList());
    mEmailList->setExcludeDomain(lst);
    mExcludeDomainLineEdit->setText(lst.join(QStringLiteral(",")));
    mOriginalExcludeDomain = lst;
    slotSelectionChanged();
}

void BlackListBalooEmailCompletionWidget::slotUnselectEmails()
{
    Q_FOREACH (QListWidgetItem *item, mEmailList->selectedItems()) {
        item->setCheckState(Qt::Unchecked);
    }
}

void BlackListBalooEmailCompletionWidget::slotSelectEmails()
{
    Q_FOREACH (QListWidgetItem *item, mEmailList->selectedItems()) {
        item->setCheckState(Qt::Checked);
    }
}

void BlackListBalooEmailCompletionWidget::slotSearchLineEditChanged(const QString &text)
{
    mSearchButton->setEnabled(text.trimmed().count() > 2);
    hideMoreResultAndChangeLimit();
}

void BlackListBalooEmailCompletionWidget::hideMoreResultAndChangeLimit()
{
    mMoreResult->setVisible(false);
    mLimit = 500;
}

void BlackListBalooEmailCompletionWidget::slotSearch()
{
    const QString searchEmail = mSearchLineEdit->text().trimmed();
    if (searchEmail.length() > 2) {
        mSearchInResultLineEdit->clear();
        KPIM::BlackListBalooEmailSearchJob *job = new KPIM::BlackListBalooEmailSearchJob(this);
        job->setSearchEmail(searchEmail);
        job->setLimit(mLimit);
        connect(job, &BlackListBalooEmailSearchJob::emailsFound, this, &BlackListBalooEmailCompletionWidget::slotEmailFound);
        job->start();
    }
}

void BlackListBalooEmailCompletionWidget::slotEmailFound(const QStringList &list)
{
    mEmailList->setEmailFound(list);
    mMoreResult->setVisible(list.count() == mLimit);
    mEmailList->scrollToBottom();
    if (list.count() == 0) {
        mNumberOfEmailsFound->setText(i18n("No email found."));
    } else {
        mNumberOfEmailsFound->setText(i18np("1 email found", "%1 emails found", list.count()));
    }
}

void BlackListBalooEmailCompletionWidget::setEmailBlackList(const QStringList &list)
{
    mEmailList->setEmailBlackList(list);
}

void BlackListBalooEmailCompletionWidget::slotSaveChanges()
{
    //TODO avoid to save a lot.
    const QHash<QString, bool> result = mEmailList->blackListItemChanged();
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("kpimbalooblacklist"));
    KConfigGroup group(config, "AddressLineEdit");
    QStringList blackList = group.readEntry("BalooBackList", QStringList());
    KPIM::BlackListBalooEmailUtil util;
    util.initialBlackList(blackList);
    util.newBlackList(result);
    blackList = util.createNewBlackList();
    group.writeEntry("BalooBackList", blackList);
    group.sync();
    mEmailList->setEmailBlackList(blackList);
    slotSearch();
}

void BlackListBalooEmailCompletionWidget::slotCheckIfUpdateBlackListIsNeeded()
{
    const QHash<QString, bool> result = mEmailList->blackListItemChanged();
    if (result.isEmpty()) {
        slotSearch();
    } else {
        mBlackListWarning->animatedShow();
    }
}

void BlackListBalooEmailCompletionWidget::save()
{
    const QString domain = mExcludeDomainLineEdit->text().remove(QLatin1Char(' '));
    const QStringList newExcludeDomain = domain.split(QStringLiteral(","), QString::SkipEmptyParts);
    bool needToSave = (mOriginalExcludeDomain != newExcludeDomain);
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("kpimbalooblacklist"));
    KConfigGroup group(config, "AddressLineEdit");
    const QHash<QString, bool> result = mEmailList->blackListItemChanged();
    if (!result.isEmpty()) {
        needToSave = true;
        QStringList blackList = group.readEntry("BalooBackList", QStringList());
        KPIM::BlackListBalooEmailUtil util;
        util.initialBlackList(blackList);
        util.newBlackList(result);
        blackList = util.createNewBlackList();
        group.writeEntry("BalooBackList", blackList);
    }
    if (needToSave) {
        group.writeEntry("ExcludeDomain", newExcludeDomain);
        group.sync();
    }
}

void BlackListBalooEmailCompletionWidget::slotLinkClicked(const QString &link)
{
    if (link == QLatin1String("more_result")) {
        mLimit += 200;
        slotSearch();
    }
}

void BlackListBalooEmailCompletionWidget::slotShowAllBlacklistedEmail()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("kpimbalooblacklist"));
    KConfigGroup group(config, "AddressLineEdit");
    const QStringList balooBlackList = group.readEntry("BalooBackList", QStringList());
    slotEmailFound(balooBlackList);
}
