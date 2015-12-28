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

#ifndef BLACKLISTBALOOEMAILCOMPLETIONWIDGET_H
#define BLACKLISTBALOOEMAILCOMPLETIONWIDGET_H

#include <QWidget>
class QPushButton;
class KLineEdit;
class QLabel;
class KListWidgetSearchLine;
namespace KPIM
{
class BlackListBalooEmailList;
class BlackListBalooEmailWarning;
class BlackListBalooEmailCompletionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BlackListBalooEmailCompletionWidget(QWidget *parent = Q_NULLPTR);
    ~BlackListBalooEmailCompletionWidget();

    void save();
    void load();
    void setEmailBlackList(const QStringList &list);
private Q_SLOTS:
    void slotSelectionChanged();
    void slotUnselectEmails();
    void slotSelectEmails();
    void slotSearchLineEditChanged(const QString &text);
    void slotSearch();
    void slotLinkClicked(const QString &link);
    void slotEmailFound(const QStringList &list);
    void slotCheckIfUpdateBlackListIsNeeded();
    void slotSaveChanges();
    void slotShowAllBlacklistedEmail();
private:
    void hideMoreResultAndChangeLimit();
    QStringList mOriginalExcludeDomain;
    QLabel *mNumberOfEmailsFound;
    KLineEdit *mSearchLineEdit;
    KLineEdit *mExcludeDomainLineEdit;
    BlackListBalooEmailList *mEmailList;
    QPushButton *mSearchButton;
    QPushButton *mSelectButton;
    QPushButton *mUnselectButton;
    QPushButton *mShowAllBlackListedEmails;
    QLabel *mMoreResult;
    KListWidgetSearchLine *mSearchInResultLineEdit;
    BlackListBalooEmailWarning *mBlackListWarning;
    int mLimit;
};
}

#endif // BLACKLISTBALOOEMAILCOMPLETIONWIDGET_H
