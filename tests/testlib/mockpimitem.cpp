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

#include "mockpimitem.h"

#include <QTimer>

#include <KJob>

class MockJob : public KJob
{
    Q_OBJECT
public:
    MockJob(int code, const QString &text)
        : m_errorCode(code),
          m_errorText(text),
          m_started(false)
    {
        start();
    }

    void start()
    {
        if (!m_started) {
            m_started = true;
            QTimer::singleShot(100, this, SLOT(onTimeout()));
        }
    }

private slots:
    void onTimeout()
    {
        setError(m_errorCode);
        setErrorText(m_errorText);
        emitResult();
    }

private:
    int m_errorCode;
    QString m_errorText;
    bool m_started;
};

MockPimItem::MockPimItem()
    : m_itemType(PimItemIndex::Todo),
      m_status(NotComplete),
      m_errorCode(KJob::NoError)
{
}

MockPimItem::~MockPimItem()
{
}

void MockPimItem::setItemType(PimItemIndex::ItemType type)
{
    m_itemType = type;
}

PimItemIndex::ItemType MockPimItem::itemType() const
{
    return m_itemType;
}

void MockPimItem::setMimeType(const QString &mimeType)
{
    m_mimeType = mimeType;
}

QString MockPimItem::mimeType() const
{
    return m_mimeType;
}

void MockPimItem::setStatus(PimItem::ItemStatus status)
{
    m_status = status;
}

PimItem::ItemStatus MockPimItem::status() const
{
    return m_status;
}

void MockPimItem::setUid(const QString &uid)
{
    m_uid = uid;
}

QString MockPimItem::uid() const
{
    return m_uid;
}

void MockPimItem::setIconName(const QString &iconName)
{
    m_iconName = iconName;
}

QString MockPimItem::iconName() const
{
    return m_iconName;
}

void MockPimItem::setText(const QString &text, bool isRich)
{
    m_text = text;
    m_isTextRich = isRich;
}

QString MockPimItem::text() const
{
    return m_text;
}

bool MockPimItem::isTextRich() const
{
    return m_isTextRich;
}

void MockPimItem::setTitle(const QString &title, bool isRich)
{
    m_title = title;
    m_isTitleRich = isRich;
}

QString MockPimItem::title() const
{
    return m_title;
}

bool MockPimItem::isTitleRich() const
{
    return m_isTitleRich;
}

KDateTime MockPimItem::date(PimItem::DateRole role) const
{
    return m_dates.value(role);
}

bool MockPimItem::setDate(PimItem::DateRole role, const KDateTime &date)
{
    m_dates.insert(role, date);
    return true;
}

void MockPimItem::setSaveError(int errorCode, const QString &errorText)
{
    m_errorCode = errorCode;
    m_errorText = errorText;
}

KJob *MockPimItem::saveItem()
{
    return new MockJob(m_errorCode, m_errorText);
}
