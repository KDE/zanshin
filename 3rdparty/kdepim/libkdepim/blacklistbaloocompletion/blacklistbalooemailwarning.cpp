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

#include "blacklistbalooemailwarning.h"
#include <KLocalizedString>
#include <QAction>

using namespace KPIM;

BlackListBalooEmailWarning::BlackListBalooEmailWarning(QWidget *parent)
    : KMessageWidget(parent)
{
    setVisible(false);
    setCloseButtonVisible(false);
    setMessageType(Warning);
    setWordWrap(true);

    setText(i18n("The list was changed. Do you want to save before to make another search ?"));
    QAction *saveAction = new QAction(i18n("Save"), this);
    saveAction->setObjectName(QStringLiteral("saveblacklist"));
    connect(saveAction, &QAction::triggered, this, &BlackListBalooEmailWarning::slotSaveBlackList);
    addAction(saveAction);

    QAction *searchAction = new QAction(i18n("Search"), this);
    searchAction->setObjectName(QStringLiteral("search"));
    connect(searchAction, &QAction::triggered, this, &BlackListBalooEmailWarning::slotSearch);
    addAction(searchAction);
}

BlackListBalooEmailWarning::~BlackListBalooEmailWarning()
{

}

void BlackListBalooEmailWarning::slotSaveBlackList()
{
    animatedHide();
    Q_EMIT saveChanges();
}

void BlackListBalooEmailWarning::slotSearch()
{
    animatedHide();
    Q_EMIT newSearch();
}
