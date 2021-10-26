/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef AKONADI_COLLECTIONFETCHJOBINTERFACE_H
#define AKONADI_COLLECTIONFETCHJOBINTERFACE_H

#include <AkonadiCore/Akonadi/Collection>

class KJob;

namespace Akonadi {

class CollectionFetchJobInterface
{
public:
    CollectionFetchJobInterface();
    virtual ~CollectionFetchJobInterface();

    KJob *kjob();

    virtual Collection::List collections() const = 0;
    virtual void setResource(const QString &resource) = 0;
};

}

#endif // AKONADI_COLLECTIONFETCHJOBINTERFACE_H
