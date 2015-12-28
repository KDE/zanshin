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

#ifndef BLACKLISTBALOOEMAILLIST_H
#define BLACKLISTBALOOEMAILLIST_H

#include <QListWidget>
#include <QListWidgetItem>
#include <QHash>
#include "kdepim_export.h"

namespace KPIM
{

class KDEPIM_EXPORT BlackListBalooEmailListItem : public QListWidgetItem
{
public:
    explicit BlackListBalooEmailListItem(QListWidget *parent = Q_NULLPTR);
    ~BlackListBalooEmailListItem();

    bool initializeStatus() const;
    void setInitializeStatus(bool initializeStatus);

private:
    bool mInitializeStatus;
};

class KDEPIM_EXPORT BlackListBalooEmailList : public QListWidget
{
    Q_OBJECT
public:
    explicit BlackListBalooEmailList(QWidget *parent = Q_NULLPTR);
    ~BlackListBalooEmailList();

    void setEmailBlackList(const QStringList &list);

    QHash<QString, bool> blackListItemChanged() const;

    void setExcludeDomain(const QStringList &domain);
    void setEmailFound(const QStringList &);
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;

private:
    void generalPaletteChanged();
    QStringList mEmailBlackList;
    QStringList mExcludeDomain;
    QColor mTextColor;
    bool mFirstResult;
};
}

#endif // BLACKLISTBALOOEMAILLIST_H
