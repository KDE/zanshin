/*
 * SPDX-FileCopyrightText: 2014 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef MESSAGEBOX_STUB_H
#define MESSAGEBOX_STUB_H

#include "widgets/messageboxinterface.h"

class MessageBoxStub : public Widgets::MessageBoxInterface
{
public:
    typedef QSharedPointer<MessageBoxStub> Ptr;

    MessageBoxStub() : m_called(false) {}

    QMessageBox::Button askConfirmation(QWidget *, const QString &, const QString &) override {
        m_called = true;
        return QMessageBox::Yes;
    }

    bool called() const { return m_called; }

private:
    bool m_called;
};

#endif
