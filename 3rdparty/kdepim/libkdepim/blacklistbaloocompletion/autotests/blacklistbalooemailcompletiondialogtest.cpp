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

#include "blacklistbalooemailcompletiondialogtest.h"
#include "../blacklistbalooemailcompletiondialog.h"
#include "../blacklistbalooemaillist.h"
#include "../blacklistbalooemailcompletionwidget.h"
#include <QLabel>
#include <klineedit.h>
#include <QPushButton>
#include <qtest.h>

BlackListBalooEmailCompletionDialogTest::BlackListBalooEmailCompletionDialogTest(QObject *parent)
    : QObject(parent)
{

}

BlackListBalooEmailCompletionDialogTest::~BlackListBalooEmailCompletionDialogTest()
{

}

void BlackListBalooEmailCompletionDialogTest::shouldHaveDefaultValue()
{
    KPIM::BlackListBalooEmailCompletionDialog dlg;

    QLabel *searchLabel = dlg.findChild<QLabel *>(QStringLiteral("search_label"));
    QVERIFY(searchLabel);

    KLineEdit *searchLineEdit = dlg.findChild<KLineEdit *>(QStringLiteral("search_lineedit"));
    QVERIFY(searchLineEdit);
    QVERIFY(searchLineEdit->isClearButtonShown());
    QVERIFY(searchLineEdit->trapReturnKey());
    QVERIFY(searchLineEdit->text().isEmpty());

    QPushButton *seachButton = dlg.findChild<QPushButton *>(QStringLiteral("search_button"));
    QVERIFY(seachButton);
    QVERIFY(!seachButton->isEnabled());

    KPIM::BlackListBalooEmailList *emailList = dlg.findChild<KPIM::BlackListBalooEmailList *>(QStringLiteral("email_list"));
    QVERIFY(emailList);

    QPushButton *selectButton = dlg.findChild<QPushButton *>(QStringLiteral("select_email"));
    QVERIFY(selectButton);
    QPushButton *unselectButton = dlg.findChild<QPushButton *>(QStringLiteral("unselect_email"));
    QVERIFY(unselectButton);

}

void BlackListBalooEmailCompletionDialogTest::shouldEnablePushButtonWhenTestSizeSupperiorToTwo()
{
    KPIM::BlackListBalooEmailCompletionDialog dlg;
    KLineEdit *searchLineEdit = dlg.findChild<KLineEdit *>(QStringLiteral("search_lineedit"));
    QPushButton *seachButton = dlg.findChild<QPushButton *>(QStringLiteral("search_button"));
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

QTEST_MAIN(BlackListBalooEmailCompletionDialogTest)
