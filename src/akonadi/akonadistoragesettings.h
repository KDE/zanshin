/*
 * SPDX-FileCopyrightText: 2012 Christian Mollekopf <chrigi_1@fastmail.fm>
   SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
   * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
   */


#ifndef AKONADI_STORAGESETTINGS_H
#define AKONADI_STORAGESETTINGS_H

#include <Akonadi/Collection>

namespace Akonadi
{

class StorageSettings : public QObject
{
    Q_OBJECT
private:
    StorageSettings();

public:
    static StorageSettings &instance();
    
    Akonadi::Collection defaultCollection();

public Q_SLOTS:
    void setDefaultCollection(const Akonadi::Collection &collection);

signals:
    void defaultCollectionChanged(const Akonadi::Collection &collection);
};

}

#endif // AKONADI_STORAGESETTINGS_H
