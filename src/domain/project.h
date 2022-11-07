/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */



#ifndef DOMAIN_PROJECT_H
#define DOMAIN_PROJECT_H

#include <QMetaType>
#include <QObject>
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

    explicit Project(QObject *parent = nullptr);
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
