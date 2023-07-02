/*
 * SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "runningtaskmodelinterface.h"

using namespace Presentation;

RunningTaskModelInterface::RunningTaskModelInterface(QObject *parent)
    : QObject(parent)
{
}

RunningTaskModelInterface::~RunningTaskModelInterface()
{
}

#include "moc_runningtaskmodelinterface.cpp"
