/*
 * SPDX-FileCopyrightText: 2014 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "messagebox.h"

Widgets::MessageBox::~MessageBox()
{
}

QMessageBox::Button Widgets::MessageBox::askConfirmation(QWidget *parent, const QString &title, const QString &text)
{
    return QMessageBox::question(parent, title, text, QMessageBox::Yes | QMessageBox::No);
}
