/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef AKONADI_ITEMFETCHJOBINTERFACE_H
#define AKONADI_ITEMFETCHJOBINTERFACE_H

#include <AkonadiCore/Akonadi/Collection>
#include <AkonadiCore/Akonadi/Item>

class KJob;

namespace Akonadi {

class ItemFetchJobInterface
{
public:
    ItemFetchJobInterface();
    virtual ~ItemFetchJobInterface();

    KJob *kjob();

    virtual Item::List items() const = 0;
    virtual void setCollection(const Akonadi::Collection &collection) = 0;
};

}

#endif // AKONADI_ITEMFETCHJOBINTERFACE_H
