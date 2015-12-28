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

#include "blacklistbalooemailcompletionwidgettest.h"
#include "../blacklistbalooemailcompletionwidget.h"
#include "../blacklistbalooemaillist.h"
#include <KListWidgetSearchLine>
#include <QLabel>
#include <klineedit.h>
#include <qpushbutton.h>
#include <qtest.h>
#include <addressline/blacklistbaloocompletion/blacklistbalooemailwarning.h>

BlackListBalooEmailCompletionWidgetTest::BlackListBalooEmailCompletionWidgetTest(QObject *parent)
    : QObject(parent)
{

}

BlackListBalooEmailCompletionWidgetTest::~BlackListBalooEmailCompletionWidgetTest()
{

}

void BlackListBalooEmailCompletionWidgetTest::shouldHaveDefaultValue()
{
    KPIM::BlackListBalooEmailCompletionWidget widget;
    widget.show();
    QTest::qWaitForWindowExposed(&widget);
    QLabel *searchLabel = widget.findChild<QLabel *>(QStringLiteral("search_label"));
    QVERIFY(searchLabel);

    KLineEdit *searchLineEdit = widget.findChild<KLineEdit *>(QStringLiteral("search_lineedit"));
    QVERIFY(searchLineEdit);
    QVERIFY(searchLineEdit->isClearButtonShown());
    QVERIFY(searchLineEdit->trapReturnKey());
    QVERIFY(searchLineEdit->text().isEmpty());

    QPushButton *seachButton = widget.findChild<QPushButton *>(QStringLiteral("search_button"));
    QVERIFY(seachButton);
    QVERIFY(!seachButton->isEnabled());

    QLabel *moreResult = widget.findChild<QLabel *>(QStringLiteral("moreresultlabel"));
    QVERIFY(moreResult);
    QVERIFY(!moreResult->isVisible());

    QLabel *mNumberOfEmailsFound = widget.findChild<QLabel *>(QStringLiteral("numberofemailsfound"));
    QVERIFY(mNumberOfEmailsFound);
    QVERIFY(mNumberOfEmailsFound->text().isEmpty());

    QPushButton *showAllBlackListedEmails = widget.findChild<QPushButton *>(QStringLiteral("show_blacklisted_email_button"));
    QVERIFY(showAllBlackListedEmails);

    KPIM::BlackListBalooEmailList *emailList = widget.findChild<KPIM::BlackListBalooEmailList *>(QStringLiteral("email_list"));
    QVERIFY(emailList);

    QPushButton *selectButton = widget.findChild<QPushButton *>(QStringLiteral("select_email"));
    QVERIFY(selectButton);
    QVERIFY(!selectButton->isEnabled());
    QPushButton *unselectButton = widget.findChild<QPushButton *>(QStringLiteral("unselect_email"));
    QVERIFY(unselectButton);
    QVERIFY(!unselectButton->isEnabled());

    QLabel *excludeDomainLabel = widget.findChild<QLabel *>(QStringLiteral("domain_label"));
    QVERIFY(excludeDomainLabel);

    KLineEdit *excludeDomainLineEdit = widget.findChild<KLineEdit *>(QStringLiteral("domain_lineedit"));
    QVERIFY(excludeDomainLineEdit);
    QVERIFY(excludeDomainLineEdit->trapReturnKey());
    QVERIFY(excludeDomainLineEdit->text().isEmpty());
    QVERIFY(excludeDomainLineEdit->isClearButtonShown());
    QVERIFY(!excludeDomainLineEdit->placeholderText().isEmpty());

    KListWidgetSearchLine *searchInResult = widget.findChild<KListWidgetSearchLine *>(QStringLiteral("searchinresultlineedit"));
    QVERIFY(searchInResult);
    QVERIFY(!searchInResult->placeholderText().isEmpty());
    QVERIFY(searchInResult->text().isEmpty());
    QVERIFY(searchInResult->isClearButtonEnabled());

    KPIM::BlackListBalooEmailWarning *blackListWarning = widget.findChild<KPIM::BlackListBalooEmailWarning *>(QStringLiteral("backlistwarning"));
    QVERIFY(blackListWarning);
}

void BlackListBalooEmailCompletionWidgetTest::shouldEnablePushButtonWhenTestSizeSupperiorToTwo()
{
    KPIM::BlackListBalooEmailCompletionWidget widget;
    KLineEdit *searchLineEdit = widget.findChild<KLineEdit *>(QStringLiteral("search_lineedit"));
    QPushButton *seachButton = widget.findChild<QPushButton *>(QStringLiteral("search_button"));
    QVERIFY(!seachButton->isEnabled());
    searchLineEdit->setText(QStringLiteral("fo"));
    QVERIFY(!seachButton->isEnabled());
    searchLineEdit->setText(QStringLiteral("foo"));
    QVERIFY(seachButton->isEnabled());

    searchLineEdit->setText(QStringLiteral("o  "));
    QVERIFY(!seachButton->isEnabled());
    searchLineEdit->setText(QStringLiteral(" o "));
    QVERIFY(!seachButton->isEnabled());
}

void BlackListBalooEmailCompletionWidgetTest::shouldChangeEnableSelectUnSelectButton()
{
    KPIM::BlackListBalooEmailCompletionWidget widget;

    QPushButton *selectButton = widget.findChild<QPushButton *>(QStringLiteral("select_email"));
    QVERIFY(!selectButton->isEnabled());

    QPushButton *unselectButton = widget.findChild<QPushButton *>(QStringLiteral("unselect_email"));
    QVERIFY(!unselectButton->isEnabled());

    KPIM::BlackListBalooEmailList *emailList = widget.findChild<KPIM::BlackListBalooEmailList *>(QStringLiteral("email_list"));
    emailList->setEmailFound(QStringList() << QStringLiteral("foo") << QStringLiteral("bla") << QStringLiteral("bli"));

    emailList->selectAll();
    QVERIFY(unselectButton->isEnabled());
    QVERIFY(selectButton->isEnabled());

    emailList->clearSelection();
    QVERIFY(!unselectButton->isEnabled());
    QVERIFY(!selectButton->isEnabled());

}

QTEST_MAIN(BlackListBalooEmailCompletionWidgetTest)
