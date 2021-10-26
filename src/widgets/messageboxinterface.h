/*
 * SPDX-FileCopyrightText: 2014 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef WIDGETS_MESSAGEBOXINTERFACE_H
#define WIDGETS_MESSAGEBOXINTERFACE_H

#include <QString>
#include <QSharedPointer>
#include <QMessageBox>

class QWidget;

namespace Widgets {

class MessageBoxInterface
{
public:
    typedef QSharedPointer<MessageBoxInterface> Ptr;

    virtual ~MessageBoxInterface();

    virtual QMessageBox::Button askConfirmation(QWidget *parent, const QString &title, const QString &text) = 0;
};

}

#endif
