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

#include "blacklistbalooemaillist.h"
#include "libkdepim_debug.h"
#include <QPainter>
#include <QEvent>
#include <KLocalizedString>
#include <KEmailAddress>
using namespace KPIM;

BlackListBalooEmailList::BlackListBalooEmailList(QWidget *parent)
    : QListWidget(parent),
      mFirstResult(false)
{
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSortingEnabled(true);
}

BlackListBalooEmailList::~BlackListBalooEmailList()
{

}

void BlackListBalooEmailList::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::PaletteChange) {
        generalPaletteChanged();
    }
    QListWidget::changeEvent(event);
}

void BlackListBalooEmailList::setEmailBlackList(const QStringList &list)
{
    mEmailBlackList = list;
}

QHash<QString, bool> BlackListBalooEmailList::blackListItemChanged() const
{
    QHash<QString, bool> result;
    for (int i = 0; i < count(); ++i) {
        QListWidgetItem *element = item(i);
        KPIM::BlackListBalooEmailListItem *blackListItem = static_cast<KPIM::BlackListBalooEmailListItem *>(element);
        bool currentStatus = (blackListItem->checkState() == Qt::Checked);
        if (blackListItem->initializeStatus() != currentStatus) {
            result.insert(blackListItem->text(), currentStatus);
        }
    }
    return result;
}

void BlackListBalooEmailList::setExcludeDomain(const QStringList &domain)
{
    mExcludeDomain = domain;
}

void BlackListBalooEmailList::setEmailFound(const QStringList &list)
{
    mFirstResult = true;
    clear();
    QStringList emailsAdded;
    Q_FOREACH (const QString &mail, list) {
        bool excludeDomain = false;
        QString email, name;
        KEmailAddress::extractEmailAddressAndName(mail, email, name);

        Q_FOREACH (const QString &domain, mExcludeDomain) {
            if (email.endsWith(domain)) {
                excludeDomain = true;
                break;
            }
        }
        if (excludeDomain) {
            continue;
        }
        if (!emailsAdded.contains(mail)) {
            BlackListBalooEmailListItem *item = new BlackListBalooEmailListItem(this);
            if (mEmailBlackList.contains(mail)) {
                item->setCheckState(Qt::Checked);
                item->setInitializeStatus(true);
            } else {
                item->setCheckState(Qt::Unchecked);
            }
            item->setText(mail);
            emailsAdded << mail;
        }
    }
}

void BlackListBalooEmailList::generalPaletteChanged()
{
    const QPalette palette = viewport()->palette();
    QColor color = palette.text().color();
    color.setAlpha(128);
    mTextColor = color;
}

void BlackListBalooEmailList::paintEvent(QPaintEvent *event)
{
    if (mFirstResult && (!model() || model()->rowCount() == 0)) {
        QPainter p(viewport());

        QFont font = p.font();
        font.setItalic(true);
        p.setFont(font);

        if (!mTextColor.isValid()) {
            generalPaletteChanged();
        }
        p.setPen(mTextColor);

        p.drawText(QRect(0, 0, width(), height()), Qt::AlignCenter, i18n("No result found"));
    } else {
        QListWidget::paintEvent(event);
    }
}

BlackListBalooEmailListItem::BlackListBalooEmailListItem(QListWidget *parent)
    : QListWidgetItem(parent),
      mInitializeStatus(false)
{
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
}

BlackListBalooEmailListItem::~BlackListBalooEmailListItem()
{

}

bool BlackListBalooEmailListItem::initializeStatus() const
{
    return mInitializeStatus;
}

void BlackListBalooEmailListItem::setInitializeStatus(bool initializeStatus)
{
    mInitializeStatus = initializeStatus;
}

