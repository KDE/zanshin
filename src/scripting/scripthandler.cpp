/* This file is part of Zanshin

   Copyright 2015 Theo Vaucher <theo.vaucher@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.
*/


#include "scripthandler.h"
#include "taskaction.h"

#include <QFile>
#include <QJSEngine>

using namespace Scripting;

ScriptHandler::ScriptHandler(const Domain::TaskRepository::Ptr &taskRepository, QObject *parent)
    : QObject(parent),
      m_taskRepository(taskRepository),
      m_engine(new QJSEngine(this))
{
    m_engine->globalObject().setProperty(QStringLiteral("task"),
                                         m_engine->newQObject(new TaskAction(m_taskRepository, this)));
}

QJSValue ScriptHandler::evaluateFile(const QString &filename)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QByteArray filecontent = file.readAll();
    file.close();

    return m_engine->evaluate(filecontent, filename);
}

QJSValue ScriptHandler::evaluateString(const QString &string)
{
    return m_engine->evaluate(string);
}

QJSEngine *ScriptHandler::engine()
{
    return m_engine;
}

