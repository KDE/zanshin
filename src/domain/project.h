/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>

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


#ifndef DOMAIN_PROJECT_H
#define DOMAIN_PROJECT_H

#include <QMetaType>
#include <QSharedPointer>
#include <QString>

namespace Domain {

class Project : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

public:
    typedef QSharedPointer<Project> Ptr;
    typedef QList<Project::Ptr> List;

    explicit Project(QObject *parent = 0);
    virtual ~Project();

    QString name() const;

public slots:
    void setName(const QString &name);

signals:
    void nameChanged(const QString &name);

private:
    QString m_name;
};

}

Q_DECLARE_METATYPE(Domain::Project::Ptr)

#endif // DOMAIN_PROJECT_H
