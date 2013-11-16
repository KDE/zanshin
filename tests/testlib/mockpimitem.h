/* This file is part of Zanshin Todo.

   Copyright 2013 Kevin Ottens <ervin@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.
*/

#ifndef MOCKPIMITEM
#define MOCKPIMITEM

#include "core/pimitem.h"

class MockPimItem : public PimItem
{
public:
    MockPimItem();
    virtual ~MockPimItem();

    void setItemType(PimItemIndex::ItemType type);
    virtual PimItemIndex::ItemType itemType() const;

    void setMimeType(const QString &mimeType);
    virtual QString mimeType() const;

    void setStatus(ItemStatus status);
    virtual ItemStatus status() const;

    void setUid(const QString &uid);
    virtual QString uid() const;

    void setIconName(const QString &iconName);
    virtual QString iconName() const;

    virtual void setText(const QString &text, bool isRich = false);
    virtual QString text() const;
    virtual bool isTextRich() const;

    virtual void setTitle(const QString &title, bool isRich = false);
    virtual QString title() const;
    virtual bool isTitleRich() const;

    virtual KDateTime date(DateRole role) const;
    virtual bool setDate(DateRole role, const KDateTime &date);

    void setSaveError(int errorCode, const QString &errorText = QString());
    virtual KJob *saveItem();

private:
    PimItemIndex::ItemType m_itemType;
    ItemStatus m_status;
    QString m_mimeType;
    QString m_uid;
    QString m_iconName;
    QString m_text;
    bool m_isTextRich;
    QString m_title;
    bool m_isTitleRich;
    QHash<DateRole, KDateTime> m_dates;
    KDateTime m_primaryDate;
    int m_errorCode;
    QString m_errorText;
};

#endif // MOCKPIMITEM
