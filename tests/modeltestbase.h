/* This file is part of Zanshin Todo.

   Copyright 2008 Kevin Ottens <ervin@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.
*/

#ifndef MODELTESTBASE_H
#define MODELTESTBASE_H

#include <QtCore/QFile>
#include <QtCore/QModelIndex>
#include <QtCore/QObject>

#include <akonadi/agentinstance.h>
#include <akonadi/collection.h>

class ModelTestBase : public QObject
{
    Q_OBJECT

public:
    ModelTestBase();

protected:
    void sleepAndProcessEvents(int ms);
    Akonadi::Collection m_collection;

protected slots:
    virtual void initTestCase();
    virtual void cleanupTestCase();

private:
    Akonadi::AgentInstance m_agentInstance;
    QFile m_testFile;
};

Q_DECLARE_METATYPE(QModelIndex);


#endif

