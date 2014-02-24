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


#ifndef DOMAIN_ARTIFACT_H
#define DOMAIN_ARTIFACT_H

#include <QMetaType>
#include <QSharedPointer>
#include <QString>

namespace Domain {

class Artifact : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)

public:
    typedef QSharedPointer<Artifact> Ptr;
    typedef QList<Artifact::Ptr> List;

    Artifact(QObject *parent = 0);
    virtual ~Artifact();

    QString text() const;
    QString title() const;

public slots:
    void setText(const QString &text);
    void setTitle(const QString &title);

signals:
    void textChanged(const QString &text);
    void titleChanged(const QString &title);

private:
    QString m_text;
    QString m_title;
};

}

Q_DECLARE_METATYPE(Domain::Artifact::Ptr)

#endif // DOMAIN_ARTIFACT_H
