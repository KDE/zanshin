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

#ifndef RECENTADDRESSDIALOG_H
#define RECENTADDRESSDIALOG_H

#include "kdepim_export.h"
#include <QDialog>
#include <QStringList>
class KConfig;

namespace KPIM
{
class RecentAddressWidget;
class KDEPIM_EXPORT RecentAddressDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RecentAddressDialog(QWidget *parent = Q_NULLPTR);
    ~RecentAddressDialog();

    void setAddresses(const QStringList &addrs);
    void storeAddresses(KConfig *config);
    bool wasChanged() const;

private:
    void readConfig();
    void writeConfig();
    RecentAddressWidget *mRecentAddressWidget;
};
}

#endif // RECENTADDRESSDIALOG_H
