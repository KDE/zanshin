/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef PRESENTATION_METATYPES_H
#define PRESENTATION_METATYPES_H

#include <QAbstractItemModel>
#include <QMetaType>
#include <QSharedPointer>

typedef QSharedPointer<QObject> QObjectPtr;
typedef QList<QObjectPtr> QObjectPtrList;

namespace Presentation {

namespace MetaTypes
{
    void registerAll();
}

}

// cppcheck's parser somehow confuses it for a C-cast
// cppcheck-suppress cstyleCast
Q_DECLARE_METATYPE(QAbstractItemModel*)

Q_DECLARE_METATYPE(QObjectPtr)
Q_DECLARE_METATYPE(QObjectPtrList)

#endif // PRESENTATION_METATYPES_H
