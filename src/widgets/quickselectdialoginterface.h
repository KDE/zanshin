/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef WIDGETS_QUICKSELECTDIALOGINTERFACE_H
#define WIDGETS_QUICKSELECTDIALOGINTERFACE_H

#include <QPersistentModelIndex>
#include <QSharedPointer>

class QAbstractItemModel;

namespace Widgets {

class QuickSelectDialogInterface
{
public:
    typedef QSharedPointer<QuickSelectDialogInterface> Ptr;

    virtual ~QuickSelectDialogInterface();

    virtual int exec() = 0;

    virtual QPersistentModelIndex selectedIndex() const = 0;
    virtual void setModel(QAbstractItemModel *model) = 0;
};

}

#endif // WIDGETS_QUICKSELECTDIALOGINTERFACE_H
