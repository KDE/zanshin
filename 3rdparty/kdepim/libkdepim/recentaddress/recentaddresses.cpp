/*
 *
 *  Copyright (c) 2001-2003 Carsten Pfeiffer <pfeiffer@kde.org>
 *  Copyright (c) 2003 Zack Rusin <zack@kde.org>
 *
 *  KMail is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License, version 2, as
 *  published by the Free Software Foundation.
 *
 *  KMail is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */
#include "recentaddresses.h"

#include <KEmailAddress>
#include <KConfig>
#include <KConfigGroup>
#include "libkdepim_debug.h"
#include <KLocalizedString>
#include <KSharedConfig>
#include <QCoreApplication>

using namespace KPIM;

RecentAddresses *s_self = Q_NULLPTR;

void deleteGlobalRecentAddresses()
{
    delete s_self;
    s_self = Q_NULLPTR;
}

RecentAddresses *RecentAddresses::self(KConfig *config)
{
    if (!s_self) {
        s_self = new RecentAddresses(config);
        qAddPostRoutine(deleteGlobalRecentAddresses);
    }
    return s_self;
}

bool RecentAddresses::exists()
{
    return s_self != Q_NULLPTR;
}

RecentAddresses::RecentAddresses(KConfig *config)
{
    if (!config) {
        load(KSharedConfig::openConfig().data());
    } else {
        load(config);
    }
}

RecentAddresses::~RecentAddresses()
{
    // if you want this destructor to get called, use K_GLOBAL_STATIC
    // on s_self
}

void RecentAddresses::load(KConfig *config)
{
    QStringList addresses;
    QString name;
    QString email;

    m_addresseeList.clear();
    KConfigGroup cg(config, "General");
    m_maxCount = cg.readEntry("Maximum Recent Addresses", 40);
    addresses = cg.readEntry("Recent Addresses", QStringList());
    QStringList::ConstIterator end(addresses.constEnd());
    for (QStringList::ConstIterator it = addresses.constBegin(); it != end; ++it) {
        KContacts::Addressee::parseEmailAddress(*it, name, email);
        if (!email.isEmpty()) {
            KContacts::Addressee addr;
            addr.setNameFromString(name);
            addr.insertEmail(email, true);
            m_addresseeList.append(addr);
        }
    }

    adjustSize();
}

void RecentAddresses::save(KConfig *config)
{
    KConfigGroup cg(config, "General");
    cg.writeEntry("Recent Addresses", addresses());
}

void RecentAddresses::add(const QString &entry)
{
    if (!entry.isEmpty() && m_maxCount > 0) {
        const QStringList list = KEmailAddress::splitAddressList(entry);
        QStringList::const_iterator e_itEnd(list.constEnd());
        for (QStringList::const_iterator e_it = list.constBegin(); e_it != e_itEnd; ++e_it) {
            KEmailAddress::EmailParseResult errorCode = KEmailAddress::isValidAddress(*e_it);
            if (errorCode != KEmailAddress::AddressOk) {
                continue;
            }
            QString email;
            QString fullName;
            KContacts::Addressee addr;

            KContacts::Addressee::parseEmailAddress(*e_it, fullName, email);

            KContacts::Addressee::List::Iterator end(m_addresseeList.end());
            for (KContacts::Addressee::List::Iterator it = m_addresseeList.begin();
                    it != end; ++it) {
                if (email == (*it).preferredEmail()) {
                    //already inside, remove it here and add it later at pos==1
                    m_addresseeList.erase(it);
                    break;
                }
            }
            addr.setNameFromString(fullName);
            addr.insertEmail(email, true);
            m_addresseeList.prepend(addr);
            adjustSize();
        }
    }
}

void RecentAddresses::setMaxCount(int count)
{
    if (count != m_maxCount) {
        m_maxCount = count;
        adjustSize();
    }
}

uint RecentAddresses::maxCount() const
{
    return m_maxCount;
}

void RecentAddresses::adjustSize()
{
    while (m_addresseeList.count() > m_maxCount) {
        m_addresseeList.takeLast();
    }
}

void RecentAddresses::clear()
{
    m_addresseeList.clear();
    adjustSize();
}

QStringList RecentAddresses::addresses() const
{
    QStringList addresses;
    addresses.reserve(m_addresseeList.count());
    KContacts::Addressee::List::ConstIterator end = m_addresseeList.constEnd();
    for (KContacts::Addressee::List::ConstIterator it = m_addresseeList.constBegin();
            it != end; ++it) {
        addresses.append((*it).fullEmail());
    }
    return addresses;
}
