/*
 * SPDX-FileCopyrightText: 2014 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef WIDGETS_MESSAGEBOX_H
#define WIDGETS_MESSAGEBOX_H

#include "messageboxinterface.h"

namespace Widgets {

class MessageBox : public MessageBoxInterface
{
public:
    typedef QSharedPointer<MessageBox> Ptr;

    virtual ~MessageBox();

    QMessageBox::Button askConfirmation(QWidget *parent, const QString &title, const QString &text) override;
};

}

#endif
